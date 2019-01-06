import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), 'nopia'))
import binascii
from bsides2019.token import Token
MASTER_HEX_FILE = os.path.join(os.path.dirname(__file__), '..', 'keys', 'master.hex')
assert os.path.exists(MASTER_HEX_FILE)
with open(MASTER_HEX_FILE, 'rb') as handle:
    KEY = binascii.a2b_hex(handle.read())
assert len(KEY) == 16

def main():
    t = Token()
    t.device_id = 1234
    t.score = 311700
    t.game_code = 0x11
    text = t.encode(KEY)
    print(t)
    print(text)
    print(Token.decode(KEY, text))

if __name__ == '__main__':
    main()
