#ifndef DISPLAY_H_INCLUDED
#define DISPLAY_H_INCLUDED

#include <u8x8.h>

extern u8x8_t u8x8;
uint8_t display_u8g2_hw_spi_gpio_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
void display_init(void);
void display_logos(void);

#endif /* DISPLAY_H_INCLUDED */
