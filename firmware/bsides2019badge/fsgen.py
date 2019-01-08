import os
import sys
import binascii

ROOT = os.path.dirname(os.path.realpath(__file__))
FS_PATH = os.path.join(ROOT, 'fs')
assert os.path.exists(FS_PATH)
VFS_GEN = os.path.join(ROOT, 'libraries', 'vfsc', 'vfscgen.py')
assert os.path.exists(VFS_GEN)
VFS_HEADER = os.path.join(ROOT, 'bsides2019', 'vfs.h')
KEY_HEX_FILE = os.path.join(ROOT, '..', '..', 'keys', 'master.hex')
assert os.path.exists(KEY_HEX_FILE)
with open(KEY_HEX_FILE, 'rb') as handle:
    KEY_BIN = binascii.a2b_hex(handle.read())
assert len(KEY_BIN) == 16
KEY_HEADER = os.path.join(ROOT, 'bsides2019', 'key.h')
KEY2_HEX_FILE = os.path.join(ROOT, '..', '..', 'keys', 'master2.hex')
assert os.path.exists(KEY2_HEX_FILE)
with open(KEY2_HEX_FILE, 'rb') as handle:
    KEY2_BIN = binascii.a2b_hex(handle.read())
assert len(KEY2_BIN) == 16
assert KEY_BIN != KEY2_BIN
KEY2_FS_FILE = os.path.join(FS_PATH, 'keys', 'key.bin')
assert os.path.exists(KEY2_FS_FILE)

sys.path.append(os.path.dirname(VFS_GEN))
from vfscgen import HexDataHeader

def main():
    assert sys.version_info.major == 3
    assert sys.version_info.minor >= 4
    with open(KEY2_FS_FILE, 'wb') as handle:
        handle.write(KEY2_BIN)
    with open(KEY2_FS_FILE, 'rb') as handle:
        assert KEY2_BIN == handle.read()
    key = KEY_BIN
    key = os.urandom(4) + key + b'\xff' + os.urandom(3)
    assert len(key) == 24
    header = HexDataHeader()
    header.data = key
    header.var_header_guard = '_H_KEY_DATA_H_'
    header.var_data_name = 'master_key_data'
    header.var_data_size = 'master_key_size'
    with open(KEY_HEADER, 'wb') as handle:
        handle.write(header.pack())
    sys.exit(os.system('{} {} {} {}'.format(sys.executable, VFS_GEN, FS_PATH, VFS_HEADER)))

if __name__ == '__main__':
    main()
