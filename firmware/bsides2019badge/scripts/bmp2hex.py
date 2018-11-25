import os
import sys
import struct
import argparse

class HexDataHeader(object):

    def __init__(self):
        self.data = None
        self.var_namespace = None
        self.var_header_guard = None
        self.var_data_name = None
        self.var_data_size = None
        self.metadata = dict()

    def pack(self):
        # indent
        indent = 0

        # source code text
        text = ''

        # header
        if not self.var_header_guard:
            text += '#pragma once\n'
        else:
            text += '#ifndef {}\n'.format(self.var_header_guard)
            text += '#define {}\n'.format(self.var_header_guard)
        text += '\n'

        # includes
        if not self.var_namespace:
            text += '#include <stdint.h>\n'
        else:
            text += '#include <cstdint>\n'
        text += '\n'

        # non-ardiuno guard
        text += '// non - arduino\n'
        text += '#ifndef PROGMEM\n'
        text += '#define PROGMEM\n'
        text += '#endif\n'
        text += '#ifndef pgm_read_byte\n'
        text += '#define pgm_read_byte(addr) (*((uint8_t*)addr))\n'
        text += '#endif\n'
        text += '\n'

        # namesapce
        if self.var_namespace:
            indent += 4
            for namespace in self.var_namespace.split('::'):
                text += 'namespace {} {}\n'.format(namespace, '{')
            text += '\n'

        # metadata
        for var_name, value in self.metadata:
            if isinstance(value, str):
                var_type = 'const char *'
                var_value = '"{}";'.format(value)
            elif isinstance(value, int):
                var_type = 'const uint32_t'
                var_value = '{};  // 0x{:08x}'.format(value, value)
            else:
                raise NotImplementedError()
            text += '{}const {} {} = {}\n'.format(' ' * indent, var_type, var_name, var_value)

        # size
        text += '{}const uint32_t {} = {};  // 0x{:08x}\n'.format(' ' * indent, self.var_data_size, len(self.data), len(self.data))

        # data
        col = 0
        text += '{}const uint8_t {}[{}] PROGMEM  = {}\n'.format(' ' * indent, self.var_data_name, len(self.data), '{')
        indent += 4
        text += ' ' * indent
        lines = list()
        for data_byte in self.data:
            line = "'\\x{:02x}',".format(data_byte)
            col += 1
            if col == 16:
                col = 0
                line += '\n{}'.format(' ' * indent)
            lines.append(line)
        text += ''.join(lines)
        text += '};\n'
        text += '\n'

        # namesapce
        if self.var_namespace:
            for namespace in self.var_namespace.split('::'):
                text += '{} // namespace {}\n'.format('}', namespace)
            text += '\n'

        # footer
        if self.var_header_guard:
            text += '#endif //{}\n'.format(self.var_header_guard)
        return text.encode('ascii')


class Bitmap(object):

    def __init__(self):
        self.width = None
        self.height = None
        self.bits_per_pixel = None
        self.data = None

    @classmethod
    def parse(cls, data):
        obj = cls()
        obj.set_raw(data)
        return obj

    def set_raw(self, data):
        if data[:2] != b'BM':
            raise NotImplementedError("magic {} is unknown".format(str(data[:2])))
        buffer_offset = struct.unpack('<I', data[0x0a: 0x0a + 4])[0]
        # header 2 size is stored in header 2 (this caused much confusion)
        header2_size = struct.unpack('<I', data[0x0e: 0x0e + 4])[0]
        if header2_size != 40:
            raise NotImplementedError("header size {}".format(header2_size))
        # but these two should match
        print(hex(buffer_offset))
        self.width = struct.unpack('<I', data[0x12: 0x12 + 4])[0]
        self.height = struct.unpack('<I', data[0x16: 0x16 + 4])[0]
        self.bits_per_pixel = struct.unpack('<H', data[0x1c: 0x1c + 2])[0]
        # scanline size
        bytes_per_pixel = self.bits_per_pixel / 8
        scan_line_size_no_pad = int(self.width * bytes_per_pixel)
        # scan lines are padded to 4-bytes (this caused much confusion)
        scan_line_size = scan_line_size_no_pad
        if scan_line_size % 4 != 0:
            scan_line_size += (4 - (scan_line_size % 4))
        assert len(data) == buffer_offset + (scan_line_size * self.height)
        # extract scan lines
        if self.bits_per_pixel == 24:
            # scan lines are stored bottom-top (this caused much confusion)
            scan_lines = list()
            for line_index in range(0, self.height):
                start = buffer_offset + (line_index * scan_line_size)
                end = start + scan_line_size_no_pad  # removing the padding
                scan_line = data[start: end]
                assert len(scan_line) == scan_line_size_no_pad
                scan_lines.append(scan_line)
            scan_lines = list(reversed(scan_lines))
            self.data = b''.join(scan_lines)
        elif self.bits_per_pixel == 1:
            # monochrome bitmap
            monochrome_bits = list()
            for y in range(0, self.height):
                # each row starts on a new scanline
                in_byte = buffer_offset + ((self.height - 1 - y) * scan_line_size)
                in_bit = 7
                for x in range(0, self.width):
                    bit = ((struct.unpack('<B', data[in_byte: in_byte + 1])[0] >> in_bit) & 0x1)
                    bit = 1 - bit
                    monochrome_bits.append(bit)
                    if in_bit > 0:
                        in_bit -= 1
                    else:
                        in_byte += 1
                        in_bit = 7
            assert len(monochrome_bits) == (self.width * self.height)
            current_byte = 0
            current_bit = 7
            monochrome_bytes = list()
            for bit in monochrome_bits:
                current_byte |= (((bit & 0x1) << current_bit) & 0xff)
                if current_bit > 0:
                    current_bit -= 1
                else:
                    monochrome_bytes.append(struct.pack('<B', current_byte))
                    current_byte = 0
                    current_bit = 7
            if current_bit != 7:
                monochrome_bytes.append(struct.pack('<B', current_byte))
            self.data = b''.join(monochrome_bytes)
        else:
            raise NotImplementedError("don't know how to parse bits per pixel of {}".format(self.bits_per_pixel))
        assert len(self.data) == int(self.height * self.width * bytes_per_pixel)


def main():
    if len(sys.argv) != 3:
        print('bmp2hex.py <84x48-monochrome.bmp> <c-header.h>')
        print('NOTE: must use python 3.4.3 or up')
        sys.exit(1)
    src = sys.argv[1]
    with open(src, 'rb') as handle:
        data = handle.read()
    bmp = Bitmap.parse(data)
    header = HexDataHeader()
    header.data = bmp.data
    header.var_header_guard = '_H_BMP_H_'
    header.var_data_name = 'bmp_data'
    header.var_data_size = 'bmp_size'
    with open(sys.argv[2], 'wb') as handle:
        handle.write(header.pack())
    sys.exit(0)

if __name__ == '__main__':
    main()
