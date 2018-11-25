#ifndef _H_FONT_4X6_C_H_
#define _H_FONT_4X6_C_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
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
typedef void(F46C_API *F46cDrawPixelFn)(void *, uint32_t, uint32_t, uint32_t);

void f46c_draw_char(uint32_t x, uint32_t y, char c, uint32_t color, uint32_t bg, F46cDrawPixelFn func, void *ctx);

#ifdef __cplusplus
}
#endif

#endif
