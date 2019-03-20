#include <vintc.h>
#include <vgfxc.h>
#include <font4x6c.h>
#include <snakec.h>
#include <avr/eeprom.h>
#include <avr/pgmspace.h>

#define LOG_ERR(...)
#define LOG(...)

//-----------------------------------------------------------------------------
// Types
//-----------------------------------------------------------------------------
typedef uint8_t button_key_t;
typedef uint8_t button_state_t;
typedef void(*ButtonPressFn)(void *ctx, button_key_t key, button_state_t state);

//-----------------------------------------------------------------------------
// Hardware
//-----------------------------------------------------------------------------
#define NOKIA_5110_CLK    13
#define NOKIA_5110_DIN    11
#define NOKIA_5110_DC     7
#define NOKIA_5110_CE     6
#define NOKIA_5110_RST    5
#define NOKIA_5110_BL     8

#define BUTTON_LEFT       2
#define BUTTON_OK         3
#define BUTTON_RIGHT      4

//-----------------------------------------------------------------------------
// Interrupt timers (Virtual)
//-----------------------------------------------------------------------------
#define TMR_INTERRUPTS_MAX    8
#define INT_INVALID_HANDLE    ((size_t)VINTC_INVALID_HANDLE)
static uint8_t interrupts[VINTC_CALC_DATA_SIZE(TMR_INTERRUPTS_MAX)] = {0};
uint32_t VINTC_API interrupts_get_tick_count_api(void *ctx) {
    ctx = ctx;
    return (uint32_t)millis();
}
void interrupts_init() {
    // Virtual interrupts
    if (!vintc_init(interrupts, sizeof(interrupts)))
    {
        LOG_ERR(3,0);
    }
    if (!vintc_set_get_tick_count(interrupts, interrupts_get_tick_count_api, NULL))
    {
        LOG_ERR(3,1);
    }
}
void interrupts_tick() {
    if(!vintc_run_loop(interrupts)) {
        LOG_ERR(3,2);
    }
}
size_t interrupts_set(uint32_t period_ms, VIntCTockFn func, void *ctx) {
    VINTC_HANDLE handle = VINTC_INVALID_HANDLE;
    if (!vintc_set_interrupt(interrupts, period_ms, func, ctx, &handle))
    {
        LOG_ERR(3,3);
        return INT_INVALID_HANDLE;
    }
    return (size_t)handle;
}
void interrupts_remove(size_t handle) {
    if (!vintc_remove(interrupts, (VINTC_HANDLE)handle))
    {
        LOG_ERR(3,4);
    }
}

//-----------------------------------------------------------------------------
// Buttons
//-----------------------------------------------------------------------------
#define BUTTON_KEY_LEFT     ((button_key_t)0)
#define BUTTON_KEY_OK       ((button_key_t)1)
#define BUTTON_KEY_RIGHT    ((button_key_t)2)
#define BUTTON_KEY_COUNT    (3)
#define BUTTON_STATE_UP       ((button_state_t)0)
#define BUTTON_STATE_DOWN     ((button_state_t)1)
#define BUTTON_STATE_HOLD     ((button_state_t)2)
static ButtonPressFn button_cb = NULL;
static void *button_ctx = NULL;
static button_key_t button_state[BUTTON_KEY_COUNT];
static uint32_t button_timestamp[BUTTON_KEY_COUNT];
void button_check(int pin, button_key_t key) {
    int val = digitalRead(pin);
    if ((val == LOW) && (BUTTON_STATE_UP == button_state[key])) {
        // DOWN
        button_state[key] = BUTTON_STATE_DOWN;
        button_timestamp[key] = (uint32_t)millis();
        if (button_cb) {
            button_cb(button_ctx, key, BUTTON_STATE_DOWN);
        }
        return;
    }
    if ((val == HIGH) && (BUTTON_STATE_UP != button_state[key])) {
        // UP
        button_state[key] = BUTTON_STATE_UP;
        if (NULL != button_cb) {
            button_cb(button_ctx, key, BUTTON_STATE_UP);
        }
        return;
    }
    if (BUTTON_STATE_DOWN == button_state[key]) {
        // HOLD
        if (((uint32_t)millis() - button_timestamp[key]) > 1000) {
            button_state[key] = BUTTON_STATE_HOLD;
            if (button_cb) {
                button_cb(button_ctx, key, BUTTON_STATE_HOLD);
            }
            return;
        }
    }
}
void button_check_interrupt(void *ctx) {
    ctx = ctx;
    button_check(BUTTON_LEFT, BUTTON_KEY_LEFT);
    button_check(BUTTON_OK, BUTTON_KEY_OK);
    button_check(BUTTON_RIGHT, BUTTON_KEY_RIGHT);
}
void button_set_callback(ButtonPressFn func, void *ctx) {
    button_cb = func;
    button_ctx = ctx;
}
void button_init() {
    pinMode(BUTTON_LEFT, INPUT_PULLUP);
    pinMode(BUTTON_OK, INPUT_PULLUP);
    pinMode(BUTTON_RIGHT, INPUT_PULLUP);

    // clear state
    memset(button_state, 0, sizeof(button_state));
    memset(button_timestamp, 0, sizeof(button_timestamp));

    // check button interrupt
    (void)interrupts_set(1, button_check_interrupt, NULL);
}

//-----------------------------------------------------------------------------
// NOKIA 5110 screen
//-----------------------------------------------------------------------------
//Nokia_LCD nokia_5110(NOKIA_5110_CLK, NOKIA_5110_DIN, NOKIA_5110_DC, NOKIA_5110_CE, NOKIA_5110_RST);
#define kClk_pin NOKIA_5110_CLK
#define kDin_pin NOKIA_5110_DIN
#define kDc_pin NOKIA_5110_DC
#define kCe_pin NOKIA_5110_CE
#define kRst_pin NOKIA_5110_RST
#define NOKIA_SCREEN_WIDTH            84
#define NOKIA_SCREEN_HEIGHT           48
#define NOKIA_SCREEN_PIXELS           (NOKIA_SCREEN_WIDTH*NOKIA_SCREEN_HEIGHT)
#define NOKIA_SCREEN_BYTES            (NOKIA_SCREEN_PIXELS/8)
#define NOKIA_SCREEN_TEXT_ROWS        6
#define NOKIA_SCREEN_PIXELS_PER_ROW   (NOKIA_SCREEN_PIXELS/NOKIA_SCREEN_TEXT_ROWS)
#define NOKIA_SCREEN_BYTES_PER_ROW    (NOKIA_SCREEN_BYTES/NOKIA_SCREEN_TEXT_ROWS)
static unsigned char nokia_screen_buffer[NOKIA_SCREEN_BYTES];
static bool nokia_color_normal = true;
#define nokia_colour_invert()   nokia_color_normal=!nokia_color_normal
void nokia_draw_clear() {
    memset(nokia_screen_buffer, nokia_color_normal ? 0 : 0xff, sizeof(nokia_screen_buffer));
}
void nokia_draw_black() {
    memset(nokia_screen_buffer, nokia_color_normal ? 0xff : 0, sizeof(nokia_screen_buffer));
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
    if (nokia_color_normal == black) {
        nokia_screen_buffer[index] |= (1 << bit);
    } else {
        nokia_screen_buffer[index] &=~ (1 << bit);
    }
}
void nokia_send(const unsigned char lcd_byte, const bool is_data) {
    // tell the LCD that we are writing either to data or a command
    digitalWrite(kDc_pin, is_data);

    // send the byte
    digitalWrite(kCe_pin, LOW);
    shiftOut(kDin_pin, kClk_pin, MSBFIRST, lcd_byte);
    digitalWrite(kCe_pin, HIGH);
}
void nokia_send_command(const unsigned char command) {
    nokia_send(command, false);
}
void nokia_send_data(const unsigned char data) {
    nokia_send(data, true);
}
void nokia_swap_fb() {
    // set cursor
    nokia_send_command(0x80 | 0);  // Column
    nokia_send_command(0x40 | 0);  // Row

    // send pixels
    for (unsigned int i = 0; i < sizeof(nokia_screen_buffer); i++) {
        nokia_send_data(nokia_screen_buffer[i]);
    }
}
void nokia_init() {
    memset(nokia_screen_buffer, 0, NOKIA_SCREEN_BYTES);

    // setup connection
    pinMode(kClk_pin, OUTPUT);
    pinMode(kDin_pin, OUTPUT);
    pinMode(kDc_pin, OUTPUT);
    pinMode(kCe_pin, OUTPUT);
    pinMode(kRst_pin, OUTPUT);

    // reset the LCD to a known state
    digitalWrite(kRst_pin, LOW);
    digitalWrite(kRst_pin, HIGH);

    nokia_send_command(0x21);  // Tell LCD extended commands follow
    nokia_send_command(0xB0);  // Set LCD Vop (Contrast)
    nokia_send_command(0x04);  // Set Temp coefficent
    nokia_send_command(0x14);  // LCD bias mode 1:48 (try 0x13)
    // we must send 0x20 before modifying the display control mode
    nokia_send_command(0x20);
    nokia_send_command(0x0C);  // Set display control, normal mode.

    // set contrast
    nokia_send_command(0x21);             // Tell LCD that extended commands follow
    nokia_send_command(0x80 | 60);        // Set LCD Vop (Contrast)
    nokia_send_command(0x20);             // Set display mode

    // backlight
    pinMode(NOKIA_5110_BL, OUTPUT);
    digitalWrite(NOKIA_5110_BL, HIGH);
}

//-----------------------------------------------------------------------------
// Screen
//-----------------------------------------------------------------------------
#define SCREEN_COLOR_WHITE  (false)
#define SCREEN_COLOR_BLACK  (true)
#define SCREEN_WIDTH        NOKIA_SCREEN_WIDTH
#define SCREEN_HEIGHT       NOKIA_SCREEN_HEIGHT
#define SCREEN_FONT_WIDTH   F46C_WIDTH
#define SCREEN_FONT_HEIGHT  F46C_HEIGHT
static VGFX_CANVAS_2D_DATA screen;
void VGFX_API screen_draw_clear_api(void *ctx, vgfx_color_t color) {
    ctx = ctx;
    color = color;
    nokia_draw_clear();
}
void VGFX_API screen_draw_pixel_api(void *ctx, size_t x, size_t y, vgfx_color_t color) {
    ctx = ctx;
    nokia_draw_pixel(x, y, color > 0 ? true : false);
}
void VGFX_API screen_draw_char_api(void *ctx, size_t x, size_t y, char c, vgfx_color_t color, vgfx_color_t bg) {
    ctx = ctx;
    f46c_draw_char(x, y, c, color, bg, screen_draw_pixel_api, NULL);
}
void screen_init() {
    // Add all the drawing APIs we need
    memset(&screen, 0, sizeof(screen));
    if (!vg2d_init(&screen, sizeof(VGFX_CANVAS_2D_DATA))) {
        LOG_ERR(5,0);
    }
    if (!vg2d_set_draw_clear(&screen, screen_draw_clear_api, NULL)) {
        LOG_ERR(5,1);
    }
    if (!vg2d_set_draw_pixel(&screen, screen_draw_pixel_api, NULL)) {
        LOG_ERR(5,2);
    }
    if (!vg2d_set_draw_char(&screen, screen_draw_char_api, NULL, F46C_WIDTH, F46C_HEIGHT)) {
        LOG_ERR(5,3);
    }
}
#define screen_swap_fb()                      nokia_swap_fb()
#define screen_draw_clear()                   nokia_draw_clear()
#define screen_fill_rect(x,y,w,h,color)       (void)vg2d_fill_rect(&screen, x, y, w, h, color ? 1 : 0)
#define screen_draw_rect(x,y,w,h,color)       (void)vg2d_draw_rect(&screen, x, y, w, h, color ? 1 : 0)
#define screen_draw_line(x0,y0,x1,y1,color)   (void)vg2d_draw_line(&screen, x0, y0, x1, y1, color ? 1 : 0)
#define screen_draw_pixel(x,y,color)          (void)nokia_draw_pixel(x, y, color ? 1 : 0)
#define screen_draw_char(x,y,c,color,bg)      (void)vg2d_draw_char(&screen, x, y, c, color ? 1 : 0, bg ? 1 : 0)
#define screen_draw_string(x,y,s,color,bg)    (void)vg2d_draw_string(&screen, x, y, s, color ? 1 : 0, bg ? 1 : 0)
size_t screen_string_width(const char *s) {
    return ((size_t)(SCREEN_FONT_WIDTH * strlen(s)));
}
#define screen_string_height(s)               (SCREEN_FONT_HEIGHT)

//-----------------------------------------------------------------------------
// Snake
//-----------------------------------------------------------------------------
#define SNAKE_SQUARE_SIZE         3
#define SNAKE_GRID_OFFSET         2
#define SNAKE_GRID_WIDTH          ((SCREEN_WIDTH - (SNAKE_GRID_OFFSET * 2)) / SNAKE_SQUARE_SIZE)
#define SNAKE_GRID_HEIGHT         ((SCREEN_HEIGHT - (SNAKE_GRID_OFFSET * 2)) / SNAKE_SQUARE_SIZE)
#define SNAKE_GAME_MEM_SIZE       (SNKC_CALC_DATA_SIZE(SNAKE_GRID_WIDTH,SNAKE_GRID_HEIGHT))
static uint8_t *snkc_mem =        NULL;
static size_t snkc_mem_size =     0;
static size_t snkc_int_handle =   INT_INVALID_HANDLE;
static uint8_t snake_direction =  0xff;
void SNKC_API snake_draw_clear_api(void *ctx) {
    char score[10];
    ctx = ctx;
    screen_draw_clear();
    screen_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_BLACK);
    sprintf(score, "%d", snkc_get_score(snkc_mem));
    screen_draw_string(SCREEN_WIDTH - screen_string_width(score) - 1, SCREEN_HEIGHT - screen_string_height(score) - 1, score, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
}
void SNKC_API snake_draw_snake_api(void *ctx, int16_t x, int16_t y) {
    ctx = ctx;
    x = SNAKE_GRID_OFFSET + (x * SNAKE_SQUARE_SIZE);
    y = SNAKE_GRID_OFFSET + (y * SNAKE_SQUARE_SIZE);
    screen_fill_rect(x, y, SNAKE_SQUARE_SIZE, SNAKE_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
void SNKC_API snake_draw_apple_api(void *ctx, int16_t x, int16_t y) {
    ctx = ctx;
    x = SNAKE_GRID_OFFSET + (x * SNAKE_SQUARE_SIZE);
    y = SNAKE_GRID_OFFSET + (y * SNAKE_SQUARE_SIZE);
    screen_draw_rect(x, y, SNAKE_SQUARE_SIZE, SNAKE_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
int16_t SNKC_API snake_random_api(void *ctx, int16_t min, int16_t max) {
    ctx = ctx;
    return random(min, max + 1);
}
void snake_start();
void snake_stop();
void snake_game_return(void *ctx) {
    ctx = ctx;
    snake_start();
}
void SNKC_API snake_game_over_api(void *ctx, uint16_t score) {
    ctx = ctx;
    snake_stop();

    // do something on game over

    snake_start();
}
void VINTC_API snake_tick_api(void *ctx) {
    ctx = ctx;
    if(!snkc_tick(snkc_mem)) {
        LOG_ERR(7,0);
    }
    if (INT_INVALID_HANDLE != snkc_int_handle) {
        screen_swap_fb();
    }
}
void snake_stop() {
    interrupts_remove(snkc_int_handle);
    snkc_int_handle = INT_INVALID_HANDLE;
    memset(snkc_mem, 0, snkc_mem_size);
}
void snake_button_press(void *ctx, button_key_t key, button_state_t state) {
    ctx = ctx;
    if ((BUTTON_KEY_LEFT == key) && (BUTTON_STATE_HOLD == state)) {
        // return to menu
        snake_stop();
        return;
    }
    if (BUTTON_STATE_DOWN != state) {
        // only response to button down events
        return;
    }
    switch(key) {
        case BUTTON_KEY_LEFT:
            // left or up
            if (0xff == snake_direction) {
                snake_direction = 0;  // start going LEFT
            } else if ((1 == snake_direction) || (3 == snake_direction)) {
                // up or down - go left
                snake_direction = 0;  // LEFT
            } else {
                // left or right - go up
                snake_direction = 1;  // UP
            }
            break;
        case BUTTON_KEY_RIGHT:
            // right or down
            if (0xff == snake_direction) {
                snake_direction = 2;  // start going RIGHT
            } else if ((0 == snake_direction) || (2 == snake_direction)) {
                // left or right - go down
                snake_direction = 3;  // DOWN
            } else {
                // up or down - go right
                snake_direction = 2;  // RIGHT
            }
            break;
        case BUTTON_KEY_OK:
            // rotate clockwise
            snake_direction++;
            break;
        default:
            return;
            break;
    }
    if (snake_direction > 3) {
        snake_direction = 0;
    }
    switch(snake_direction) {
        case 0:
            (void)snkc_key_left(snkc_mem);
            break;
        case 1:
            (void)snkc_key_up(snkc_mem);
            break;
        case 2:
            (void)snkc_key_right(snkc_mem);
            break;
        case 3:
            (void)snkc_key_down(snkc_mem);
            break;
        default:
            break;
    }
}
void snake_init(void *mem, size_t mem_size) {
    snkc_mem = (uint8_t*)mem;
    snkc_mem_size = mem_size;
}
void snake_start() {
    snake_direction = 0xff;

    // Snake game
    if (!snkc_init(snkc_mem, snkc_mem_size))
    {
        LOG_ERR(7,5);
    }
    if (!snkc_set_grid(snkc_mem, SNAKE_GRID_WIDTH, SNAKE_GRID_HEIGHT))
    {
        LOG_ERR(7,6);
    }

    // Snake will draw on the screen
    if (!snkc_set_draw_clear(snkc_mem, snake_draw_clear_api, &screen)) {
        LOG_ERR(7,7);
    }
    if (!snkc_set_draw_snake(snkc_mem, snake_draw_snake_api, &screen)) {
        LOG_ERR(7,8);
    }
    if (!snkc_set_draw_apple(snkc_mem, snake_draw_apple_api, &screen)) {
        LOG_ERR(7,9);
    }
    if (!snkc_set_random(snkc_mem, snake_random_api, NULL)) {
        LOG_ERR(7,10);
    }
    if (!snkc_set_game_over(snkc_mem, snake_game_over_api, NULL)) {
        LOG_ERR(7,11);
    }

    // Snake frame rate
    snkc_int_handle = interrupts_set(1000/15, snake_tick_api, snkc_mem);

    // Snake buttons
    button_set_callback(snake_button_press, NULL);

    // Game reset
    if (!snkc_reset(snkc_mem))
    {
        LOG_ERR(7,12);
    }
}

//-----------------------------------------------------------------------------
// Game and application dynamic memory
//-----------------------------------------------------------------------------
#define APP_MEM_SIZE    SNAKE_GAME_MEM_SIZE
static uint8_t app_mem[APP_MEM_SIZE];
const static size_t app_mem_size = APP_MEM_SIZE;

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
void setup() {
    interrupts_init();
    button_init();
    nokia_init();
    screen_init();
    snake_init(app_mem, app_mem_size);

    randomSeed(analogRead(0));

    snake_start();
}

void loop() {
    interrupts_tick();
}
