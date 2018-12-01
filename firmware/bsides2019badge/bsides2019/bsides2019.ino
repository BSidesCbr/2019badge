#include <Nokia_LCD.h>
#include <vintc.h>
#include <vgfxc.h>
#include <font4x6c.h>
#include <vfsc.h>
#include <snakec.h>
#include <tetrisc.h>
#include <viewerc.h>
#include <tinymenuc.h>
#include <qrcodegen.h>

//-----------------------------------------------------------------------------
// Binary data (generated) for Virtual File System (VFS)
//-----------------------------------------------------------------------------
#include "vfs.h"

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
// Schedule
//-----------------------------------------------------------------------------
typedef struct _ScheduleItem {
   const char *start_time;
   const char *end_time;
   const char *title;
   const char *presenters;
   const char *description;
} ScheduleItem;

/*
const ScheduleItem schedule_day_1[] = {
  { F("9:00am"), F("9:10am"), F("Conference Opening"), F("Silvio & Kylie"), F("(desc1)") },
  { F("9:10am"), F("10:00am"), F("Keynote - My Cyber is Trickling Down and the other Diverse Problems of the Aging Hacker"), F("Metlstorm"), F("(desc2)") },
};*/
/*
const unsigned char schedule_data[] = {
   '\x00', '\x00',
};
*/

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
// Virtual File System (VFS)
//-----------------------------------------------------------------------------
static uint8_t vfs[VFSC_VF_CALC_DATA_SIZE(4)];  // max 4 open handles
uint8_t VFSC_API vfs_read_byte(void *ctx, void *addr) {
  return pgm_read_byte(addr);
}
void vfs_init() {
  if (0 != vfsc_init(vfs, sizeof(vfs), vfs_data, vfs_size, vfs_read_byte, NULL)) {
    LOG("failed to init vfs");
  }
}
#define open_hash(hash,...)     vfsc_open_hash(vfs,hash)
#define open(pathname,...)      vfsc_open(vfs,pathname)
#define read(fd,buf,count)      vfsc_read(vfs,fd,buf,count)
#define close(fd)               vfsc_close(vfs,fd)

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
#define TMR_INTERRUPTS_MAX 8
#define INT_INVALID_HANDLE ((uint32_t)VINTC_INVALID_HANDLE)
static uint8_t interrupts[VINTC_CALC_DATA_SIZE(TMR_INTERRUPTS_MAX)] = {0};
uint32_t VINTC_API interrupts_get_tick_count_api(void *ctx) {
    ctx;
    return (uint32_t)millis();
}
void interrupts_init() {
    // Virtual interrupts
    if (!vintc_init(interrupts, sizeof(interrupts)))
    {
        LOG("init interrupts failed");
    }
    if (!vintc_set_get_tick_count(interrupts, interrupts_get_tick_count_api, NULL))
    {
        LOG("interrupts set get tick count failed");
    }
}
void interrupts_tick() {
    if(!vintc_run_loop(interrupts)) {
        LOG("interrupts run loop failed");
    }
}
uint32_t interrupts_set(uint32_t period_ms, VIntCTockFn func, void *ctx) {
    VINTC_HANDLE handle = VINTC_INVALID_HANDLE;
    if (!vintc_set_interrupt(interrupts, period_ms, func, ctx, &handle))
    {
        LOG("set interrupt failed");
        return INT_INVALID_HANDLE;
    }
    return (uint32_t)handle;
}
void interrupts_remove(uint32_t handle) {
    if (!vintc_remove(interrupts, (VINTC_HANDLE)handle))
    {
        LOG("reomve interrupt failed");
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
void button_read_digital_inputs(void *ctx) {
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
void button_set_callback(void *func, void *ctx) {
    button_cb = func;
    button_ctx = ctx;
}
void button_init() {
    pinMode(BUTTON_LEFT, INPUT);
    pinMode(BUTTON_OK, INPUT);
    pinMode(BUTTON_RIGHT, INPUT);

    // check button interrupt
    (void)interrupts_set(1, button_read_digital_inputs, NULL);
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
void nokia_draw_img(void *data, uint32_t x = 0, uint32_t y = 0, uint32_t width = NOKIA_SCREEN_WIDTH, uint32_t height = NOKIA_SCREEN_HEIGHT) {
     for (uint32_t dy = 0; dy < height; dy++) {
        for (uint32_t dx = 0; dx < width; dx++) {
          uint32_t index = (dy * width) + dx;
          uint32_t index_byte = index / 8;
          uint32_t index_bit = 7 - (index % 8);
          uint8_t value = pgm_read_byte(&(((uint8_t*)data)[index_byte]));
          nokia_draw_pixel(x + dx, y + dy, ((value >> index_bit) & 0x1) != 0 ? true : false);
      }
    }
}
void nokia_draw_raw_hash(uint32_t hash, uint32_t x = 0, uint32_t y = 0, uint32_t width = NOKIA_SCREEN_WIDTH, uint32_t height = NOKIA_SCREEN_HEIGHT) {
    int fd = open_hash(hash);
    if (fd < 0) {
      LOG("Failed to open bitmap");
    }
    uint8_t value = 0;
    for (uint32_t dy = 0; dy < height; dy++) {
        for (uint32_t dx = 0; dx < width; dx++) {
            uint32_t index = (dy * width) + dx;
            uint32_t index_bit = 7 - (index % 8);
            if (0 == (index % 8)) {
                if (1 != read(fd, &value, sizeof(uint8_t))) {
                    LOG("Failed read byte from bitmap");
                    close(fd);
                    return;
                }
            }
            nokia_draw_pixel(x + dx, y + dy, ((value >> index_bit) & 0x1) != 0 ? true : false);
        }
    }
    close(fd);
}
#define nokia_draw_raw(path) nokia_draw_raw_hash(VFSC_HASH(path))
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
#define screen_draw_raw(path) nokia_draw_raw(path)
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
// Dashboard Power and Signal
//-----------------------------------------------------------------------------
static uint8_t dash_power = 3;
void dash_power_draw() {
  screen_fill_rect(SCREEN_WIDTH - 6, 0, 6, 38, SCREEN_COLOR_WHITE);
  if (dash_power >= 4) {
    screen_fill_rect(SCREEN_WIDTH - 4, 0, 4, 7, SCREEN_COLOR_BLACK);
  }
  if (dash_power >= 3) {
    screen_fill_rect(SCREEN_WIDTH - 3, 8, 3, 7, SCREEN_COLOR_BLACK);
  }
  if (dash_power >= 2) {
    screen_fill_rect(SCREEN_WIDTH - 2, 16, 2, 7, SCREEN_COLOR_BLACK);
  }
  if (dash_power >= 1) {
    screen_fill_rect(SCREEN_WIDTH - 2, 24, 2, 6, SCREEN_COLOR_BLACK);
  }
  screen_fill_rect(SCREEN_WIDTH - 3, 31, 2, 2, SCREEN_COLOR_BLACK);
  screen_draw_rect(SCREEN_WIDTH - 4, 32, 4, 5, SCREEN_COLOR_BLACK);
}
static uint8_t dash_signal = 4;
void dash_signal_draw() {
  screen_fill_rect(0, 0, 6, 38, SCREEN_COLOR_WHITE);
  if (dash_signal >= 4) {
    screen_fill_rect(0, 0, 4, 7, SCREEN_COLOR_BLACK);
  }
  if (dash_signal >= 3) {
    screen_fill_rect(0, 8, 3, 7, SCREEN_COLOR_BLACK);
  }
  if (dash_signal >= 2) {
    screen_fill_rect(0, 16, 2, 7, SCREEN_COLOR_BLACK);
  }
  if (dash_signal >= 1) {
    screen_fill_rect(0, 24, 2, 6, SCREEN_COLOR_BLACK);
  }
  screen_draw_line(0, 31, 4, 31, SCREEN_COLOR_BLACK);
  screen_draw_line(2, 31, 2, 31 + 5, SCREEN_COLOR_BLACK);
  screen_draw_line(0, 32, 1, 32 + 1, SCREEN_COLOR_BLACK);
  screen_draw_pixel(3, 33, SCREEN_COLOR_BLACK);
  screen_draw_pixel(4, 32, SCREEN_COLOR_BLACK);
}
void VINTC_API dash_update(void *ctx) {
    dash_power = rng_random(0, 4);
    dash_signal = rng_random(0, 4);
}
void dash_init() {
    (void)interrupts_set(1000, dash_update, NULL);
}

//-----------------------------------------------------------------------------
// RETURN TO MENU
//-----------------------------------------------------------------------------
void boot_unlocked_draw(void *mem, uint32_t mem_size) {
    // Your device is unlocked and can't be trusted
    memcpy_P(mem, F("Your device is"), sizeof("Your device is"));
    screen_draw_string(0, 0, mem, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
    memcpy_P(mem, F("unlocked and"), sizeof("unlocked and"));
    screen_draw_string(0, 8, mem, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
    memcpy_P(mem, F("can't be trusted"), sizeof("can't be trusted"));
    screen_draw_string(0, 16, mem, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
}


//-----------------------------------------------------------------------------
// RETURN TO MENU
//-----------------------------------------------------------------------------
void link_menu();
void game_menu();
void main_menu();

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
}
void dialer_start() {
    dialer_x = 0;
    dialer_y = 0;
    memset(dialer_number, 0, sizeof(dialer_number));
    button_set_callback(dialer_button_press, NULL);
}
void dialer_stop() {

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
static uint32_t snkc_mem_size = 0;
static uint32_t snkc_int_handle = INT_INVALID_HANDLE;
static uint8_t snake_direction = 0xff;
void SNKC_API snake_draw_clear_api(void *ctx) {
    screen_draw_clear();
}
void SNKC_API snake_draw_snake_api(void *ctx, int16_t x, int16_t y) {
    x = SNAKE_GRID_OFFSET + (x * SNAKE_SQUARE_SIZE);
    y = SNAKE_GRID_OFFSET + (y * SNAKE_SQUARE_SIZE);
    screen_fill_rect(x, y, SNAKE_SQUARE_SIZE, SNAKE_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
void SNKC_API snake_draw_apple_api(void *ctx, int16_t x, int16_t y) {
    x = SNAKE_GRID_OFFSET + (x * SNAKE_SQUARE_SIZE);
    y = SNAKE_GRID_OFFSET + (y * SNAKE_SQUARE_SIZE);
    screen_draw_rect(x, y, SNAKE_SQUARE_SIZE, SNAKE_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
int16_t SNKC_API snake_random_api(void *ctx, int16_t min, int16_t max) {
    return (int16_t)rng_random((uint32_t)min, (uint32_t)max);
}
void snake_draw_end() {
    screen_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_BLACK);
    screen_swap_fb();
}
void VINTC_API snake_tick_api(void *ctx) {
    if(!snkc_tick(snkc_mem)) {
        LOG("snake tick failed");
    }
    snake_draw_end();
}
void snake_stop() {
    interrupts_remove(snkc_int_handle);
    snkc_int_handle = INT_INVALID_HANDLE;
    memset(snkc_mem, 0, snkc_mem_size);
}
void snake_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
  int32_t new_direction = (int32_t)snake_direction;
  if ((BUTTON_UP == down) && ((BUTTON_KEY_LEFT == key) || (BUTTON_KEY_RIGHT == key))) {
    if (duration > 1000) {
        // return to menu
        snake_stop();
        game_menu();
        return;
    }
  }
  if (BUTTON_UP == down) {
    return;
  }
  switch(key) {
    case BUTTON_KEY_LEFT:
      if (0xff == snake_direction) {
        new_direction = 0;  // start going LEFT
      } else if ((1 == snake_direction) || (3 == snake_direction)) {
        // up or down - go left
        new_direction = 0;  // LEFT
      } else {
        // left or right - go up
        new_direction = 1;  // UP
      }
      break;
    case BUTTON_KEY_RIGHT:
      if (0xff == snake_direction) {
        new_direction = 2;  // start going RIGHT
      } else if ((0 == snake_direction) || (2 == snake_direction)) {
        // left or right - go down
        new_direction = 3;  // DOWN
      } else {
        // up or down - go right
        new_direction = 2;  // RIGHT
      }
      break;
    case BUTTON_KEY_OK:
      // rotate clockwise
      new_direction++;
      if (new_direction > 3) {
          new_direction = 0;
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
void snake_init(void *mem, uint32_t mem_size) {
    snkc_mem = (uint8_t*)mem;
    snkc_mem_size = mem_size;
}
void snake_start() {
    snake_direction = 0xff;

    // Snake game
    if (!snkc_init(snkc_mem, snkc_mem_size))
    {
        LOG("snake init failed");
    }
    if (!snkc_set_grid(snkc_mem, SNAKE_GRID_WIDTH, SNAKE_GRID_HEIGHT))
    {
        LOG("snake set grid failed");
    }

     // Snake will draw on the screen
    if (!snkc_set_draw_clear(snkc_mem, snake_draw_clear_api, &screen)) {
        LOG("snake set draw empty failed");
    }
    if (!snkc_set_draw_snake(snkc_mem, snake_draw_snake_api, &screen)) {
        LOG("snake set draw snake failed");
    }
    if (!snkc_set_draw_apple(snkc_mem, snake_draw_apple_api, &screen)) {
        LOG("snake set draw apple failed");
    }
    if (!snkc_set_random(snkc_mem, snake_random_api, NULL)) {
        LOG("snake set random failed");
    }

    // Snake frame rate
    snkc_int_handle = interrupts_set(1000/15, snake_tick_api, snkc_mem);

    // Snake buttons
    button_set_callback(snake_button_press, NULL);

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
static uint32_t ttrs_mem_size = 0;
static uint32_t ttrs_int_handle = INT_INVALID_HANDLE;
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
void tetris_stop() {
    interrupts_remove(ttrs_int_handle);
    ttrs_int_handle = INT_INVALID_HANDLE;
    memset(ttrs_mem, 0, ttrs_mem_size);
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
    if (duration > 500) {
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
  if ((BUTTON_UP == down) && ((BUTTON_KEY_LEFT == key) || (BUTTON_KEY_RIGHT == key))) {
    if (duration > 1000) {
        // return to menu
        tetris_stop();
        game_menu();
        return;
    }
  }
}
void tetris_init(void *mem, uint32_t mem_size) {
    ttrs_mem = (uint8_t*)mem;
    ttrs_mem_size = mem_size;
}
void tetris_start() {
    // Tetris game
    if (!ttrs_init(ttrs_mem, ttrs_mem_size))
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
    button_set_callback(tetris_button_press, NULL);

    // Tetris frame rate
    ttrs_int_handle = interrupts_set(700, tetris_tick, ttrs_mem);

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
void viewer_init(void *mem, uint32_t mem_size) {
    vwrc_mem = (uint8_t*)mem;

    // reset
    vwrc_data_size = 0;
    vwrc_c_str = NULL;

    // init
    if (!vwrc_init(vwrc_mem, mem_size)) {
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
    button_set_callback(viewer_button_press, NULL);
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
#define QR_MEM_QR_CODE_SIZE     (qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_MAX_VERSION))
#define QR_MEM_TEMP_SIZE        (qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_MAX_VERSION))
#define QR_MEM_SIZE             (QR_MEM_QR_CODE_SIZE) // + QR_MEM_TEMP_SIZE)
static uint8_t *qrcode_mem = NULL;
static uint8_t *qrcode_temp_mem = NULL;
void qrcode_init(void *mem, uint32_t mem_size) {
  if (mem_size < QR_MEM_SIZE) {
    return;
  }
  qrcode_mem = (uint8_t*)mem;
  qrcode_temp_mem = nokia_screen_buffer; // dirty hack to save memory
}
void qrcode_draw(void *text, void *label) {
  LOG("Begin");
  enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;  // Error correction level
  bool ok = qrcodegen_encodeText(text, qrcode_temp_mem, qrcode_mem, errCorLvl,
    qrcodegen_VERSION_MIN, qrcodegen_MAX_VERSION, qrcodegen_Mask_AUTO, true);
  if (ok) {
    LOG("OK");
  } else {
    LOG("QRERR");
    return;
  }
  int size = qrcodegen_getSize(qrcode_mem);
  int x_offset = (SCREEN_WIDTH / 2) - (size / 2);
  int y_offset = (SCREEN_HEIGHT / 2) - (size / 2);
  screen_draw_clear();
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      nokia_draw_pixel(x_offset + x, y_offset + y, qrcodegen_getModule(qrcode_mem, x, y) ? 1 : 0);
    }
  }
  // another dirty hack to save memory, use the qrcode mem
  memcpy_P(qrcode_mem, label, 22);
  qrcode_mem[21] = '\0';
  screen_draw_string(0, SCREEN_HEIGHT - 6, qrcode_mem, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
  LOG("Done");
}
typedef void (*QRCodeFn)(void);
void qrcode_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
    QRCodeFn backfn = (QRCodeFn)ctx;
    if (BUTTON_KEY_LEFT == key) {
        backfn();
    }
}
void qrcode_display(void *text, void *label, void* backfn) {
    qrcode_draw(text, label);
    screen_swap_fb();
    button_set_callback(qrcode_button_press, (void*)backfn);
}


//-----------------------------------------------------------------------------
// Tiny Menu
//-----------------------------------------------------------------------------
#define MENU_VIEW_WIDTH     56
#define MENU_OFFSET_Y       9
#define MENU_VIEW_HEIGHT    (SCREEN_HEIGHT - MENU_OFFSET_Y)
#define MENU_OFFSET_X       ((SCREEN_WIDTH - MENU_VIEW_WIDTH) / 2)
#define MENU_STR_OFFSET_X   2
#define MENU_STR_OFFSET_Y   2
#define MENU_MEM_SIZE       (TMNU_CALC_DATA_SIZE(84))
static uint8_t *tmnu_mem = NULL;
static uint32_t tmnu_mem_size = 0;
static uint32_t tmnu_int_handle = INT_INVALID_HANDLE;
static char menu_title[12];
void menu_set_title(void *title) {
  memcpy_P(menu_title, title, sizeof(menu_title));
  menu_title[sizeof(menu_title)-1] = '\0';
}
void VWRC_API menu_calc_string_view_api(void *ctx, const char *str, uint32_t *width, uint32_t *height) {
    if (width) {
        *width = (uint32_t)(4 * strlen(str)); // 4x6 font used
        *width += (MENU_STR_OFFSET_X * 2); // padding on sides
    }
    if (height) {
        *height = 6; // 4x6 font used
        *height += ((MENU_STR_OFFSET_Y * 2) - 1); // padding top/bottom
    }
}
void VWRC_API menu_draw_string_api(void *ctx, uint32_t x, uint32_t y, const char *str, TMNU_BOOL selected) {
    uint32_t color = SCREEN_COLOR_BLACK;
    uint32_t bg = SCREEN_COLOR_WHITE;
    if (TMNU_TRUE == selected) {
        color = SCREEN_COLOR_WHITE;
        bg = SCREEN_COLOR_BLACK;
        screen_fill_rect(MENU_OFFSET_X + x, MENU_OFFSET_Y + y, MENU_VIEW_WIDTH - 2, 6 + ((MENU_STR_OFFSET_Y * 2) - 1), bg);
    }
    screen_draw_string(MENU_OFFSET_X + x + MENU_STR_OFFSET_X, MENU_OFFSET_Y + y + MENU_STR_OFFSET_Y, str, color, bg);
}
void menu_draw() {
    uint32_t item = 0;
    uint32_t item_count = 0;
    uint32_t items_per_view = 0;
    uint32_t y = 0;
    uint32_t h = 0;
    uint32_t line_offset_x = 0;
    uint32_t line_width = 0;
    screen_draw_clear();
    screen_draw_line(0, 5, SCREEN_WIDTH-1, 5, SCREEN_COLOR_BLACK);
    menu_calc_string_view_api(NULL, menu_title, &line_width, &line_offset_x);
    line_offset_x = (SCREEN_WIDTH - line_width) / 2;
    screen_fill_rect(line_offset_x, 0, line_width, 9, SCREEN_COLOR_WHITE);
    screen_draw_string(line_offset_x + MENU_STR_OFFSET_X, MENU_STR_OFFSET_Y, menu_title, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
    dash_power_draw();
    dash_signal_draw();
    if (!tmnu_draw_view(tmnu_mem)) {
        LOG("failed to draw menu");
    }
    if (!tmnu_get_item(tmnu_mem, &item)) {
        LOG("failed to get item");
    }
    if (!tmnu_get_item_count(tmnu_mem, &item_count)) {
        LOG("failed to get item count");
    }
    if (!tmnu_get_items_per_view(tmnu_mem, &items_per_view)) {
        LOG("failed to get items per view");
    }
    if (items_per_view < item_count) {
      // progress bars required
      y = (MENU_VIEW_HEIGHT * item) / item_count;
      h = (MENU_VIEW_HEIGHT * 1) / item_count;
      screen_fill_rect(MENU_OFFSET_X + MENU_VIEW_WIDTH - 2, MENU_OFFSET_Y + y, 2, h, SCREEN_COLOR_BLACK);
    }
    screen_swap_fb();
}
void menu_button_press(void *ctx, uint32_t key, bool down, uint32_t duration) {
  if (BUTTON_DOWN == down) {
    switch(key) {
      case BUTTON_KEY_LEFT:
        if (!tmnu_key_up(tmnu_mem)) {
            LOG("menu scroll up failed");
        }
        menu_draw();
        break;
      case BUTTON_KEY_RIGHT:
        if (!tmnu_key_down(tmnu_mem)) {
            LOG("menu scroll down failed");
        }
        menu_draw();
        break;
      case BUTTON_KEY_OK:
        if (!tmnu_key_enter(tmnu_mem)) {
            LOG("menu select item failed");
        }
        break;
      default:
        break;
    }
  }
}
void VINTC_API menu_update(void *ctx) {
    menu_draw();
}
void menu_init(void *mem, uint32_t mem_size) {
    tmnu_mem = (uint8_t*)mem;
    tmnu_mem_size = mem_size;
}
void menu_start() {
    // init
    if (!tmnu_init(tmnu_mem, tmnu_mem_size)) {
        LOG("failed to init menu");
    }

    // view size (2 pixels for scroll bar)
    if (!tmnu_set_view(tmnu_mem, MENU_VIEW_WIDTH - 2, MENU_VIEW_HEIGHT)) {
        LOG("failed to set view in text viewer");
    }

    // how to calculate the pixel size of a string
    if (!tmnu_set_calc_string_view(tmnu_mem, menu_calc_string_view_api, NULL)) {
        LOG("failed to set calc string view in text viewer");
    }

    // how to draw the text in the view
    if (!tmnu_set_draw_string(tmnu_mem, menu_draw_string_api, NULL)) {
        LOG("failed to set draw string in text viewer");
    }

    // interrupt to update the display (for signal/power changes)
    tmnu_int_handle = interrupts_set(1000, menu_update, NULL);

    // Viewer buttons
    button_set_callback(menu_button_press, NULL);
}
void menu_stop() {
    memset(tmnu_mem, 0, tmnu_mem_size);
    interrupts_remove(tmnu_int_handle);
    tmnu_int_handle = INT_INVALID_HANDLE;
}
void menu_set(uint32_t item_count, TmnuMenuItemStringFn items, TmnuMenuItemOnSelectFn action, void *ctx) {
    if (!tmnu_set_menu_item_string(tmnu_mem, item_count, items, ctx)) {
        LOG("failed to set main menu items");
    }
    if (!tmnu_set_on_select(tmnu_mem, action, ctx)) {
        LOG("failed to set main menu actions");
    }
}
#define MENU_ITEM(name,buffer,size) (memcpy_P(buffer,F(name),(sizeof(name)-1) > size ? size : (sizeof(name)-1)))

//-----------------------------------------------------------------------------
// LINK MENU
//-----------------------------------------------------------------------------
#define LINK_MENU_COUNT 2
void link_menu_items(void *ctx, uint32_t item, char *buffer, uint32_t buffer_size) {
    switch(item) {
        case 0:
            MENU_ITEM("..", buffer, buffer_size);
            break;
        case 1:
            MENU_ITEM("bsides", buffer, buffer_size);
            break;
    }
}
void link_menu_return(void) {
    link_menu();
}
#define LINK_ITEM(title,url) qrcode_display(url, F(title), link_menu_return)
void link_menu_action(void *ctx, uint32_t item) {
  switch(item) {
        case 0:
            menu_stop();
            main_menu();
            break;
        case 1:
            menu_stop();
            LINK_ITEM("bsides.com.au", "https://www.bsidesau.com.au/");
            break;
    }
}
void link_menu() {
    menu_start();
    menu_set_title(F("Links"));
    menu_set(LINK_MENU_COUNT, link_menu_items, link_menu_action, NULL);
}

//-----------------------------------------------------------------------------
// GAME MENU
//-----------------------------------------------------------------------------
#define GAME_MENU_COUNT 3
void game_menu_items(void *ctx, uint32_t item, char *buffer, uint32_t buffer_size) {
    switch(item) {
        case 0:
            MENU_ITEM("..", buffer, buffer_size);
            break;
        case 1:
            MENU_ITEM("Snake", buffer, buffer_size);
            break;
        case 2:
            MENU_ITEM("Tetris", buffer, buffer_size);
            break;
    }
}
void game_menu_action(void *ctx, uint32_t item) {
  switch(item) {
        case 0:
            menu_stop();
            main_menu();
            break;
        case 1:
            menu_stop();
            snake_start();
            break;
        case 2:
            menu_stop();
            tetris_start();
            break;
    }
}
void game_menu() {
    menu_start();
    menu_set_title(F("Games"));
    menu_set(GAME_MENU_COUNT, game_menu_items, game_menu_action, NULL);
}

//-----------------------------------------------------------------------------
// MAIN MENU
//-----------------------------------------------------------------------------
#define MAIN_MENU_COUNT 5
void main_menu_items(void *ctx, uint32_t item, char *buffer, uint32_t buffer_size) {
    switch(item) {
        case 0:
            MENU_ITEM("..", buffer, buffer_size);
            break;
        case 1:
            MENU_ITEM("Schedule", buffer, buffer_size);
            break;
        case 2:
            MENU_ITEM("Links", buffer, buffer_size);
            break;
        case 3:
            MENU_ITEM("Games", buffer, buffer_size);
            break;
        case 4:
            MENU_ITEM("Photos", buffer, buffer_size);
            break;
    }
}
void main_menu_action(void *ctx, uint32_t item) {
  switch(item) {
        case 0:
            break;
        case 1:
            break;
        case 2:
            menu_stop();
            link_menu();
            break;
        case 3:
            menu_stop();
            game_menu();
            break;
        case 4:
            break;
    }
}
void main_menu() {
    menu_start();
    menu_set_title(F("NOPIA 1337"));
    menu_set(MAIN_MENU_COUNT, main_menu_items, main_menu_action, NULL);
}

//-----------------------------------------------------------------------------
// Game and application dynamic memory
//-----------------------------------------------------------------------------
#define APP_MEM_SIZE_0 0
#define APP_MEM_SIZE_1 (SNAKE_GAME_MEM_SIZE > APP_MEM_SIZE_0 ? SNAKE_GAME_MEM_SIZE : APP_MEM_SIZE_0)
#define APP_MEM_SIZE_2 (TETRIS_GAME_MEM_SIZE > APP_MEM_SIZE_1 ? TETRIS_GAME_MEM_SIZE : APP_MEM_SIZE_1)
#define APP_MEM_SIZE_3 (MENU_MEM_SIZE > APP_MEM_SIZE_2 ? MENU_MEM_SIZE : APP_MEM_SIZE_2)
#define APP_MEM_SIZE_4 (TEXT_VIEWER_MEM_SIZE > APP_MEM_SIZE_3 ? TEXT_VIEWER_MEM_SIZE : APP_MEM_SIZE_3)
#define APP_MEM_SIZE_5 (QR_MEM_SIZE > APP_MEM_SIZE_4 ? QR_MEM_SIZE : APP_MEM_SIZE_4)
#define APP_MEM_SIZE    APP_MEM_SIZE_5
static uint8_t app_mem[APP_MEM_SIZE];
const static uint32_t app_mem_size = APP_MEM_SIZE;

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
void setup() {
    serial_init();
    vfs_init();
    rng_init();
    interrupts_init();
    button_init();
    nokia_init();
    screen_init();
    dash_init();
    dialer_init();
    snake_init(app_mem, app_mem_size);
    tetris_init(app_mem, app_mem_size);
    qrcode_init(app_mem, app_mem_size);
    menu_init(app_mem, app_mem_size);

    // IMAGE: BSIDESCBR
    screen_draw_raw("/img/bsidescbr.raw");
    screen_swap_fb();
    delay(2000);

    // IMAGE: NOPIA
    screen_draw_raw("/img/nopia.raw");
    screen_swap_fb();
    delay(2000);

    // Boot unlocked
    boot_unlocked_draw(app_mem, app_mem_size);
    screen_swap_fb();
    delay(2000);

    // IMAGE: CYBERNATS
    screen_draw_raw("/img/cybernats.raw");
    screen_swap_fb();
    delay(1000);

    // QR CODE GENERATOR
    qrcode_draw("https://www.bsidesau.com.au/", F("SCAN TO UPLOAD SCORE"));
    screen_swap_fb();
    delay(1000);

    // SNAKE
    //snake_init(app_mem, app_mem_size);

    // TETRIS
    //tetris_init(app_mem, app_mem_size);

    // TEXT VIEWER
    //viewer_init(app_mem, app_mem_size);
    //viewer_c_str("Deep into that darkness peering, long I stood there, wondering, fearing, doubting\n\n - Edgar Allan Poe\n\nRead more at: https://www.brainyquote.com/quotes/edgar_allan_poe_393723");
    //viewer_draw();
    //screen_swap_fb();

    // MENU
    main_menu();

    // DIALER
    //dialer_init();
    //dialer_draw();
    //screen_swap_fb();
}

void loop() {
    // Virtual interrupt handling
    interrupts_tick();
}

