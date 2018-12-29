#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

// pin configuration

#define DISPLAY_BL_DDR DDRB
#define DISPLAY_BL_PORT PORTB
#define DISPLAY_BL_PIN PB0

#define DISPLAY_CLK_DDR DDRB
#define DISPLAY_CLK_PORT PORTB
#define DISPLAY_CLK_PIN PB5

#define DISPLAY_DATA_DDR DDRB
#define DISPLAY_DATA_PORT PORTB
#define DISPLAY_DATA_PIN PB3

#define DISPLAY_CS_DDR DDRD
#define DISPLAY_CS_PORT PORTD
#define DISPLAY_CS_PIN PD6

#define DISPLAY_DC_DDR DDRD
#define DISPLAY_DC_PORT PORTD
#define DISPLAY_DC_PIN PD7

#define DISPLAY_RESET_DDR DDRD
#define DISPLAY_RESET_PORT PORTD
#define DISPLAY_RESET_PIN PD5

#define BUTTON_DDR DDRD
#define BUTTON_PORT PORTD
#define BUTTON_LEFT_PIN PD2
#define BUTTON_OK_PIN PD3
#define BUTTON_RIGHT_PIN PD4

#endif /* CONFIG_H_INCLUDED */
