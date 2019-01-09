#ifndef _H_KEY_DATA_H_
#define _H_KEY_DATA_H_

#include <stdint.h>

#include <avr/pgmspace.h>

const uint32_t master_key_size = 24;  // 0x00000018
const uint8_t master_key_data[24] PROGMEM  = {
    '\x39','\x6a','\xd1','\x1a','\x6c','\x1c','\x54','\x8f','\x7c','\xf7','\x56','\x3d','\x43','\x7e','\x85','\xda', // 9j..l.T.|.V=C~..
    '\x6d','\x5c','\xb0','\x82','\xff','\x97','\xb9','\xe0',};

#endif //_H_KEY_DATA_H_
