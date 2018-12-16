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

        # support for PROGMEM
        text += '#include <avr/pgmspace.h>\n'
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
        self.width = struct.unpack('<I', data[0x12: 0x12 + 4])[0]
        self.height = struct.unpack('<I', data[0x16: 0x16 + 4])[0]
        self.bits_per_pixel = struct.unpack('<H', data[0x1c: 0x1c + 2])[0]
        # scanline size
        bytes_per_pixel = self.bits_per_pixel / 8.0
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


class Hash(object):
    #  credits go to:
    #  http://lolengine.net/blog/2011/12/20/cpp-constant-string-hash

    @classmethod
    def sizeof(cls, s):
        # must be bytes
        if isinstance(s, str):
            s = s.encode('ascii') + b'\x00'
        return len(s)

    @classmethod
    def h1(cls, s, i, x):
        # must be bytes
        if isinstance(s, str):
            s = s.encode('ascii') + b'\x00'
        #define H1(s,i,x)   (x*65599u+(uint8_t)s[(i)<(sizeof(s)-1)?(sizeof(s)-1)-1-(i):(sizeof(s)-1)])
        index = (cls.sizeof(s)-1)-1-(i) if (i)<(cls.sizeof(s)-1) else (cls.sizeof(s)-1)
        size = cls.sizeof(s)
        return (x * 65599 + s[index]) & 0xffffffff

    @classmethod
    def h4(cls, s, i, x):
        # #define H4(s,i,x)   H1(s,i,H1(s,i+1,H1(s,i+2,H1(s,i+3,x))))
        return cls.h1(s, i, cls.h1(s, i + 1, cls.h1(s, i + 2, cls.h1(s, i + 3, x))))

    @classmethod
    def h16(cls, s, i, x):
        # #define H16(s,i,x)  H4(s,i,H4(s,i+4,H4(s,i+8,H4(s,i+12,x))))
        return cls.h4(s, i, cls.h4(s, i + 4, cls.h4(s, i + 8, cls.h4(s, i + 12, x))))

    @classmethod
    def h64(cls, s, i, x):
        # #define H64(s,i,x)  H16(s,i,H16(s,i+16,H16(s,i+32,H16(s,i+48,x))))
        return cls.h16(s, i, cls.h16(s, i + 16, cls.h16(s, i + 32, cls.h16(s, i + 48, x))))

    @classmethod
    def h256(cls, s, i, x):
        # #define H256(s,i,x) H64(s,i,H64(s,i+64,H64(s,i+128,H64(s,i+192,x))))
        return cls.h64(s, i, cls.h64(s, i + 64, cls.h64(s, i + 128, cls.h64(s, i + 192, x))))

    @classmethod
    def hash(cls, s):
        # #define HASH(s)     ((uint32_t)(H256(s,0,0)^(H256(s,0,0)>>16)))
        return (cls.h256(s,0,0)^(cls.h256(s,0,0)>>16)) & 0xffffffff


class VirtualFile(object):

    def __init__(self, arc=None, data=None):
        self.arc = arc
        self.data = data

    def size(self):
        return len(self.data)

    def pack(self):
        return self.data

    @classmethod
    def parse(self, path, arc):
        with open(path, 'rb') as handle:
            data = handle.read()
        return VirtualFile(arc, data)


class VirtualFileSystem(object):

    def __init__(self, files=None):
        self.files = files
        if not self.files:
            self.files = list()

    def size(self):
        size = 0
        size += struct.calcsize('<I')  # reserved
        size += struct.calcsize('<I')
        for vf in self.files:
            size += struct.calcsize('<I')
        for vf in self.files:
            size += struct.calcsize('<I') + vf.size()
        return size

    def pack(self):
        used_hashes = list()
        data = list()
        data += struct.pack('<I', 0)
        data += struct.pack('<I', len(self.files))
        for vf in self.files:
            vf_hash = Hash.hash(vf.arc)
            assert vf_hash not in used_hashes
            used_hashes.append(vf_hash)
            data += struct.pack('<I', vf_hash)
        for vf in self.files:
            vf_size = vf.size()
            vf_data = vf.pack()
            assert len(vf_data) == vf_size
            data += struct.pack('<I', vf_size) + vf_data
        return data


def main():
    assert sys.version_info.major == 3
    assert sys.version_info.minor >= 4
    if len(sys.argv) != 3:
        print('vfscgen.py <root> <c-header.h>')
        print('NOTE: must use python 3.4.3 or up')
        sys.exit(1)
    fs_root = os.path.realpath(sys.argv[1])
    fs = VirtualFileSystem()
    for root, folders, files in os.walk(fs_root):
        for filename in sorted(files):
            path = os.path.join(root, filename)
            arc = path[len(fs_root):].replace('\\', '/')
            vf = VirtualFile.parse(path, arc)
            if vf.arc.endswith('.bmp'):
                # for speed, use a raw (no header) bitmap
                vf.arc = vf.arc.replace('.bmp', '.raw')
                vf.data = data = Bitmap.parse(vf.data).data
            fs.files.append(vf)
    for vf in fs.files:
        print("{:08x} {} [{} bytes]".format(Hash.hash(vf.arc), vf.arc, vf.size()))
    print("[total {} bytes]".format(fs.size()))
    header = HexDataHeader()
    header.data = fs.pack()
    assert len(header.data) == fs.size()
    header.var_header_guard = '_H_VFS_DATA_H_'
    header.var_data_name = 'vfs_data'
    header.var_data_size = 'vfs_size'
    with open(sys.argv[2], 'wb') as handle:
        handle.write(header.pack())
    sys.exit(0)

if __name__ == '__main__':
    main()
