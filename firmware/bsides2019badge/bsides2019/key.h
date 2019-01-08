#ifndef _H_KEY_DATA_H_
#define _H_KEY_DATA_H_

#include <stdint.h>

#include <avr/pgmspace.h>

const uint32_t master_key_size = 24;  // 0x00000018
const uint8_t master_key_data[24] PROGMEM  = {
    '\x75','\xb7','\x23','\x72','\x6c','\x1c','\x54','\x8f','\x7c','\xf7','\x56','\x3d','\x43','\x7e','\x85','\xda', // u.#rl.T.|.V=C~..
    '\x6d','\x5c','\xb0','\x82','\xff','\xb8','\x59','\xbf',};

#endif //_H_KEY_DATA_H_