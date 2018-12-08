#ifndef _H_FONT_4X6_C_H_
#define _H_FONT_4X6_C_H_

#include <stdint.h>
#include <limits.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// need ssize_t
#if SIZE_MAX == UINT_MAX
typedef int ssize_t;        /* common 32 bit case */
typedef int off_t;
#elif SIZE_MAX == ULONG_MAX
typedef long ssize_t;       /* linux 64 bits */
typedef long off_t;
#elif SIZE_MAX == ULLONG_MAX
typedef long long ssize_t;  /* windows 64 bits */
typedef long long off_t;
#elif SIZE_MAX == USHRT_MAX
typedef short ssize_t;      /* think AVR with 32k of flash */
typedef short off_t;
#else
#error platform has exotic SIZE_MAX
#endif

// Credits to:
// https://hackaday.io/project/6309-vga-graphics-over-spi-and-serial-vgatonic/log/20759-a-tiny-4x6-pixel-font-that-will-fit-on-almost-any-microcontroller-license-mit

// non-arduino
#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*((uint8_t*)addr))
#endif

#define F46C_WIDTH      4
#define F46C_HEIGHT     6

#define F46C_API
typedef void(F46C_API *F46cDrawPixelFn)(void *, size_t, size_t, uint32_t);

void f46c_draw_char(size_t x, size_t y, char c, uint32_t color, uint32_t bg, F46cDrawPixelFn func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
