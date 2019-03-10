import os
import sys
sys.path.append(os.path.join(os.path.dirname(__file__), 'nopia'))
import binascii
from bsides2019.token import Token
MASTER_HEX_FILE = os.path.join(os.path.dirname(__file__), '..', 'keys', 'master.hex')
assert os.path.exists(MASTER_HEX_FILE)
with open(MASTER_HEX_FILE, 'rb') as handle:
    KEY = binascii.a2b_hex(handle.read())
MASTER_HEX_2_FILE = os.path.join(os.path.dirname(__file__), '..', 'keys', 'master2.hex')
assert os.path.exists(MASTER_HEX_2_FILE)
with open(MASTER_HEX_2_FILE, 'rb') as handle:
    KEY_2 = binascii.a2b_hex(handle.read())
assert len(KEY_2) == 16

def main():
    t = Token()
    t.device_id = 2176901350
    t.score = 3400
    t.game_code = Token.SCORE_CODE_SNAKE
    text = t.encode(KEY, KEY_2)
    print(t)
    print(text)
    print(Token.decode(KEY, KEY_2, text))

if __name__ == '__main__':
    main()
