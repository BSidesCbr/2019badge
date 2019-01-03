import os
import binascii
from bsides2019.token import Token
from key import key

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
