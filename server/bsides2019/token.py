import os
import sys
import struct
import binascii
from Crypto.Cipher import AES

class Token(object):
    BLOCK_SIZE = 16
    SCORE_CODE_DEBUG = 0x11
    SCORE_CODE_SNAKE = 0x55
    SCORE_CODE_TETRIS = 0xaa

    def __init__(self):
        self.device_id = 0
        self.game_code = 0
        self.score = 0

    @property
    def imei(self):
        return "353434{:010d}".format(self.device_id)

    @property
    def game_name(self):
        if self.game_code == self.SCORE_CODE_DEBUG:
            return 'debug'
        elif self.game_code == self.SCORE_CODE_SNAKE:
            return 'snake'
        elif self.game_code == self.SCORE_CODE_TETRIS:
            return 'tetris'
        else:
            return hex(self.game_code)

    def __str__(self):
        return '{}\n{} {} pts'.format(self.imei, self.game_name, self.score)

    @classmethod
    def decode_key(cls, key):
        new_key = b''
        for i in range(0, len(key)):
            value = key[i]
            value ^= i
            value ^= 0xaa
            new_key += struct.pack('<B', value)
        assert len(new_key) == len(key)
        return new_key

    @classmethod
    def decrypt_aes_128_cbc_no_iv_single_block(cls, key, data):
        assert len(data) == cls.BLOCK_SIZE
        assert len(key) == cls.BLOCK_SIZE
        iv = b'\x00' * 16
        aes = AES.new(key, AES.MODE_CBC, iv)
        return aes.decrypt(data)

    @classmethod
    def encrypt_aes_128_cbc_no_iv_single_block(cls, key, data):
        assert len(data) == cls.BLOCK_SIZE
        assert len(key) == cls.BLOCK_SIZE
        iv = b'\x00' * 16
        aes = AES.new(key, AES.MODE_CBC, iv)
        return aes.encrypt(data)

    @classmethod
    def fb64_encode_char(cls, prev, value):
        if 0 != (prev & 0x1):
            if 0 == (prev & 0x2):
                if value < 10:
                    return chr(ord('0') + value)
                elif value == 10:
                    return '+'
                elif value == 11:
                    return '-'
                elif value > 11:
                    return chr(ord('N') + (value - 11))
            else:
                if 0 == (prev & 0x4):
                    return chr(ord('K') + value)
                else:
                    return chr(ord('k') + value)
        else:
            if 0 == (prev & 0x4):
                return chr(ord('A') + value)
            else:
                return chr(ord('a') + value)

    @classmethod
    def fb64_encode(cls, data):
        out = ''
        assert len(data) == 16
        prev = 0
        value = 0
        j = 0
        for i in range(0, len(data)):
            value = (data[i] >> 4) & 0xf
            out += cls.fb64_encode_char(prev, value)
            j += 1
            prev = value
            value = (data[i] >> 0) & 0xf
            out += cls.fb64_encode_char(prev, value)
            j += 1
            prev = value
        for i in range(32, 43):
            value = os.urandom(1)[0] & 0xf
            out += cls.fb64_encode_char(prev, value)
            prev = value
        out += '='
        assert len(out) == 44
        return out

    @classmethod
    def fb64_decode_char(cls, prev, letter):
        if 0 != (prev & 0x1):
            if 0 == (prev & 0x2):
                if ord(letter) >= ord('0') and ord(letter) <= ord('9'):
                    return ord(letter) - ord('0')
                elif letter == '+':
                    return 10
                elif letter == '-':
                    return 11
                else:
                    return (ord(letter) - ord('N')) + 11
            else:
                if 0 == (prev & 0x4):
                    return ord(letter) - ord('K')
                else:
                    return ord(letter) - ord('k')
        else:
            if 0 == (prev & 0x4):
                return ord(letter) - ord('A')
            else:
                return ord(letter) - ord('a')

    @classmethod
    def fb64_decode(cls, data):
        assert len(data) == 44
        data = data.replace(b'-', b'/').replace(b' ', b'+')
        assert len(binascii.a2b_base64(data)) == 32
        prev = 0
        out = b''
        for i in range(0, 32, 2):
            letter = chr(data[i])
            value = cls.fb64_decode_char(prev, letter)
            assert value >= 0
            assert value <= 0xf
            value_high = value
            prev = value
            letter = chr(data[i + 1])
            value = cls.fb64_decode_char(prev, letter)
            assert value >= 0
            assert value <= 0xf
            value_low = value
            prev = value
            value = ((value_high << 4) & 0xf0) | ((value_low << 0) & 0x0f)
            assert value >= 0
            assert value <= 0xff
            out += struct.pack('<B', value)
        assert len(out) == 16
        return out

    @classmethod
    def decode(cls, key, data):
        obj = cls()
        key = cls.decode_key(key)
        if isinstance(data, str):
            data = data.encode('ascii')
        data = cls.fb64_decode(data)
        data = cls.decrypt_aes_128_cbc_no_iv_single_block(key, data)
        obj.device_id = struct.unpack('<I', data[0: 4])[0]
        obj.score = struct.unpack('<I', data[4: 8])[0]
        obj.game_code = struct.unpack('<B', data[8: 9])[0]
        return obj

    def encode(self, key):
        key = self.decode_key(key)
        data = struct.pack('<IIB', self.device_id, self.score, self.game_code)
        data += os.urandom(3)
        data = data + b'\x00' * (self.BLOCK_SIZE - len(data))
        assert len(data) == 16
        data = self.encrypt_aes_128_cbc_no_iv_single_block(key, data)
        text = self.fb64_encode(data)
        return text
