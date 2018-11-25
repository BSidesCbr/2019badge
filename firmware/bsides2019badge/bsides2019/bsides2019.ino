#include <Nokia_LCD.h>
#include <vintc.h>
#include <vgfxc.h>
#include <font4x6c.h>
#include <snakec.h>
#include <tetrisc.h>
#include <viewerc.h>
#include <tinymenuc.h>
#include <qrcodegen.h>

//-----------------------------------------------------------------------------
// Hardware
//-----------------------------------------------------------------------------
#define NOKIA_5110_CLK    13
#define NOKIA_5110_DIN    11
#define NOKIA_5110_DC     7
#define NOKIA_5110_CE     6
#define NOKIA_5110_RST    5
#define BUTTON_LEFT       4
#define BUTTON_OK         3
#define BUTTON_RIGHT      2

//-----------------------------------------------------------------------------
// Images
//-----------------------------------------------------------------------------
const unsigned char nopia_logo[504] PROGMEM = {
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x30','\x80','\x00','\x00','\x00','\x00','\x00','\x00','\x40','\x10','\x06','\x1f','\x00',
    '\x02','\x00','\x00','\x00','\x3e','\x00','\x00','\x06','\xc3','\xbc','\x00','\x00','\x80','\x10','\x1e','\x78',
    '\x00','\x06','\xce','\xff','\xc1','\x00','\x00','\x00','\x0f','\x83','\xc0','\x00','\xfc','\xff','\xfe','\x00',
    '\x00','\x00','\x07','\x9f','\xff','\x01','\x0c','\xe7','\xfe','\xe0','\x00','\x00','\x0f','\xc3','\xc0','\x70',
    '\x00','\xee','\x7f','\xfc','\x00','\x00','\x03','\xcf','\xc0','\x00','\x00','\x0f','\xff','\xbf','\xc0','\x08',
    '\x01','\xf8','\xf0','\x00','\x00','\x00','\xff','\xdf','\xd0','\x00','\x00','\x19','\xc3','\x00','\x00','\x00',
    '\x3f','\xff','\xfb','\x00','\x00','\x03','\x0f','\x18','\x80','\x80','\x87','\xff','\xbf','\xc0','\x40','\x10',
    '\x70','\x79','\xc3','\x80','\x00','\xf7','\xef','\xf8','\x00','\x00','\x1f','\xe7','\xfc','\x58','\x00','\x0c',
    '\x3f','\xff','\x00','\x04','\x03','\xfe','\x3f','\xf9','\x01','\x00','\x01','\xff','\xc4','\x01','\x00','\x71',
    '\xf3','\xe7','\xb0','\x00','\x20','\x37','\xb8','\x20','\x80','\x0d','\x8f','\xe7','\x3e','\x20','\x00','\x07',
    '\xdf','\x40','\x10','\x81','\x98','\x7e','\x39','\x00','\x07','\x27','\xff','\x80','\x08','\x01','\x33','\xc1',
    '\xf1','\xd2','\x22','\x3f','\xff','\xf1','\x00','\x00','\x06','\x3f','\x1f','\xff','\x80','\x00','\xff','\xf0',
    '\x02','\x40','\x50','\xe3','\x9f','\xde','\x58','\x00','\x01','\xe1','\x04','\x00','\x00','\x9f','\xa1','\xfd',
    '\xb5','\x92','\x84','\x00','\x40','\x00','\x09','\x93','\xbe','\x1e','\xcb','\x70','\x02','\x11','\x00','\x20',
    '\xa4','\x04','\x33','\xfd','\xec','\xb6','\x22','\x08','\x44','\x40','\x01','\x00','\x0f','\x3c','\xff','\x6f',
    '\x05','\x55','\x5f','\xf0','\x94','\xa2','\xd3','\x19','\x8f','\x36','\xf1','\x88','\x03','\xc7','\x91','\x88',
    '\x80','\x85','\xf9','\xe7','\x6e','\x55','\xdd','\xf8','\x1d','\x5e','\xa5','\x65','\xcf','\x9d','\xfc','\xcf',
    '\xff','\xf5','\xc3','\xc7','\xe1','\x53','\x22','\x5f','\xbb','\xc3','\xff','\xff','\xee','\x7b','\xf7','\x6a',
    '\x0a','\xd5','\xfb','\x8f','\xfe','\xa1','\x7f','\xef','\x5c','\x72','\xae','\x52','\xfd','\xf9','\xff','\x48',
    '\x04','\x7c','\x6b','\x87','\xd5','\x5b','\xff','\x0f','\x0e','\x00','\xf1','\xe1','\xc7','\x30','\x3d','\x9c',
    '\x29','\x71','\xf0','\xc7','\x86','\x1f','\x0c','\x7f','\x01','\xf7','\x6f','\x3f','\x0f','\x1c','\xfc','\x21',
    '\xf0','\xc7','\xf0','\x1e','\xbb','\xde','\xe0','\x70','\x8f','\xc2','\x1e','\x1c','\x7e','\x18','\x7e','\x3d',
    '\x4e','\x07','\x00','\xf8','\x20','\x01','\xc3','\xc3','\xc3','\x7e','\xff','\xe0','\x20','\x0f','\x82','\x08',
    '\xfc','\x3c','\x3c','\x3b','\x7f','\xce','\x30','\x08','\x78','\x61','\xff','\xe1','\x83','\x83','\xbb','\xff',
    '\xf1','\x80','\x87','\x0e','\x1f','\xfc','\x30','\x00','\x1f','\xff','\xff','\x1c','\x08','\x20','\xe0','\xff',
    '\xc3','\x0f','\xc3','\xff','\xff','\xf1','\xf1','\xe0','\x1e','\x1f','\xfc','\x71','\xfc','\x3f','\xff','\xff',
    '\x8f','\x1f','\x97','\xf1','\xff','\x87','\x3f','\xe3','\xff','\xff','\xff','\xf9','\xff','\xff','\x1f','\xfd',
    '\xf7','\xfe','\x3f','\xff','\xff','\xff','\xcf','\xff','\xf9','\xff','\xff','\xff','\xef','\xff','\xff','\xff',
    '\xff','\xff','\xff','\xdf','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff',
    '\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff',
    '\xff','\xff','\xff','\xff','\xff','\xff','\xff','\xff',};

const unsigned char bsidescbr_logo[504] PROGMEM  = {
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x60','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x0e','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x01',
    '\xe0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x1a','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x20','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x02',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x20','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x07','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\xf8',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x18','\xc0','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x03','\x06','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x0c','\x0f','\xff','\xff',
    '\x81','\x80','\x00','\x00','\x00','\x00','\x00','\xff','\xab','\x26','\xaf','\xf8','\x00','\x07','\xfc','\x07',
    '\xf0','\x0a','\xaa','\xb8','\xea','\xaa','\x80','\x00','\xff','\xe1','\xff','\x80','\xbf','\xab','\xfe','\xaf',
    '\xe8','\x00','\x0f','\xce','\x3f','\xfc','\x0a','\xaa','\xaa','\xaa','\xaa','\x80','\x00','\xe0','\xe7','\xc3',
    '\xe0','\xaa','\xaa','\xaa','\xaa','\xa8','\x00','\x0e','\x0e','\x78','\x1f','\x0a','\xaa','\xaa','\xaa','\xaa',
    '\x80','\x00','\xff','\xcf','\x00','\xf0','\xaf','\xff','\xff','\xea','\xa8','\x00','\x0f','\xfe','\xf1','\x8f',
    '\x0f','\x80','\x00','\x07','\xff','\x80','\x00','\xe0','\xee','\x3c','\x63','\x00','\x00','\x00','\x00','\x7f',
    '\x00','\x0e','\x0e','\xe3','\xc0','\x00','\x00','\x00','\x00','\x00','\x3e','\x00','\xe1','\xef','\x19','\xfe',
    '\x67','\xe0','\x1f','\x87','\xf8','\x78','\x0f','\xfc','\xf0','\x3f','\xee','\xff','\xcf','\xfc','\xff','\x81',
    '\xe0','\xff','\x87','\x87','\xfc','\xef','\xfe','\xff','\xdf','\xf0','\x07','\x00','\x00','\x78','\x70','\x0e',
    '\xe1','\xef','\x01','\xc0','\x00','\x3c','\x30','\xc3','\x87','\x00','\xee','\x0e','\xf0','\x1c','\x00','\x00',
    '\xe7','\x9e','\x38','\x7f','\x0c','\xe0','\xef','\xf9','\xf8','\x00','\x0e','\x79','\xe3','\x81','\xf9','\xde',
    '\x0e','\xff','\x8f','\xe0','\x00','\x77','\x8c','\x78','\x07','\xdd','\xc0','\xee','\x00','\x1f','\x00','\x07',
    '\x3c','\x0f','\x80','\x1d','\xdc','\x1d','\xe0','\x00','\x38','\x00','\x73','\xe1','\xf0','\x03','\xdd','\xc3',
    '\xde','\x00','\x07','\x80','\x07','\x1f','\xfe','\x0f','\xfd','\x9f','\xf9','\xff','\xbf','\xf8','\x00','\xe0',
    '\xff','\xc0','\xff','\x99','\xff','\x1f','\xfb','\xff','\x00','\x0c','\x03','\xf0','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x01','\x80','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x30','\x00',
    '\x00','\x06','\x18','\x68','\xc3','\x8e','\x38','\x60','\x0c','\x00','\x00','\x00','\x82','\x46','\x8a','\x20',
    '\xa2','\x89','\x03','\x00','\x00','\x00','\x08','\x3c','\x58','\xc3','\x8c','\x30','\xf1','\xc0','\x00','\x00',
    '\x00','\x82','\x45','\x8a','\x20','\xa2','\x89','\x60','\x00','\x00','\x00','\x06','\x24','\x48','\xc3','\x8a',
    '\x28','\x90','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x30','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x3f','\xf8','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',};

const unsigned char cybernatspc_logo[504] PROGMEM  = {
    '\x00','\x00','\x00','\x00','\x00','\x71','\x8c','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x4f',
    '\x73','\x90','\x88','\x00','\x00','\x00','\x00','\x00','\x00','\x01','\xff','\x78','\x80','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x02','\x38','\x7f','\x78','\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x06','\x30',
    '\xff','\x48','\x00','\x00','\x00','\x00','\x00','\x00','\x04','\x0f','\xe6','\xe0','\x10','\x00','\x00','\x00',
    '\x00','\x00','\x01','\xff','\xff','\x7c','\x70','\x00','\x00','\x00','\x00','\x00','\x00','\x7f','\xff','\xf7',
    '\xde','\x08','\x00','\x00','\x00','\x00','\x00','\x3f','\xfe','\xff','\x3f','\xc0','\x00','\x00','\x00','\x00',
    '\x00','\x0f','\xc6','\x7f','\xfb','\x6c','\x80','\x00','\x00','\x00','\x00','\x00','\xdf','\xf1','\xff','\xbf',
    '\x92','\x20','\x00','\x00','\x00','\x00','\x0d','\x8b','\x87','\xfb','\xf0','\x80','\x00','\x00','\x00','\x00',
    '\x02','\xf3','\xfc','\x7f','\xb7','\x50','\x00','\x00','\x00','\x00','\x00','\x46','\x67','\xc7','\xfd','\xe7',
    '\x40','\x00','\x00','\x00','\x00','\x11','\xcd','\xfc','\x7f','\xdf','\xf8','\x00','\x00','\x00','\x00','\x02',
    '\x31','\xbf','\x87','\xfd','\xde','\x00','\x00','\x00','\x00','\x00','\x0b','\xf7','\x98','\x7f','\xdf','\x60',
    '\x00','\x00','\x00','\x00','\x01','\x3f','\xe5','\x87','\xfd','\xb9','\x00','\x00','\x00','\x00','\x00','\x0f',
    '\x19','\x58','\x7f','\xdf','\xa0','\x00','\x00','\x00','\x00','\x01','\xe4','\xd5','\x87','\xfd','\xe0','\x00',
    '\x00','\x00','\x00','\x00','\x3e','\xed','\x70','\x7f','\xdd','\x00','\x00','\x00','\x00','\x00','\x0f','\x64',
    '\x57','\xff','\xfd','\xe8','\x00','\x00','\x00','\x00','\x00','\xd6','\xb5','\xff','\xff','\x8c','\x00','\x00',
    '\x00','\x00','\x00','\x0d','\x73','\x6f','\x8f','\xf2','\x08','\x00','\x00','\x00','\x00','\x00','\xda','\x82',
    '\xc3','\xff','\xe0','\x00','\x00','\x00','\x00','\x00','\x06','\xf1','\x3f','\xc2','\xfc','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x1b','\xfb','\x87','\xff','\xe0','\x00','\x00','\x00','\x00','\x00','\x01','\xfd','\x3f',
    '\xc0','\xbe','\x00','\x00','\x00','\x00','\x00','\x00','\x01','\xe3','\xff','\xf7','\xe0','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x1b','\xff','\xff','\xfc','\x00','\x00','\x00','\x00','\x00','\x00','\x03','\x7f','\xf8',
    '\xff','\x80','\x00','\x00','\x00','\x00','\x00','\x00','\x72','\x7f','\xff','\xf8','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x0e','\x0a','\xbf','\xff','\x00','\x00','\x00','\x00','\x00','\x00','\x01','\xcf','\x55','\xff',
    '\xf0','\x00','\x00','\x00','\x00','\x00','\x00','\x1f','\xa2','\x86','\xff','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x03','\xfd','\xdd','\x7f','\xe0','\x00','\x00','\x00','\x00','\x00','\x00','\x0f','\xf9','\x1a','\xfc',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x3f','\xfe','\xdf','\xc0','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\xff','\xff','\xf8','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x01','\xff','\xff','\x80',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x07','\xff','\xf0','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x0f','\xff','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x1f','\xe0','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x7c','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',};

const unsigned char cybernats_logo[504] PROGMEM  = {
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x3e','\x00','\x00',
    '\x00','\x00','\x60','\x00','\x00','\x00','\x00','\x0c','\x10','\x00','\x00','\x00','\x09','\x00','\x00','\x00',
    '\x00','\x03','\x3e','\x8c','\x00','\x00','\x07','\x6c','\x00','\x00','\x00','\x00','\xcf','\xe9','\x20','\x00',
    '\x00','\x86','\x30','\x00','\x00','\x00','\x33','\xff','\x6d','\x00','\x00','\x17','\xfc','\x80','\x00','\x00',
    '\x04','\xff','\xf5','\xd6','\x1c','\x66','\xff','\xb4','\x00','\x00','\x00','\xbf','\xff','\x3d','\x92','\x39',
    '\xc1','\xf7','\x40','\x00','\x00','\x17','\xff','\xf7','\xb6','\xdc','\x63','\x8e','\xe8','\x00','\x00','\x02',
    '\xfe','\x3f','\x76','\xf3','\xde','\xfc','\xfa','\x80','\x00','\x00','\x5f','\xdd','\xee','\xdb','\x7b','\xec',
    '\xdf','\x68','\x00','\x00','\x0b','\xfa','\x5d','\xdb','\xb5','\xb6','\xcd','\xfe','\x80','\x00','\x01','\x7f',
    '\x48','\x1f','\x76','\xdb','\x7d','\xff','\xc9','\x00','\x00','\x2f','\xe9','\x63','\xf7','\xed','\xf7','\xff',
    '\xd9','\x00','\x00','\x05','\xfd','\x2e','\xff','\xfd','\x9f','\x6f','\x1d','\x3c','\x10','\x00','\x5f','\xa5',
    '\xcf','\x9f','\x99','\xa1','\x0e','\x3f','\xfc','\x00','\x0b','\xf4','\xbc','\xd9','\xfb','\x4f','\xff','\xff',
    '\xff','\xe0','\x00','\xbe','\x8b','\xdd','\x99','\xf7','\x83','\x03','\xfb','\xfb','\x20','\x0b','\xe8','\xbf',
    '\xbb','\x9e','\x97','\x87','\x9f','\x7b','\x78','\x01','\x7d','\x0b','\xfb','\xf6','\x17','\x6c','\xfd','\xff',
    '\x7f','\xc0','\x17','\xd0','\xbf','\xfe','\x9e','\x77','\x9d','\xde','\xff','\x7e','\x01','\x7d','\x05','\xbe',
    '\x10','\x0f','\x6d','\xad','\xdf','\x6f','\xe2','\x17','\xa0','\x23','\xfe','\x1f','\xf6','\xd2','\xdf','\xef',
    '\xf6','\x01','\x7a','\x06','\xfe','\x87','\xff','\x7b','\x01','\xff','\xf3','\xf0','\x17','\xa0','\x9f','\xd0',
    '\xfb','\xf8','\x36','\x01','\x04','\xdf','\x01','\x7d','\xd7','\xfa','\x3e','\x0f','\x56','\xf6','\xe7','\x9e',
    '\xf0','\x17','\xe2','\xfb','\x47','\x80','\xf5','\x6d','\x6f','\x61','\xaf','\x41','\x7f','\xde','\x74','\xf0',
    '\x8f','\x6e','\xc6','\xd7','\x99','\xe0','\x0b','\xfd','\x9e','\x9c','\x01','\xf7','\xd6','\x6d','\x62','\xde',
    '\x00','\x5f','\xdf','\xd0','\x04','\x3f','\x79','\xe6','\xf6','\x3d','\xc0','\x02','\x7d','\xf2','\x04','\x0f',
    '\xfb','\x0c','\x6e','\x79','\x9c','\x00','\x18','\x2e','\xdf','\x83','\xfd','\xcf','\x39','\x18','\x63','\x80',
    '\x00','\x7d','\x17','\xff','\xff','\x0f','\xff','\xff','\xff','\xf0','\x00','\x00','\x0e','\xff','\xff','\xc6',
    '\x03','\xff','\xff','\xfe','\x08','\x00','\x00','\x1e','\x7f','\xc1','\xfc','\xfc','\xff','\xfe','\x10','\x00',
    '\x01','\x03','\x03','\xf0','\x3f','\xff','\x1e','\x3f','\x88','\x00','\x00','\x00','\xe1','\x3e','\x4f','\x3f',
    '\xc3','\x81','\xf0','\x00','\x00','\x40','\x38','\x07','\xc1','\xc1','\xf8','\x72','\x1e','\x20','\x00','\x00',
    '\x1e','\x08','\xf8','\x30','\x9f','\x0e','\x05','\xc0','\x00','\x00','\x7f','\xd0','\x1f','\x26','\x01','\xf1',
    '\xd0','\x3a','\x40','\x00','\x03','\xe0','\x7f','\xe0','\xc8','\x1e','\x38','\x27','\x80','\x00','\x00','\x00',
    '\x7f','\xf8','\x1c','\x0b','\xc7','\x20','\xf0','\x00','\x00','\x00','\x3f','\xc0','\x07','\x02','\x78','\xe0',
    '\x3e','\x00','\x00','\x02','\x00','\x00','\x01','\xe3','\xfe','\x00','\x7f','\x82','\x00','\x00','\x00','\x00',
    '\x42','\x00','\xff','\x80','\xff','\xe0','\x00','\x00','\x00','\x02','\x00','\x02','\x7c','\x00','\x3f','\xe0',
    '\x80','\x00','\x00','\x04','\x00','\x00','\x00','\x02','\x11','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x01','\x00','\x00','\x00','\x80','\x00','\x00','\x00',};

const unsigned char home_demo[504] PROGMEM  = {
    '\xf0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\xff','\x00','\x00','\x00','\x00','\x01',
    '\xc6','\x30','\x00','\x00','\x0f','\xf0','\x00','\x00','\x00','\x01','\x3d','\xce','\x42','\x00','\x00','\xff',
    '\x00','\x00','\x00','\x00','\x07','\xfd','\xe2','\x00','\x00','\x0f','\xf0','\x00','\x00','\x00','\x08','\xe1',
    '\xfd','\xe0','\x00','\x00','\xff','\x00','\x00','\x00','\x00','\x18','\xc3','\xfd','\x20','\x00','\x0f','\xf0',
    '\x00','\x00','\x00','\x10','\x3f','\x9b','\x80','\x00','\x00','\xf0','\x00','\x00','\x00','\x07','\xff','\xfd',
    '\xf1','\xc0','\x00','\x00','\xe0','\x00','\x00','\x01','\xff','\xff','\xdf','\x78','\x00','\x00','\x7e','\x00',
    '\x00','\x00','\xff','\xfb','\xfc','\xff','\x00','\x00','\x07','\xe0','\x00','\x00','\x3f','\x19','\xff','\xed',
    '\xb2','\x00','\x00','\x7e','\x00','\x00','\x03','\x7f','\xc7','\xfe','\xfe','\x48','\x00','\x07','\xe0','\x00',
    '\x00','\x36','\x2e','\x1f','\xef','\xc2','\x00','\x00','\x7e','\x00','\x00','\x0b','\xcf','\xf1','\xfe','\xdd',
    '\x40','\x00','\x07','\xe0','\x00','\x01','\x19','\x9f','\x1f','\xf7','\x9d','\x00','\x00','\x70','\x00','\x00',
    '\x47','\x37','\xf1','\xff','\x7f','\xe0','\x00','\x00','\xc0','\x00','\x08','\xc6','\xfe','\x1f','\xf7','\x78',
    '\x00','\x00','\x3c','\x00','\x00','\x2f','\xde','\x61','\xff','\x7d','\x80','\x00','\x03','\xc0','\x00','\x04',
    '\xff','\x96','\x1f','\xf6','\xe4','\x00','\x00','\x3c','\x00','\x00','\x3c','\x65','\x61','\xff','\x7e','\x80',
    '\x00','\x03','\xc0','\x00','\x07','\x93','\x56','\x1f','\xf7','\x80','\x00','\x00','\x3c','\x00','\x00','\xfb',
    '\xb5','\xc1','\xff','\x74','\x00','\x00','\x03','\xc0','\x00','\x3d','\x91','\x5f','\xff','\xf7','\xa0','\x00',
    '\x00','\x30','\x00','\x03','\x5a','\xd7','\xff','\xfe','\x30','\x00','\x00','\x00','\xc0','\x00','\x35','\xcd',
    '\xbe','\x3f','\xc8','\x20','\x00','\x00','\x3c','\x00','\x03','\x6a','\x0b','\x0f','\xff','\x80','\x00','\x00',
    '\x03','\xc0','\x00','\x1b','\xc4','\xff','\x0b','\xf0','\x00','\x00','\x00','\x3c','\x00','\x00','\x6f','\xee',
    '\x1f','\xff','\x80','\x00','\x00','\x03','\xc0','\x00','\x07','\xf4','\xff','\x02','\xf8','\x00','\x00','\x00',
    '\x3c','\x00','\x00','\x07','\x8f','\xff','\xdf','\x80','\x00','\x00','\x03','\x00','\x00','\x00','\x6f','\xff',
    '\xff','\xf0','\x00','\x00','\x00','\x0f','\x80','\x00','\x0d','\xff','\xe3','\xfe','\x00','\x00','\x00','\x06',
    '\xa8','\x00','\x01','\xc9','\xff','\xff','\xe0','\x00','\x00','\x00','\xf7','\x00','\x00','\x38','\x2a','\xff',
    '\xfc','\x00','\x00','\x00','\x09','\x20','\x00','\x07','\x3d','\x57','\xff','\xc0','\x00','\x00','\x00','\x92',
    '\x00','\x00','\x7e','\x8a','\x1b','\xfc','\x00','\x00','\x00','\x09','\x20','\x00','\x0f','\xf7','\x75','\xff',
    '\x80','\x00','\x00','\x00','\xf0','\x00','\x00','\x3f','\xe4','\x6b','\xf0','\x00','\x00','\x00','\x00','\xff',
    '\xff','\xff','\xff','\xfb','\x7f','\x00','\x0f','\xff','\xff','\xf0','\x00','\x00','\x0b','\xff','\xff','\xe0',
    '\x01','\x00','\x00','\x00','\x00','\x00','\x00','\x47','\xff','\xfe','\x00','\x20','\x00','\x00','\x00','\x67',
    '\x34','\x02','\x1f','\xff','\xc0','\x04','\x0a','\x6c','\xa0','\x05','\x25','\x40','\x10','\x3f','\xfc','\x00',
    '\x80','\xe8','\xaa','\x00','\x52','\x74','\x01','\x00','\x7f','\x80','\x08','\x0a','\xca','\xa0','\x05','\x25',
    '\x40','\x10','\x01','\xf0','\x00','\x80','\xa8','\xaa','\x00','\x67','\x57','\x01','\x00','\x00','\x00','\x08',
    '\x0a','\xea','\x60','\x00','\x00','\x00','\x10','\x00','\x00','\x00','\x80','\x00','\x00','\x00','\x00','\x00',
    '\x01','\x00','\x00','\x00','\x08','\x00','\x00','\x00',};

const unsigned char menu_demo[504] PROGMEM  = {
    '\xf0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\xff','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x01','\xcf','\xf0','\x00','\x00','\x00','\x14','\xd9','\x40','\x00','\x00','\x1c','\xff',
    '\x00','\x00','\x00','\x01','\xd1','\x54','\x00','\x00','\x01','\xcf','\xf0','\x1f','\xff','\xff','\x95','\x95',
    '\x4f','\xff','\xff','\x9c','\xff','\x00','\x00','\x00','\x01','\x51','\x54','\x00','\x00','\x01','\xcf','\xf0',
    '\x00','\x00','\x00','\x15','\xd4','\xc0','\x00','\x00','\x1c','\xf0','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x01','\xc0','\xe0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x1c','\x7e','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x01','\xc7','\xe0','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x1c','\x7e','\x00','\x00','\xce','\x68','\x00','\x00','\x00','\x00','\x01','\xc7','\xe0','\x00',
    '\x0a','\x4a','\x80','\x00','\x00','\x00','\x00','\x1c','\x7e','\x00','\x00','\xa4','\xe8','\x00','\x00','\x00',
    '\x00','\x00','\x87','\xe0','\x00','\x0a','\x4a','\x80','\x00','\x00','\x00','\x00','\x08','\x70','\x00','\x00',
    '\xce','\xae','\x00','\x00','\x00','\x00','\x00','\x80','\xc0','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x08','\x3c','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x83','\xc0','\x00','\x3f',
    '\xff','\xff','\xff','\xff','\xff','\xc0','\x08','\x3c','\x00','\x03','\xff','\xff','\xff','\xff','\xff','\xfc',
    '\x00','\x83','\xc0','\x00','\x33','\x19','\x7f','\xff','\xff','\xff','\xc0','\x08','\x3c','\x00','\x03','\x5b',
    '\x57','\xff','\xff','\xff','\xfc','\x00','\x83','\xc0','\x00','\x35','\xb1','\x7f','\xff','\xff','\xff','\xc0',
    '\x08','\x30','\x00','\x03','\x5b','\x57','\xff','\xff','\xff','\xfc','\x00','\x80','\xc0','\x00','\x33','\x15',
    '\x1f','\xff','\xff','\xff','\xc0','\x08','\x3c','\x00','\x03','\xff','\xff','\xff','\xff','\xff','\xfc','\x00',
    '\x83','\xc0','\x00','\x3f','\xff','\xff','\xff','\xff','\xff','\xc0','\x08','\x3c','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x83','\xc0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x08',
    '\x3c','\x00','\x00','\xce','\x68','\x00','\x00','\x00','\x00','\x00','\x83','\x00','\x00','\x0a','\x4a','\x80',
    '\x00','\x00','\x00','\x00','\x08','\x0f','\x80','\x00','\xa4','\xe8','\x00','\x00','\x00','\x00','\x00','\x86',
    '\xa8','\x00','\x0a','\x4a','\x80','\x00','\x00','\x00','\x00','\x08','\xf7','\x00','\x00','\xce','\xae','\x00',
    '\x00','\x00','\x00','\x00','\x89','\x20','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x08','\x92',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x89','\x20','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x08','\xf0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x80','\x00',
    '\x00','\x0c','\xe6','\x80','\x00','\x00','\x00','\x00','\x08','\x00','\x00','\x00','\xa4','\xa8','\x00','\x00',
    '\x00','\x00','\x00','\x80','\x00','\x00','\x0a','\x4e','\x80','\x00','\x00','\x00','\x00','\x08','\x00','\x00',
    '\x00','\xa4','\xa8','\x00','\x00','\x00','\x00','\x00','\x80','\x00','\x00','\x0c','\xea','\xe0','\x00','\x00',
    '\x00','\x00','\x08','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x80','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x08','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x80','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x08','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00',};

const unsigned char silvo_juggle[504] PROGMEM  = {
    '\x00','\x00','\x00','\x00','\x06','\x06','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x60','\xf0',
    '\xf0','\x60','\x00','\x00','\x00','\x00','\x00','\x00','\x0f','\x0f','\x0f','\x0f','\x00','\x00','\x00','\x00',
    '\x00','\x00','\x00','\xf0','\x60','\x60','\xf0','\x00','\x00','\x00','\x00','\x00','\x06','\x06','\x00','\x00',
    '\x06','\x06','\x00','\x00','\x00','\x00','\x00','\xf0','\x00','\x00','\x00','\x00','\xf0','\x00','\x00','\x00',
    '\x00','\x0f','\x00','\x06','\x06','\x00','\x0f','\x00','\x00','\x00','\x00','\x00','\x60','\x00','\xf0','\xf0',
    '\x00','\x60','\x00','\x00','\x00','\x00','\x00','\x06','\x0f','\x0f','\x06','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\xf0','\x60','\x60','\xf0','\x00','\x00','\x00','\x00','\x00','\x00','\x0f','\x00','\x00','\x0f',
    '\x00','\x00','\x00','\x00','\x00','\x30','\x00','\x60','\x60','\x60','\x60','\x00','\xc0','\x00','\x00','\x07',
    '\x83','\x00','\x0f','\x0f','\x00','\x0c','\x1e','\x00','\x00','\x00','\x78','\x78','\x00','\xf0','\xf0','\x01',
    '\xe1','\xe0','\x00','\x00','\x03','\x07','\x80','\x06','\x06','\x00','\x1e','\x0c','\x00','\x00','\x00','\x00',
    '\x30','\x30','\x00','\x00','\xc0','\xc0','\x00','\x00','\x00','\x00','\x00','\x07','\x81','\xfc','\x1e','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x78','\x3f','\xe1','\xe0','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x03','\x07','\xff','\x0c','\x00','\x00','\x00','\x00','\x01','\x80','\x00','\x00','\xff','\xf8','\x00','\x00',
    '\x18','\x00','\x00','\x3c','\x30','\x00','\x0f','\x83','\xc0','\x00','\xc3','\xc0','\x00','\x03','\xc7','\x86',
    '\x01','\xe1','\x1c','\x06','\x1e','\x3c','\x00','\x00','\x18','\x78','\xf0','\x18','\x80','\xc0','\xf1','\xe1',
    '\x80','\x00','\x00','\x03','\x0f','\x01','\x71','\xc6','\x0f','\x0c','\x00','\x00','\x00','\x00','\x00','\x60',
    '\x18','\xa2','\xe0','\x60','\x00','\x00','\x00','\x00','\x00','\x00','\x03','\x60','\xc6','\x00','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x00','\x37','\x0e','\x70','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x02',
    '\x08','\x11','\x00','\x00','\x00','\x00','\x00','\x30','\x60','\xc0','\x20','\x84','\x10','\x30','\x60','\xc0',
    '\x00','\x07','\x8f','\x1e','\x03','\x10','\x02','\x07','\x8f','\x1e','\x00','\x00','\x78','\xf1','\xe0','\x11',
    '\xca','\x40','\x78','\xf1','\xe0','\x00','\x03','\x36','\x8c','\x01','\x80','\x04','\x03','\x16','\xcc','\x00',
    '\x00','\x03','\x98','\x00','\x09','\xc0','\x80','\x01','\x9c','\x00','\x00','\x00','\x1f','\x00','\x00','\xa2',
    '\x10','\x00','\x0f','\x80','\x00','\x00','\x00','\x60','\x00','\x0f','\xf1','\x00','\x00','\x60','\x00','\x00',
    '\x00','\x00','\x00','\x00','\x60','\x20','\x00','\x00','\x00','\x00','\x00','\x00','\x60','\x00','\x03','\x04',
    '\x00','\x00','\x60','\x00','\x00','\x00','\x06','\x00','\x00','\x1f','\x80','\x00','\x06','\x00','\x00','\x00',
    '\x00','\x30','\x00','\x3c','\x03','\xc0','\x00','\xc0','\x00','\x00','\x00','\x03','\x00','\x3f','\xcf','\x3f',
    '\xc0','\x0c','\x00','\x00','\x00','\x00','\x18','\x3f','\xc1','\xf8','\x3f','\xc1','\x80','\x00','\x00','\x00',
    '\x01','\xbf','\xc0','\x3f','\xc0','\x3f','\xd8','\x00','\x00','\x00','\x00','\x0b','\xc0','\x03','\xfc','\x00',
    '\x3d','\x00','\x00','\x00','\x00','\x00','\x00','\x05','\x00','\x0a','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\x00','\xdb','\xfd','\xb0','\x00','\x00','\x00','\x00','\x00','\x00','\x00','\x18','\xdf','\xb1','\x80',
    '\x00','\x00','\x00','\x00','\x00','\x00','\x03','\x86','\x66','\x1c','\x00','\x00','\x00','\x00','\x00','\x00',
    '\x00','\xfc','\x00','\x03','\xf0','\x00','\x00','\x00',};

//-----------------------------------------------------------------------------
// Serial
//-----------------------------------------------------------------------------
void serial_init() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("Serial ready");
}

//-----------------------------------------------------------------------------
// Logging
//-----------------------------------------------------------------------------
#define LOG(msg)  Serial.println(F(msg))

//-----------------------------------------------------------------------------
// Random
//-----------------------------------------------------------------------------
void rng_init() {
    // not great, got a better idea?
    // got a better idea??
    randomSeed(analogRead(0));
}
uint32_t rng_random(uint32_t min, uint32_t max) {
    return (uint32_t) random((long)min, (long)(max + 1));
}

//-----------------------------------------------------------------------------
// Interrupt timers (virtual)
//-----------------------------------------------------------------------------
#define TMR_INTERRUPTS_MAX 4
static uint8_t interrupts[VINTC_CALC_DATA_SIZE(TMR_INTERRUPTS_MAX)] = {0};
uint32_t VINTC_API interrupt_get_tick_count(void *ctx) {
    ctx;
    return (uint32_t)millis();
}
void interrupts_init() {
    // Virtual interrupts
    if (!vintc_init(interrupts, sizeof(interrupts)))
    {
        LOG("init interrupts failed");
    }
    if (!vintc_set_get_tick_count(interrupts, interrupt_get_tick_count, NULL))
    {
        LOG("interrupts set get tick count failed");
    }
}
void interrupts_tick() {
    if(!vintc_run_loop(interrupts)) {
        LOG("interrupts run loop failed");
    }
}

//-----------------------------------------------------------------------------
// Buttons
//-----------------------------------------------------------------------------
#define BUTTON_KEY_LEFT     0
#define BUTTON_KEY_OK       1
#define BUTTON_KEY_RIGHT    2
#define BUTTON_DOWN         true
#define BUTTON_UP           false
static uint8_t button_bits;
typedef void (*ButtonPressFn)(void *ctx, uint32_t key, bool down, uint32_t duration);
static ButtonPressFn button_cb = NULL;
static void *button_ctx = NULL;
uint32_t button_left_duration = 0;
uint32_t button_ok_duration = 0;
uint32_t button_right_duration = 0;
void button_tick(void *ctx) {
    int val = 0;
    val = digitalRead(BUTTON_LEFT);
    if (val == HIGH) {
      if (0 == (button_bits & 0x1)) {
        if (NULL != button_cb) {
          button_left_duration = ((uint32_t)millis()) - button_left_duration;
          button_cb(button_ctx, BUTTON_KEY_LEFT, BUTTON_UP, button_left_duration);
        }
      }
      button_bits |= 0x1;
    } else {
      if (0 != (button_bits & 0x1)) {
        if (NULL != button_cb) {
          button_left_duration = (uint32_t)millis();
          button_cb(button_ctx, BUTTON_KEY_LEFT, BUTTON_DOWN, 0);
        }
      }
      button_bits &=~ 0x1;
    }
    val = digitalRead(BUTTON_OK);
    if (val == HIGH) {
      if (0 == (button_bits & 0x2)) {
        if (NULL != button_cb) {
          button_ok_duration = ((uint32_t)millis()) - button_ok_duration;
          button_cb(button_ctx, BUTTON_KEY_OK, BUTTON_UP, button_ok_duration);
        }
      }
      button_bits |= 0x2;
    } else {
      if (0 != (button_bits & 0x2)) {
        if (NULL != button_cb) {
          button_ok_duration = (uint32_t)millis();
          button_cb(button_ctx, BUTTON_KEY_OK, BUTTON_DOWN, 0);
        }
      }
      button_bits &=~ 0x2;
    }
    val = digitalRead(BUTTON_RIGHT);
    if (val == HIGH) {
      if (0 == (button_bits & 0x4)) {
        if (NULL != button_cb) {
          button_right_duration = ((uint32_t)millis()) - button_right_duration;
          button_cb(button_ctx, BUTTON_KEY_RIGHT, BUTTON_UP, button_right_duration);
        }
      }
      button_bits |= 0x4;
    } else {
      if (0 != (button_bits & 0x4)) {
        if (NULL != button_cb) {
          button_right_duration = (uint32_t)millis();
          button_cb(button_ctx, BUTTON_KEY_RIGHT, BUTTON_DOWN, 0);
        }
      }
      button_bits &=~ 0x4;
    }
}
void button_callback(void *func, void *ctx) {
  button_cb = func;
  button_ctx = ctx;
}
void button_init() {
  pinMode(BUTTON_LEFT, INPUT);
  pinMode(BUTTON_OK, INPUT);
  pinMode(BUTTON_RIGHT, INPUT);

  // check button interrupt
  if (!vintc_set_interrupt(interrupts, 20, button_tick, NULL, NULL))
  {
      LOG("button set interrupts failed");
  }
}

//-----------------------------------------------------------------------------
// NOKIA 5110 screen
//-----------------------------------------------------------------------------
Nokia_LCD nokia_5110(NOKIA_5110_CLK, NOKIA_5110_DIN, NOKIA_5110_DC, NOKIA_5110_CE, NOKIA_5110_RST);
#define NOKIA_SCREEN_WIDTH            84
#define NOKIA_SCREEN_HEIGHT           48
#define NOKIA_SCREEN_PIXELS           (NOKIA_SCREEN_WIDTH*NOKIA_SCREEN_HEIGHT)
#define NOKIA_SCREEN_BYTES            (NOKIA_SCREEN_PIXELS/8)
#define NOKIA_SCREEN_TEXT_ROWS        6
#define NOKIA_SCREEN_PIXELS_PER_ROW   (NOKIA_SCREEN_PIXELS/NOKIA_SCREEN_TEXT_ROWS)
#define NOKIA_SCREEN_BYTES_PER_ROW    (NOKIA_SCREEN_BYTES/NOKIA_SCREEN_TEXT_ROWS)
static unsigned char nokia_screen_buffer[NOKIA_SCREEN_BYTES];
void nokia_draw_clear() {
    memset(nokia_screen_buffer, 0, sizeof(nokia_screen_buffer));
}
void nokia_draw_black() {
    memset(nokia_screen_buffer, 0xff, sizeof(nokia_screen_buffer));
}
void nokia_draw_pixel(int16_t x, int16_t y, bool black) {
    int16_t pixel = (y * NOKIA_SCREEN_WIDTH) + x;
    int16_t row = pixel / NOKIA_SCREEN_PIXELS_PER_ROW;
    int16_t pixel_in_row = pixel % NOKIA_SCREEN_PIXELS_PER_ROW;
    int16_t column = pixel_in_row % NOKIA_SCREEN_WIDTH;
    int16_t bit = pixel_in_row / NOKIA_SCREEN_WIDTH;
    int16_t index = (row * NOKIA_SCREEN_BYTES_PER_ROW) + column;
    if ((index < 0) || (index >= NOKIA_SCREEN_BYTES)) {
      return;
    }
    if (black) {
        nokia_screen_buffer[index] |= (1 << bit);
    } else {
        nokia_screen_buffer[index] &=~ (1 << bit);
    }
}
void nokia_draw_img(void *data) {
     for (uint32_t y = 0; y < 48; y++) {
        for (uint32_t x = 0; x < 84; x++) {
          uint32_t index = (y * 84) + x;
          uint32_t index_byte = index / 8;
          uint32_t index_bit = 7 - (index % 8);
          uint8_t value = pgm_read_byte(&(((uint8_t*)data)[index_byte]));
          nokia_draw_pixel(x, y, ((value >> index_bit) & 0x1) != 0 ? true : false);
      }
    }
}
void nokia_swap_fb() {
    nokia_5110.setCursor(0, 0);
    nokia_5110.draw(nokia_screen_buffer,
            sizeof(nokia_screen_buffer) / sizeof(nokia_screen_buffer[0]),
            false);
}
void nokia_init() {
    memset(nokia_screen_buffer, 0, NOKIA_SCREEN_BYTES);
    nokia_5110.begin();
    nokia_5110.setContrast(60);  // Good values are usualy between 40 and 60
    nokia_draw_black();
    nokia_swap_fb();
    delay(1000);
}

//-----------------------------------------------------------------------------
// Screen
//-----------------------------------------------------------------------------
#define SCREEN_COLOR_WHITE     0
#define SCREEN_COLOR_BLACK     1
#define SCREEN_WIDTH    NOKIA_SCREEN_WIDTH
#define SCREEN_HEIGHT   NOKIA_SCREEN_HEIGHT
static VGFX_CANVAS_2D_DATA screen;
void VGFX_API screen_draw_clear_api(void *ctx, uint32_t color) {
    nokia_draw_clear();
}
void VGFX_API screen_draw_pixel_api(void *ctx, uint32_t x, uint32_t y, uint32_t color) {
    nokia_draw_pixel(x, y, color > 0 ? true : false);
}
void VGFX_API screen_draw_char_api(void *ctx, uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bg) {
    f46c_draw_char(x, y, c, color, bg, screen_draw_pixel_api, NULL);
}
void screen_init() {
    // Add all the drawing APIs we need
    memset(&screen, 0, sizeof(screen));
    if (!vg2d_init(&screen, sizeof(VGFX_CANVAS_2D_DATA))) {
        LOG("screen canvas2d init failed");
    }
    if (!vg2d_set_draw_clear(&screen, screen_draw_clear_api, NULL)) {
        LOG("screen canvas2d set draw clear failed");
    }
    if (!vg2d_set_draw_pixel(&screen, screen_draw_pixel_api, NULL)) {
        LOG("screen canvas2d set draw pixel failed");
    }
    if (!vg2d_set_draw_char(&screen, screen_draw_char_api, NULL, F46C_WIDTH, F46C_HEIGHT)) {
        LOG("screen canvas2d set draw char failed");
    }
}
void screen_draw_img(void *data) {
  nokia_draw_img(data);
}
void screen_swap_fb() {
    nokia_swap_fb();
}
void screen_draw_clear() {
    nokia_draw_clear();
}
void screen_fill_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    vg2d_fill_rect(&screen, x, y, w, h, color);
}
void screen_draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color) {
    vg2d_draw_rect(&screen, x, y, w, h, color);
}
void screen_draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint32_t color) {
    vg2d_draw_line(&screen, x0, y0, x1, y1, color);
}
void screen_draw_pixel(uint32_t x, uint32_t y, uint32_t color) {
    nokia_draw_pixel(x, y, color > 0 ? true : false);
}
void screen_draw_char(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bg) {
    vg2d_draw_char(&screen, x, y, c, color, bg);
}
void screen_draw_string(uint32_t x, uint32_t y, const char *s, uint32_t color, uint32_t bg) {
    vg2d_draw_string(&screen, x, y, s, color, bg);
}

//-----------------------------------------------------------------------------
// Dialer
//-----------------------------------------------------------------------------
static uint8_t dialer_x = 0;
static uint8_t dialer_y = 0;
static char dialer_number[13] = {0};
#define DIALER_BUTTON_OFFSET_X  18
#define DIALER_BUTTON_OFFSET_Y  9
#define DIALER_BUTTON_WIDTH     15
#define DIALER_BUTTON_HEIGHT    9
char dialer_char(uint8_t x, uint8_t y) {
  char value = '0';
  value = '1' + ((y * 3) + x);
  if (10 == (value - '0')) {
    value = '*';
  }
  if (11 == (value - '0')) {
    value = '0';
  }
  if (12 == (value - '0')) {
    value = '#';
  }
  return value;
}
void dialer_draw() {
  uint32_t btn_x = 0;
  uint32_t btn_y = 0;
  uint32_t color = 0;
  uint32_t bg = 0;
  char label[4] = {0};
  screen_draw_clear();
  screen_draw_string(DIALER_BUTTON_OFFSET_X, 1, dialer_number, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
  for (uint8_t x = 0; x < 3; x++) {
    for (uint8_t y = 0; y < 4; y++) {
      btn_x = DIALER_BUTTON_OFFSET_X + (x * (DIALER_BUTTON_WIDTH+1));
      btn_y = DIALER_BUTTON_OFFSET_Y + (y * (DIALER_BUTTON_HEIGHT+1));
      if ((x == dialer_x) && (y == dialer_y)) {
        screen_fill_rect(btn_x, btn_y, DIALER_BUTTON_WIDTH, DIALER_BUTTON_HEIGHT, SCREEN_COLOR_BLACK);
        color = SCREEN_COLOR_WHITE;
        bg = SCREEN_COLOR_BLACK;
      } else {
        screen_draw_rect(btn_x, btn_y, DIALER_BUTTON_WIDTH, DIALER_BUTTON_HEIGHT, SCREEN_COLOR_BLACK);
        color = SCREEN_COLOR_BLACK;
        bg = SCREEN_COLOR_WHITE;
      }
      label[0] = ' ';
      label[1] = dialer_char(x, y);
      label[2] = ' ';
      label[3] = '\0';
      screen_draw_pixel(btn_x, btn_y, SCREEN_COLOR_WHITE);
      screen_draw_pixel(btn_x + DIALER_BUTTON_WIDTH - 1, btn_y, SCREEN_COLOR_WHITE);
      screen_draw_pixel(btn_x, btn_y + DIALER_BUTTON_HEIGHT - 1, SCREEN_COLOR_WHITE);
      screen_draw_pixel(btn_x + DIALER_BUTTON_WIDTH - 1, btn_y + DIALER_BUTTON_HEIGHT - 1, SCREEN_COLOR_WHITE);
      screen_draw_string(btn_x + 2, btn_y + 2, label, color, bg);
    }
  }
}
void dialer_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
  if (BUTTON_DOWN == down) {
    switch(key) {
      case BUTTON_KEY_LEFT:
        dialer_y++;
        if (dialer_y >= 4) {
          dialer_y = 0;
        }
        dialer_draw();
        screen_swap_fb();
        break;
      case BUTTON_KEY_RIGHT:
        dialer_x++;
        if (dialer_x >= 3) {
          dialer_x = 0;
        }
        dialer_draw();
        screen_swap_fb();
        break;
      case BUTTON_KEY_OK:
        if (strlen(dialer_number) < (sizeof(dialer_number) - 1)) {
          dialer_number[strlen(dialer_number)] = dialer_char(dialer_x, dialer_y);
        }
        dialer_draw();
        screen_swap_fb();
        break;
      default:
        break;
    }
  }
}
void dialer_init() {
  dialer_x = 0;
  dialer_y = 0;
  memset(dialer_number, 0, sizeof(dialer_number));
  button_callback(dialer_button_press, NULL);
}

//-----------------------------------------------------------------------------
// Snake
//-----------------------------------------------------------------------------
#define SNAKE_SQUARE_SIZE   3
#define SNAKE_GRID_OFFSET   2
#define SNAKE_GRID_WIDTH    ((SCREEN_WIDTH - (SNAKE_GRID_OFFSET * 2)) / SNAKE_SQUARE_SIZE)
#define SNAKE_GRID_HEIGHT   ((SCREEN_HEIGHT - (SNAKE_GRID_OFFSET * 2)) / SNAKE_SQUARE_SIZE)
#define SNAKE_GAME_MEM_SIZE (SNKC_CALC_DATA_SIZE(SNAKE_GRID_WIDTH,SNAKE_GRID_HEIGHT))
static uint8_t *snkc_mem = NULL;
static uint8_t snake_direction = 0xff;
void SNKC_API snake_draw_clear(void *ctx) {
    if (!vg2d_draw_clear(&screen, SCREEN_COLOR_WHITE)) {
        LOG("snake draw clear failed");
    }
}
void SNKC_API snake_draw_snake(void *ctx, int16_t x, int16_t y) {
    x = SNAKE_GRID_OFFSET + (x * SNAKE_SQUARE_SIZE);
    y = SNAKE_GRID_OFFSET + (y * SNAKE_SQUARE_SIZE);
    if (!vg2d_fill_rect(&screen, x, y, SNAKE_SQUARE_SIZE, SNAKE_SQUARE_SIZE, SCREEN_COLOR_BLACK)) {
        LOG("snake draw pixel failed");
    }
}
void SNKC_API snake_draw_apple(void *ctx, int16_t x, int16_t y) {
    x = SNAKE_GRID_OFFSET + (x * SNAKE_SQUARE_SIZE);
    y = SNAKE_GRID_OFFSET + (y * SNAKE_SQUARE_SIZE);
    if (!vg2d_draw_rect(&screen, x, y, SNAKE_SQUARE_SIZE, SNAKE_SQUARE_SIZE, SCREEN_COLOR_BLACK)) {
        LOG("snake draw apple failed");
    }
}
int16_t SNKC_API snake_random(void *ctx, int16_t min, int16_t max) {
    return (int16_t)rng_random((uint32_t)min, (uint32_t)max);
}
void snake_draw_end() {
    vg2d_draw_rect(&screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_BLACK);
    screen_swap_fb();
}
void VINTC_API snake_tick(void *ctx) {
    if(!snkc_tick(snkc_mem)) {
        LOG("snake tick failed");
    }
    snake_draw_end();
}
void snake_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
  int32_t new_direction = (int32_t)snake_direction;
  if (BUTTON_UP == down) {
    return;
  }
  switch(key) {
    case BUTTON_KEY_LEFT:
      if (0xff == snake_direction) {
        new_direction = 0;
      } else {
        new_direction--;
      }
      break;
    case BUTTON_KEY_RIGHT:
      if (0xff == snake_direction) {
        new_direction = 2;
      } else {
        new_direction++;
      }
      break;
    default:
      return;
      break;
  }
  if (new_direction < 0) {
    new_direction = 3;
  }
  if (new_direction > 3) {
    new_direction = 0;
  }
  snake_direction = (uint8_t)new_direction;
  switch(snake_direction) {
    case 0:
      if(!snkc_key_left(snkc_mem)) {
          LOG("snake key left failed");
      }
      snake_draw_end();
      break;
    case 1:
      if(!snkc_key_up(snkc_mem)) {
          LOG("snake key up failed");
      }
      snake_draw_end();
      break;
    case 2:
      if(!snkc_key_right(snkc_mem)) {
          LOG("snake key right failed");
      }
      snake_draw_end();
      break;
    case 3:
      if(!snkc_key_down(snkc_mem)) {
          LOG("snake key down failed");
      }
      snake_draw_end();
      break;
    default:
      break;
  }
}
void snake_init(void *mem) {
    snkc_mem = (uint8_t*)mem;
    snake_direction = 0xff;

    // Snake game
    if (!snkc_init(snkc_mem, SNAKE_GAME_MEM_SIZE))
    {
        LOG("snake init failed");
    }
    if (!snkc_set_grid(snkc_mem, SNAKE_GRID_WIDTH, SNAKE_GRID_HEIGHT))
    {
        LOG("snake set grid failed");
    }

     // Snake will draw on the screen
    if (!snkc_set_draw_clear(snkc_mem, snake_draw_clear, &screen)) {
        LOG("snake set draw empty failed");
    }
    if (!snkc_set_draw_snake(snkc_mem, snake_draw_snake, &screen)) {
        LOG("snake set draw snake failed");
    }
    if (!snkc_set_draw_apple(snkc_mem, snake_draw_apple, &screen)) {
        LOG("snake set draw apple failed");
    }
    if (!snkc_set_random(snkc_mem, snake_random, NULL)) {
        LOG("snake set random failed");
    }

    // Snake frame rate
    if (!vintc_set_interrupt(interrupts, 1000/15, snake_tick, snkc_mem, NULL))
    {
        LOG("snake set interrupts failed");
    }

    // Snake buttons
    button_callback(snake_button_press, NULL);

    // Game reset
    if (!snkc_reset(snkc_mem))
    {
        LOG("snake reset failed");
    }
}

//-----------------------------------------------------------------------------
// Tetris
//-----------------------------------------------------------------------------
#define TETRIS_SQUARE_SIZE    2
#define TETRIS_GRID_OFFSET    2
#define TETRIS_GRID_WIDTH     10
#define TETRIS_GRID_HEIGHT    ((SCREEN_HEIGHT - (TETRIS_GRID_OFFSET * 2)) / TETRIS_SQUARE_SIZE)
#define TETRIS_DISPLAY_WIDTH  ((TETRIS_GRID_OFFSET * 2) + (TETRIS_GRID_WIDTH * TETRIS_SQUARE_SIZE))
#define TETRIS_DISPLAY_HEIGHT ((TETRIS_GRID_OFFSET * 2) + (TETRIS_GRID_HEIGHT * TETRIS_SQUARE_SIZE))
#define TETRIS_DISPLAY_OFFSET ((SCREEN_WIDTH / 2) - (TETRIS_DISPLAY_WIDTH / 2))
#define TETRIS_NEXT_PIECE_OFFSET 2
#define TETRIS_NEXT_PIECE_SQUARE_SIZE 4
#define TETRIS_GAME_MEM_SIZE (TTRS_CALC_DATA_SIZE(TETRIS_GRID_WIDTH, TETRIS_GRID_HEIGHT))
static uint8_t *ttrs_mem = NULL;
void TTRS_API tetris_draw_clear(void *ctx) {
    if (!vg2d_draw_clear(&screen, SCREEN_COLOR_WHITE)) {
        LOG("tetris draw clear failed");
    }
}
void TTRS_API tetris_draw_piece(void *ctx, int16_t x, int16_t y, TTRS_PIECE_TYPE piece) {
    x = TETRIS_GRID_OFFSET + (x * TETRIS_SQUARE_SIZE) + TETRIS_DISPLAY_OFFSET;
    y = TETRIS_GRID_OFFSET + (y * TETRIS_SQUARE_SIZE);
    if (!vg2d_fill_rect(&screen, x, y, TETRIS_SQUARE_SIZE, TETRIS_SQUARE_SIZE, SCREEN_COLOR_BLACK)) {
        LOG("tetris draw piece failed");
    }
}
void TTRS_API tetris_draw_next_piece(void *ctx, int16_t x, int16_t y, TTRS_PIECE_TYPE piece) {
    x = TETRIS_NEXT_PIECE_OFFSET + (x * TETRIS_NEXT_PIECE_SQUARE_SIZE);
    y = TETRIS_NEXT_PIECE_OFFSET + (y * TETRIS_NEXT_PIECE_SQUARE_SIZE);
    if (!vg2d_draw_rect(&screen, x, y, TETRIS_NEXT_PIECE_SQUARE_SIZE, TETRIS_NEXT_PIECE_SQUARE_SIZE, SCREEN_COLOR_BLACK)) {
        LOG("tetris draw piece failed");
    }
}
void TTRS_API tetris_draw_block(void *ctx, int16_t x, int16_t y) {
    x = TETRIS_GRID_OFFSET + (x * TETRIS_SQUARE_SIZE) + TETRIS_DISPLAY_OFFSET;
    y = TETRIS_GRID_OFFSET + (y * TETRIS_SQUARE_SIZE);
    if (!vg2d_fill_rect(&screen, x, y, TETRIS_SQUARE_SIZE, TETRIS_SQUARE_SIZE, SCREEN_COLOR_BLACK)) {
        LOG("tetris draw apple failed");
    }
}
int16_t TTRS_API tetris_random(void *ctx, int16_t min, int16_t max) {
    return (int16_t)rng_random((uint32_t)min, (uint32_t)max);
}
void TTRS_API tetris_game_over(void *ctx) {
    if (!ttrs_reset(ttrs_mem))
    {
        LOG("tetris reset (2) failed");
    }
}
void tetris_draw_end() {
    vg2d_draw_rect(&screen, TETRIS_DISPLAY_OFFSET, 0, TETRIS_DISPLAY_WIDTH, TETRIS_DISPLAY_HEIGHT, SCREEN_COLOR_BLACK);
    screen_swap_fb();
}
void VINTC_API tetris_tick(void *ctx) {
    screen_draw_clear();
    if(!ttrs_tick(ttrs_mem)) {
        LOG("tetris tick failed");
    }
    tetris_draw_end();
}
void tetris_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
  if (BUTTON_DOWN == down) {
    switch(key) {
      case BUTTON_KEY_LEFT:
        if(!ttrs_key_left(ttrs_mem)) {
            LOG("tetris key left failed");
        }
        tetris_draw_end();
        break;
      case BUTTON_KEY_RIGHT:
        if(!ttrs_key_right(ttrs_mem)) {
            LOG("tetris key right failed");
        }
        tetris_draw_end();
        break;
      default:
        break;
    }
  }
  if ((BUTTON_UP == down) && (BUTTON_KEY_OK == key)) {
    if (duration > 1000) {
        if(!ttrs_key_drop(ttrs_mem)) {
            LOG("tetris key drop failed");
        }
        tetris_draw_end();
    } else {
        if(!ttrs_key_rotate(ttrs_mem)) {
            LOG("tetris key rotate failed");
        }
        tetris_draw_end();
    }
  }
}
void tetris_init(void *mem) {
    ttrs_mem = (uint8_t*)mem;

    // Tetris game
    if (!ttrs_init(ttrs_mem, TETRIS_GAME_MEM_SIZE))
    {
        LOG("tetris init failed");
    }
    if (!ttrs_set_grid(ttrs_mem, TETRIS_GRID_WIDTH, TETRIS_GRID_HEIGHT))
    {
        LOG("tetris set grid failed");
    }

     // Tetris will draw on the screen
    if (!ttrs_set_draw_clear(ttrs_mem, tetris_draw_clear, &screen)) {
        LOG("tetris set draw empty failed");
    }
    if (!ttrs_set_draw_piece(ttrs_mem, tetris_draw_piece, &screen)) {
        LOG("tetris set draw piece failed");
    }
    if (!ttrs_set_draw_next_piece(ttrs_mem, tetris_draw_next_piece, &screen)) {
        LOG("tetris set draw next piece failed");
    }
    if (!ttrs_set_draw_block(ttrs_mem, tetris_draw_block, &screen)) {
        LOG("tetris set draw block failed");
    }
    if (!ttrs_set_random(ttrs_mem, tetris_random, NULL)) {
        LOG("tetris set random failed");
    }
    if (!ttrs_set_game_over(ttrs_mem, tetris_game_over, NULL)) {
        LOG("tetris set game over failed");
    }

    // Tetris buttons
    button_callback(tetris_button_press, NULL);

    // Tetris frame rate
    if (!vintc_set_interrupt(interrupts, 700, tetris_tick, ttrs_mem, NULL))
    {
        LOG("tetris set interrupts failed");
    }

    // Tetris reset
    if (!ttrs_reset(ttrs_mem))
    {
        LOG("tetris reset failed");
    }
}

//-----------------------------------------------------------------------------
// Text viewer
//-----------------------------------------------------------------------------
#define TEXT_VIEWER_MEM_SIZE (VWRC_CALC_DATA_SIZE(84))
static uint8_t *vwrc_mem = NULL;
static uint32_t vwrc_data_size = 0;
static const char *vwrc_c_str = 0;
int32_t VWRC_API viewer_read_c_str_api(void *ctx, uint32_t offset, char *buffer, uint32_t buffer_size) {
    uint32_t read_size = 0;
    if (buffer_size >= vwrc_data_size) {
        // can't read more than the amount of data we have
        buffer_size = vwrc_data_size;
    }
    if (offset >= vwrc_data_size) {
        // read out of bounds
        return -1;
    }
    if ((offset + buffer_size) > vwrc_data_size) {
        // reading the last few bytes at the end
        buffer_size = vwrc_data_size - offset;
    }
    if (0 == buffer_size) {
        // no data was requested
        return 0;
    }
    memcpy(buffer, vwrc_c_str + offset, buffer_size);
    return (int32_t)buffer_size;
}
void VWRC_API viewer_calc_string_view_api(void *ctx, const char *str, uint32_t *width, uint32_t *height) {
    if (width) {
        *width = (uint32_t)(4 * strlen(str)); // 4x6 font used
    }
    if (height) {
        *height = 6; // 4x6 font used
    }
}
void VWRC_API viewer_draw_string_api(void *ctx, uint32_t x, uint32_t y, const char *str) {
    screen_draw_string(x, y, str, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
}
void viewer_draw() {
    uint32_t row = 0;
    uint32_t row_count = 0;
    uint32_t rows_per_view = 0;
    uint32_t y = 0;
    uint32_t h = 0;
    screen_draw_clear();
    if (!vwrc_draw_view(vwrc_mem)) {
        LOG("failed to draw text viewer");
    }
    if (!vwrc_get_row(vwrc_mem, &row)) {
        LOG("failed to get row");
    }
    if (!vwrc_get_row_count(vwrc_mem, &row_count)) {
        LOG("failed to get row count");
    }
    if (!vwrc_get_rows_per_view(vwrc_mem, &rows_per_view)) {
        LOG("failed to get rows per view");
    }
    if (rows_per_view < row_count) {
      // progress bars required
      y = (SCREEN_HEIGHT * row) / row_count;
      h = (SCREEN_HEIGHT * rows_per_view) / row_count;
      screen_fill_rect(SCREEN_WIDTH - 2, y, 2, h, SCREEN_COLOR_BLACK);
    }
}
void viewer_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
  if (BUTTON_DOWN == down) {
    switch(key) {
      case BUTTON_KEY_LEFT:
        if (!vwrc_scroll_up(vwrc_mem)) {
            LOG("viewer scroll up failed");
        }
        viewer_draw();
        screen_swap_fb();
        break;
      case BUTTON_KEY_RIGHT:
        if (!vwrc_scroll_down(vwrc_mem)) {
            LOG("viewer scroll down failed");
        }
        viewer_draw();
        screen_swap_fb();
        break;
      default:
        break;
    }
  }
}
void viewer_init(void *mem) {
    vwrc_mem = (uint8_t*)mem;

    // reset
    vwrc_data_size = 0;
    vwrc_c_str = NULL;

    // init
    if (!vwrc_init(vwrc_mem, TEXT_VIEWER_MEM_SIZE)) {
        LOG("failed to init text viewer");
    }

    // view size (2 pixels for scroll bar)
    if (!vwrc_set_view(vwrc_mem, SCREEN_WIDTH - 2, SCREEN_HEIGHT)) {
        LOG("failed to set view in text viewer");
    }

    // how to calculate the pixel size of a string
    if (!vwrc_set_calc_string_view(vwrc_mem, viewer_calc_string_view_api, NULL)) {
        LOG("failed to set calc string view in text viewer");
    }

    // how to draw the text in the view
    if (!vwrc_set_draw_string(vwrc_mem, viewer_draw_string_api, NULL)) {
        LOG("failed to set draw string in text viewer");
    }

    // Viewer buttons
    button_callback(viewer_button_press, NULL);
}

void viewer_c_str(const char *text) {
    vwrc_c_str = text;
    vwrc_data_size = (uint32_t)strlen(text);
    if (!vwrc_set_text(vwrc_mem, vwrc_data_size, viewer_read_c_str_api, NULL)) {
        LOG("failed to set data for text viewer");
    }
}

//-----------------------------------------------------------------------------
// QR Code
//-----------------------------------------------------------------------------
#define qrcodegen_MAX_VERSION   6
static uint8_t qrcode[qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_MAX_VERSION)];
static uint8_t tempBuffer[qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_MAX_VERSION)];
void qr_code_draw(const char *text) {
  LOG("Begin");
  enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
  bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
    qrcodegen_VERSION_MIN, qrcodegen_MAX_VERSION, qrcodegen_Mask_AUTO, true);
  if (ok) {
    LOG("OK");
  } else {
    LOG("QRERR");
    return;
  }

  int size = qrcodegen_getSize(qrcode);
  int x_offset = (SCREEN_WIDTH / 2) - (size / 2);
  int y_offset = (SCREEN_HEIGHT / 2) - (size / 2);
  nokia_draw_clear();
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      nokia_draw_pixel(x_offset + x, y_offset + y, qrcodegen_getModule(qrcode, x, y) ? 1 : 0);
    }

  }
  LOG("Done");
}

//-----------------------------------------------------------------------------
// Power and Signal
//-----------------------------------------------------------------------------
void screen_power_display(int level) {
  screen_fill_rect(SCREEN_WIDTH - 6, 0, 6, 38, SCREEN_COLOR_WHITE);
  if (level >= 4) {
    screen_fill_rect(SCREEN_WIDTH - 4, 0, 4, 7, SCREEN_COLOR_BLACK);
  }
  if (level >= 3) {
    screen_fill_rect(SCREEN_WIDTH - 3, 8, 3, 7, SCREEN_COLOR_BLACK);
  }
  if (level >= 2) {
    screen_fill_rect(SCREEN_WIDTH - 2, 16, 2, 7, SCREEN_COLOR_BLACK);
  }
  if (level >= 1) {
    screen_fill_rect(SCREEN_WIDTH - 2, 24, 2, 6, SCREEN_COLOR_BLACK);
  }
  screen_fill_rect(SCREEN_WIDTH - 3, 31, 2, 2, SCREEN_COLOR_BLACK);
  screen_draw_rect(SCREEN_WIDTH - 4, 32, 4, 5, SCREEN_COLOR_BLACK);
}

void screen_signal_display(int level) {
  screen_fill_rect(0, 0, 6, 38, SCREEN_COLOR_WHITE);
  if (level >= 4) {
    screen_fill_rect(0, 0, 4, 7, SCREEN_COLOR_BLACK);
  }
  if (level >= 3) {
    screen_fill_rect(0, 8, 3, 7, SCREEN_COLOR_BLACK);
  }
  if (level >= 2) {
    screen_fill_rect(0, 16, 2, 7, SCREEN_COLOR_BLACK);
  }
  if (level >= 1) {
    screen_fill_rect(0, 24, 2, 6, SCREEN_COLOR_BLACK);
  }
  screen_draw_line(0, 31, 4, 31, SCREEN_COLOR_BLACK);
  screen_draw_line(2, 31, 2, 31 + 5, SCREEN_COLOR_BLACK);
  screen_draw_line(0, 32, 1, 32 + 1, SCREEN_COLOR_BLACK);
  screen_draw_pixel(3, 33, SCREEN_COLOR_BLACK);
  screen_draw_pixel(4, 32, SCREEN_COLOR_BLACK);
}

//-----------------------------------------------------------------------------
// Tiny Menu
//-----------------------------------------------------------------------------
#define MENU_MEM_SIZE (TMNU_CALC_DATA_SIZE())
static uint8_t *tmnu_mem = NULL;


//-----------------------------------------------------------------------------
// Game and application dynamic memory
//-----------------------------------------------------------------------------
#define GAME_APP_MEM_SIZE_0 0
#define GAME_APP_MEM_SIZE_1 (SNAKE_GAME_MEM_SIZE > GAME_APP_MEM_SIZE_0 ? SNAKE_GAME_MEM_SIZE : GAME_APP_MEM_SIZE_0)
#define GAME_APP_MEM_SIZE_2 (TETRIS_GAME_MEM_SIZE > GAME_APP_MEM_SIZE_1 ? TETRIS_GAME_MEM_SIZE : GAME_APP_MEM_SIZE_1)
#define GAME_APP_MEM_SIZE_3 (MENU_MEM_SIZE > GAME_APP_MEM_SIZE_2 ? MENU_MEM_SIZE : GAME_APP_MEM_SIZE_2)
#define GAME_APP_MEM_SIZE_4 (TEXT_VIEWER_MEM_SIZE > GAME_APP_MEM_SIZE_3 ? TEXT_VIEWER_MEM_SIZE : GAME_APP_MEM_SIZE_3)
#define GAME_APP_MEM_SIZE GAME_APP_MEM_SIZE_4
static uint8_t game_app_mem[GAME_APP_MEM_SIZE];

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
void setup() {
    serial_init();
    rng_init();
    interrupts_init();
    button_init();
    nokia_init();
    screen_init();

    // IMAGE: BSIDESCBR
    //screen_draw_img(bsidescbr_logo);
    //screen_swap_fb();

    // IMAGE: NOPIA
    //screen_draw_img(nopia_logo);
    //screen_swap_fb();

    // IMAGE: CYBERNATS
    //screen_draw_img(cybernats_logo);
    //screen_swap_fb();

    // IMAGE: CYBERNATS (EXPLODING PC)
    //screen_draw_img(cybernatspc_logo);
    //screen_swap_fb();

    // IMAGE: DEMO HOME MENU
    //screen_draw_img(home_demo);
    //screen_swap_fb();

    // IMAGE: DEMO MENU
    //screen_draw_img(menu_demo);
    //screen_swap_fb();

    // IMAGE: SILVO JUGGLE
    screen_draw_img(silvo_juggle);
    screen_swap_fb();

    // QR CODE GENERATOR
    //qr_code_draw("https://www.bsidesau.com.au/");
    //screen_swap_fb();

    // SNAKE
    //snake_init(game_app_mem);

    // TETRIS
    //tetris_init(game_app_mem);

    // TEXT VIEWER
    //viewer_init(game_app_mem);
    //viewer_c_str("Deep into that darkness peering, long I stood there, wondering, fearing, doubting\n\n - Edgar Allan Poe\n\nRead more at: https://www.brainyquote.com/quotes/edgar_allan_poe_393723");
    //viewer_draw();
    //screen_swap_fb();

    // DIALER
    //dialer_init();
    //dialer_draw();
    //screen_swap_fb();
}

void loop() {
    // NOPIA SIGNAL / POWER
    //screen_power_display(rng_random(0, 4));
    //screen_signal_display(rng_random(0, 4));
    //screen_swap_fb();
    //delay(1000);

    // Virtual interrupt handling
    interrupts_tick();
}

