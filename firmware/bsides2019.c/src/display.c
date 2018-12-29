#include "bsides2019.h"

uint8_t display_u8g2_hw_spi_gpio_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{

    switch(msg)
    {
        // for the first two cases, we really can't get down to
        // that kind of resolution here.
        // for the 100ns delay, we give it a go anyway, but any 
        // number of individual nanosecond delays we're being asked to
        // perform will pass before we even get into this case statement
        // at 16Mhz
        case U8X8_MSG_DELAY_NANO:
                break;
        case U8X8_MSG_DELAY_100NANO:
                for(int i=0 ; i < arg_int ; i++)
                        _delay_us(0.1);
                break;
        case U8X8_MSG_DELAY_10MICRO:    // delay arg_int * 10 micro seconds
                for(int i=0 ; i < arg_int ; i++)
                        _delay_us(10);
                break;
        case U8X8_MSG_DELAY_MILLI:      // delay arg_int * 1 milli second
                for(int i=0 ; i < arg_int ; i++)
                        _delay_ms(1);
                break;
        case U8X8_MSG_GPIO_AND_DELAY_INIT:  // called once during init phase of u8x8/u8x8
                DISPLAY_CLK_DDR   |= 1 << DISPLAY_CLK_PIN;
                DISPLAY_DATA_DDR  |= 1 << DISPLAY_DATA_PIN;
                DISPLAY_CS_DDR    |= 1 << DISPLAY_CS_PIN;
                DISPLAY_DC_DDR    |= 1 << DISPLAY_DC_PIN;
                DISPLAY_RESET_DDR |= 1 << DISPLAY_RESET_PIN;
                break;              // can be used to setup pins
        case U8X8_MSG_GPIO_SPI_CLOCK:        // Clock pin: Output level in arg_int
                if(arg_int)
                        DISPLAY_CLK_PORT |=  (1 << DISPLAY_CLK_PIN);
                else
                        DISPLAY_CLK_PORT &= ~(1 << DISPLAY_CLK_PIN);
                break;
        case U8X8_MSG_GPIO_SPI_DATA:        // MOSI pin: Output level in arg_int
                if(arg_int)
                        DISPLAY_DATA_PORT |=  (1 << DISPLAY_DATA_PIN);
                else
                        DISPLAY_DATA_PORT &= ~(1 << DISPLAY_DATA_PIN);
                break;
        case U8X8_MSG_GPIO_CS:        // CS (chip select) pin: Output level in arg_int
                if(arg_int)
                        DISPLAY_CS_PORT |=  (1 << DISPLAY_CS_PIN);
                else
                        DISPLAY_CS_PORT &= ~(1 << DISPLAY_CS_PIN);
                break;
        case U8X8_MSG_GPIO_DC:        // DC (data/cmd, A0, register select) pin: Output level in arg_int
                if(arg_int)
                        DISPLAY_DC_PORT |=  (1 << DISPLAY_DC_PIN);
                else
                        DISPLAY_DC_PORT &= ~(1 << DISPLAY_DC_PIN);
                break;
        
        case U8X8_MSG_GPIO_RESET:     // Reset pin: Output level in arg_int
                if(arg_int)
                        DISPLAY_RESET_PORT |=  (1 << DISPLAY_RESET_PIN);
                else
                        DISPLAY_RESET_PORT &= ~(1<<DISPLAY_RESET_PIN);
                break;
        default:
                u8x8_SetGPIOResult(u8x8, 1);      // default return value
                break;
    }

    return 1;
}

void display_init()
{

    // switch on backlight
    DISPLAY_BL_DDR  |=  (1 << DISPLAY_BL_PIN);
    DISPLAY_BL_PORT &= ~(1 << DISPLAY_BL_PIN);
    DISPLAY_BL_PORT |=  (1 << DISPLAY_BL_PIN);

    // toggle reset on LCD
    DISPLAY_RESET_DDR  |=  (1 << DISPLAY_RESET_PIN);
    DISPLAY_RESET_PORT &= ~(1 << DISPLAY_RESET_PIN);
    DISPLAY_RESET_PORT |=  (1 << DISPLAY_RESET_PIN);

    u8g2_Setup_pcd8544_84x48_f(&u8g2, U8G2_R0, u8x8_byte_4wire_sw_spi, display_u8g2_hw_spi_gpio_delay);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    u8g2_ClearBuffer(&u8g2);
    u8g2_SetFontMode(&u8g2, 0);
    u8g2_SetFont(&u8g2, u8g2_font_profont10_mr);

}

void display_logo(u8g2_uint_t bitmap_width, u8g2_uint_t bitmap_height, const uint8_t *bitmap)
{

    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawXBMP(&u8g2, 0, 0, bitmap_width, bitmap_height, bitmap);
    u8g2_SendBuffer(&u8g2);

}

void display_logos()
{

    u8g2_SetDrawColor(&u8g2, 1);

    display_logo(bsidescbr_width, bsidescbr_height, bsidescbr_bits);
    _delay_ms(2000);

    display_logo(nopia_width, nopia_height, nopia_bits);
    _delay_ms(2000);

}
