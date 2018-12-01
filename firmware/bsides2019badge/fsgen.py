import os
import sys

def main():
    assert sys.version_info.major == 3
    assert sys.version_info.minor >= 4
    root = os.path.dirname(os.path.realpath(__file__))
    assert os.path.exists(root)
    script = os.path.join(root, 'libraries', 'vfsc', 'vfscgen.py')
    assert os.path.exists(script)
    fs_path = os.path.join(root, 'fs')
    assert os.path.exists(fs_path)
    assert os.path.isdir(fs_path)
    c_header = os.path.join(root, 'bsides2019', 'vfs.h')
    sys.exit(os.system('{} {} {} {}'.format(sys.executable, script,fs_path,c_header)))

if __name__ == '__main__':
    main()
