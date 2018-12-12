#include <vintc.h>
#include <vgfxc.h>
#include <font4x6c.h>
#include <vfsc.h>
#include <csvc.h>
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
// Logging
//-----------------------------------------------------------------------------
#define LOG(msg)          Serial.println(F(msg))
#define LOG_HEX(number)   Serial.println(number,HEX)
#define LOG_DEC(number)   Serial.println(number,DEC)
void log_err_imp(uint16_t item_encoded) {
    uint8_t item = (uint8_t)((item_encoded >> 8) & 0xff);
    uint8_t subitem = (uint8_t)((item_encoded >> 0) & 0xff);
    Serial.print(F("LOG_ERR("));
    Serial.print(item, DEC);
    Serial.print(F(","));
    Serial.print(subitem, DEC);
    Serial.println(F(");"));
}
#define LOG_ERR(item,subitem) log_err_imp(((item<<8)&0xff00)|((subitem<<0)&0x00ff))

//-----------------------------------------------------------------------------
// Serial
//-----------------------------------------------------------------------------
void serial_init() {
    Serial.begin(115200);
    while (!Serial);
}

//-----------------------------------------------------------------------------
// Virtual File System (VFS)
//-----------------------------------------------------------------------------
static uint8_t vfs[VFSC_VF_CALC_DATA_SIZE(4)];  // max 4 open handles
uint8_t VFSC_API vfs_read_byte(void *ctx, void *addr) {
  return pgm_read_byte(addr);
}
void vfs_init() {
  if (0 != vfsc_init(vfs, sizeof(vfs), vfs_data, (size_t)vfs_size, vfs_read_byte, NULL)) {
    LOG_ERR(1,0);
  }
}
#define open_hash(hash,...)     vfsc_open_hash(vfs,hash)
#define open(pathname,...)      vfsc_open(vfs,pathname)
#define lseek(fd,offset,whence) vfsc_lseek(vfs,fd,offset,whence)
#define read(fd,buf,count)      vfsc_read(vfs,fd,buf,count)
#define close(fd)               vfsc_close(vfs,fd)
#define SEEK_SET                VFSC_SEEK_SET
#define SEEK_CUR                VFSC_SEEK_CUR
#define SEEK_END                VFSC_SEEK_END

//-----------------------------------------------------------------------------
// CSV file
//-----------------------------------------------------------------------------
ssize_t CSVC_API csv_read_api(void *ctx, size_t offset, char *buffer, size_t buffer_size) {
  int fd = (int)ctx;
  if (fd < 0) {
    LOG_ERR(2,0);
    return -1;
  }
  if (offset != (size_t)lseek(fd, offset, SEEK_SET)) {
    LOG_ERR(2,1);
    return -1;
  }
  return read(fd, buffer, buffer_size);
}
size_t csv_file_size(int fd)
{
  size_t size = 0;
  off_t offset = lseek(fd, 0, SEEK_END);
  if (offset < 0) {
    LOG_ERR(2,2);
    return 0;
  }
  size = (size_t)offset;
  offset = lseek(fd, 0, SEEK_SET);
  if (offset != 0) {
    LOG_ERR(2,3);
    return 0;
  }
  return size;
}
size_t csv_row_count(int fd)
{
  size_t rows = 0;
  if (!csvc_dimensions(csv_file_size(fd), csv_read_api, (void *)fd, &rows, NULL)) {
    LOG_ERR(2,4);
    return 0;
  }
  return rows;
}
void csv_for_each_cell(int fd, CsvcCellFn cell_fn, void *cell_ctx, char *buffer, size_t buffer_size) {
  if (!csvc_for_each_cell(csv_file_size(fd), csv_read_api, (void *)fd, cell_fn, cell_ctx, buffer, buffer_size)) {
    LOG_ERR(2,7);
    return;
  }
}
void csv_read(int fd, size_t row, size_t column, char *buffer, size_t buffer_size) {
  buffer[0] = '\0';
  if (!csvc_read_cell(csv_file_size(fd), csv_read_api, (void *)fd, row, column, buffer, buffer_size)) {
    LOG_ERR(2,10);
  }
}
off_t csv_lseek(int fd, size_t row, size_t column) {
  off_t offset = -1;
  char unused = '\0';
  csv_read(fd, row, column, &unused, sizeof(unused));
  offset = lseek(fd, 0, SEEK_CUR);
  if (offset < 1) {
    return offset;
  }
  if (offset == lseek(fd, 0, SEEK_END)) {
      return offset;
  }
  return lseek(fd, offset - sizeof(unused), SEEK_SET);
}
size_t csv_row_size(int fd, size_t row) {
  off_t offset1 = csv_lseek(fd, row, 0);
  off_t offset2 = csv_lseek(fd, row + 1, 0);
  if (offset1 < 0) {
    return 0;
  }
  if (offset2 < offset1) {
    return 0;
  }
  return (size_t)(offset2 - offset1);
}

//-----------------------------------------------------------------------------
// Unique device ID
//-----------------------------------------------------------------------------
uint32_t device_id(void) {
    uint32_t dev_id = 0;
    int fd = open("/dev/id");
    if (fd >= 0) {
        if (4 == read(fd, &dev_id, sizeof(uint32_t))) {
            return dev_id;
        }
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Random
//-----------------------------------------------------------------------------
void rng_init() {
    // not great, got a better idea?
    // got a better idea??
    randomSeed(analogRead(0));
}
int16_t rng_random_s16(int16_t min, int16_t max) {
    return (int16_t) random((long)min, (long)(max + 1));
}

//-----------------------------------------------------------------------------
// Interrupt timers (virtual)
//-----------------------------------------------------------------------------
#define TMR_INTERRUPTS_MAX 8
#define INT_INVALID_HANDLE ((size_t)VINTC_INVALID_HANDLE)
static uint8_t interrupts[VINTC_CALC_DATA_SIZE(TMR_INTERRUPTS_MAX)] = {0};
uint32_t VINTC_API interrupts_get_tick_count_api(void *ctx) {
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
//typedef uint8_t button_key_t;
#define button_key_t uint8_t
#define BUTTON_KEY_LEFT     ((button_key_t)0)
#define BUTTON_KEY_OK       ((button_key_t)1)
#define BUTTON_KEY_RIGHT    ((button_key_t)2)
#define BUTTON_KEY_COUNT    (3)
//typedef uint8_t button_state_t;
#define button_state_t uint8_t
#define BUTTON_STATE_UP       ((button_state_t)0)
#define BUTTON_STATE_DOWN     ((button_state_t)1)
#define BUTTON_STATE_HOLD     ((button_state_t)2)
typedef void (*ButtonPressFn)(void *ctx, button_key_t key, button_state_t state);
static ButtonPressFn button_cb = NULL;
static void *button_ctx = NULL;
static button_key_t button_state[BUTTON_KEY_COUNT];
static uint32_t button_timestamp[BUTTON_KEY_COUNT];
void button_check(int pin, button_key_t key) {
    int val = digitalRead(pin);
    int mask = 1 << key;
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
    button_check(BUTTON_LEFT, BUTTON_KEY_LEFT);
    button_check(BUTTON_OK, BUTTON_KEY_OK);
    button_check(BUTTON_RIGHT, BUTTON_KEY_RIGHT);
}
void button_set_callback(void *func, void *ctx) {
    button_cb = func;
    button_ctx = ctx;
}
void button_init() {
    pinMode(BUTTON_LEFT, INPUT);
    pinMode(BUTTON_OK, INPUT);
    pinMode(BUTTON_RIGHT, INPUT);

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
void nokia_draw_img(void *data, size_t x = 0, size_t y = 0, size_t width = NOKIA_SCREEN_WIDTH, size_t height = NOKIA_SCREEN_HEIGHT) {
     for (size_t dy = 0; dy < height; dy++) {
        for (size_t dx = 0; dx < width; dx++) {
          size_t index = (dy * width) + dx;
          size_t index_byte = index / 8;
          size_t index_bit = 7 - (index % 8);
          uint8_t value = pgm_read_byte(&(((uint8_t*)data)[index_byte]));
          nokia_draw_pixel(x + dx, y + dy, ((value >> index_bit) & 0x1) != 0 ? true : false);
      }
    }
}
void nokia_draw_raw_hash(uint32_t hash, size_t x = 0, size_t y = 0, size_t width = NOKIA_SCREEN_WIDTH, size_t height = NOKIA_SCREEN_HEIGHT) {
    int fd = open_hash(hash);
    if (fd < 0) {
      LOG_ERR(4,0);
    }
    uint8_t value = 0;
    for (size_t dy = 0; dy < height; dy++) {
        for (size_t dx = 0; dx < width; dx++) {
            size_t index = (dy * width) + dx;
            size_t index_bit = 7 - (index % 8);
            if (0 == (index % 8)) {
                if (1 != read(fd, &value, sizeof(uint8_t))) {
                    LOG_ERR(4,1);
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
    
    nokia_draw_black();
    nokia_swap_fb();
    delay(1000);
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
    nokia_draw_clear();
}
void VGFX_API screen_draw_pixel_api(void *ctx, size_t x, size_t y, vgfx_color_t color) {
    nokia_draw_pixel(x, y, color > 0 ? true : false);
}
void VGFX_API screen_draw_char_api(void *ctx, size_t x, size_t y, char c, vgfx_color_t color, vgfx_color_t bg) {
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
void screen_fill_rect(size_t x, size_t y, size_t w, size_t h, bool color) {
    vg2d_fill_rect(&screen, x, y, w, h, color ? 1 : 0);
}
void screen_draw_rect(size_t x, size_t y, size_t w, size_t h, bool color) {
    vg2d_draw_rect(&screen, x, y, w, h, color ? 1 : 0);
}
void screen_draw_line(size_t x0, size_t y0, size_t x1, size_t y1, bool color) {
    vg2d_draw_line(&screen, x0, y0, x1, y1, color ? 1 : 0);
}
void screen_draw_pixel(size_t x, size_t y, bool color) {
    nokia_draw_pixel(x, y, color ? 1 : 0);
}
void screen_draw_char(size_t x, size_t y, char c, bool color, bool bg) {
    vg2d_draw_char(&screen, x, y, c, color ? 1 : 0, bg ? 1 : 0);
}
void screen_draw_string(size_t x, size_t y, const char *s, bool color, bool bg) {
    vg2d_draw_string(&screen, x, y, s, color ? 1 : 0, bg ? 1 : 0);
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
    dash_power = rng_random_s16(0, 4);
    dash_signal = rng_random_s16(0, 4);
}
void dash_init() {
    (void)interrupts_set(1000, dash_update, NULL);
}

//-----------------------------------------------------------------------------
// Unlocked bootloader message
//-----------------------------------------------------------------------------
void boot_unlocked_draw(void *mem, size_t mem_size) {
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
void schedule_menu_hash(uint32_t hash);
void link_menu();
void game_menu();
void main_menu();

//-----------------------------------------------------------------------------
// QR Code
//-----------------------------------------------------------------------------
#define qrcodegen_MAX_VERSION   6
#define QR_MEM_QR_CODE_SIZE     (qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_MAX_VERSION))
#define QR_MEM_TEMP_SIZE        (qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_MAX_VERSION))
#define QR_MEM_SIZE             (QR_MEM_QR_CODE_SIZE) // + QR_MEM_TEMP_SIZE)
static uint8_t *qrcode_mem = NULL;
static uint8_t *qrcode_temp_mem = NULL;
void qrcode_init(void *mem, size_t mem_size) {
  if (mem_size < QR_MEM_SIZE) {
    return;
  }
  qrcode_mem = (uint8_t*)mem;
  qrcode_temp_mem = nokia_screen_buffer; // dirty hack to save memory
}
void qrcode_draw(void *text) {
  bool ok = qrcodegen_encodeText(text, qrcode_temp_mem, qrcode_mem, qrcodegen_Ecc_LOW,
    qrcodegen_MAX_VERSION, qrcodegen_MAX_VERSION, qrcodegen_Mask_0, true);
  if (!ok) {
    LOG_ERR(6,0);
    return;
  }
  int size = qrcodegen_getSize(qrcode_mem);
  int x_offset = (SCREEN_WIDTH / 2) - (size / 2);
  int y_offset = 0; //(SCREEN_HEIGHT / 2) - (size / 2);
  screen_draw_clear();
  for (int y = 0; y < size; y++) {
    for (int x = 0; x < size; x++) {
      nokia_draw_pixel(x_offset + x, y_offset + y, qrcodegen_getModule(qrcode_mem, x, y) ? 1 : 0);
    }
  }
}
typedef void (*QRCodeFn)(void);
void qrcode_button_press(void *ctx, button_key_t key, button_state_t state) {
    QRCodeFn backfn = (QRCodeFn)ctx;
    if (BUTTON_STATE_DOWN == state) {
        backfn();
    }
}
void qrcode_display(void *text, void* backfn) {
    qrcode_draw(text);
    screen_swap_fb();
    button_set_callback(qrcode_button_press, (void*)backfn);
}

    
//-----------------------------------------------------------------------------
// Score (upload via QR code)
//-----------------------------------------------------------------------------
#define SCORE_CODE_SNAKE          0x55
#define SCORE_CODE_TETRIS         0xaa
#define SCORE_FORMAT_BUFFER_SIZE  (sizeof("3123456789"))
void score_format(char *buffer, size_t buffer_size, uint16_t score) {
    if (buffer_size < SCORE_FORMAT_BUFFER_SIZE) {
        buffer[0] = '\0';
        return;
    }
    sprintf(buffer, "%lu", ((unsigned long)score) * 100);
}
void score_token_encode(char *buffer, size_t buffer_size, const uint8_t *data, size_t data_size) {
    // for now, just a hex token
    if (buffer_size <= 0) {
        return;
    }
    if (buffer_size < ((data_size * 2) + 1)) {
        buffer[0] = '\0';
        return;
    }
    for (size_t i = 0; i < data_size; i++) {
        sprintf(&(buffer[i*2]), "%02x", data[i]);
    }
}
void score_token(char *buffer, size_t buffer_size, uint8_t game, uint16_t score) {
    uint8_t token[16];
    uint32_t token_device_id = device_id();
    uint32_t token_score = ((uint32_t)score) * 100;
    memset(token, 0, sizeof(token));
    memcpy(token + 0, &token_device_id, 4);
    memcpy(token + 4, &token_score, 4);
    memcpy(token + 8, &game, 1);
    // 7 bytes left for authentication
    // encode token
    score_token_encode(buffer, buffer_size, token, sizeof(token));
}
void score_upload(uint8_t game, uint16_t score, void *backfn) {
    char url[100];
    int fd = open("/text/score-url.txt");
    memset(url, 0, sizeof(url));
    if (fd >= 0) {
        if (read(fd, url, sizeof(url)-1) > 10) {
            score_token(url+strlen(url), (sizeof(url)-1)-strlen(url), game, score);
            qrcode_display(url, backfn);
            memset(url, 0, sizeof(url));
            memcpy_P(url, F("UPLOAD SCORE "), sizeof("UPLOAD SCORE "));
            screen_draw_string(0, SCREEN_HEIGHT - SCREEN_FONT_HEIGHT, url, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
            url[0] = ' ';
            score_format(url+1, sizeof(url)-1, score);
            screen_draw_string(SCREEN_WIDTH-(strlen(url) * SCREEN_FONT_WIDTH), SCREEN_HEIGHT - SCREEN_FONT_HEIGHT, url, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
            screen_swap_fb();
        }
        close(fd);
    }
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
static size_t snkc_mem_size = 0;
static size_t snkc_int_handle = INT_INVALID_HANDLE;
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
    return rng_random_s16(min, max);
}
void snake_start();
void snake_stop();
void snake_game_return(void) {
    snake_start();
}
void SNKC_API snake_game_over_api(void *ctx, uint16_t score) {
    snake_stop();
    score_upload(SCORE_CODE_SNAKE, score, snake_game_return);
}
void snake_draw_end() {
    screen_draw_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_COLOR_BLACK);
    screen_swap_fb();
}
void VINTC_API snake_tick_api(void *ctx) {
    if(!snkc_tick(snkc_mem)) {
        LOG_ERR(7,0);
    }
    if (INT_INVALID_HANDLE != snkc_int_handle) {
        snake_draw_end();
    }
}
void snake_stop() {
    interrupts_remove(snkc_int_handle);
    snkc_int_handle = INT_INVALID_HANDLE;
    memset(snkc_mem, 0, snkc_mem_size);
}
void snake_button_press(void *ctx, button_key_t key, button_state_t state) {
  int8_t new_direction = (int8_t)snake_direction;
  if ((BUTTON_KEY_LEFT == key) && (BUTTON_STATE_HOLD == state)) {
        // return to menu
        snake_stop();
        game_menu();
        return;
  }
  if (BUTTON_STATE_DOWN != state) {
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
          LOG_ERR(7,1);
      }
      snake_draw_end();
      break;
    case 1:
      if(!snkc_key_up(snkc_mem)) {
          LOG_ERR(7,2);
      }
      snake_draw_end();
      break;
    case 2:
      if(!snkc_key_right(snkc_mem)) {
          LOG_ERR(7,3);
      }
      snake_draw_end();
      break;
    case 3:
      if(!snkc_key_down(snkc_mem)) {
          LOG_ERR(7,4);
      }
      snake_draw_end();
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
static size_t ttrs_mem_size = 0;
static size_t ttrs_int_handle = INT_INVALID_HANDLE;
void TTRS_API tetris_draw_clear(void *ctx) {
    screen_draw_clear();
}
void TTRS_API tetris_draw_piece(void *ctx, int16_t x, int16_t y, TTRS_PIECE_TYPE piece) {
    x = TETRIS_GRID_OFFSET + (x * TETRIS_SQUARE_SIZE) + TETRIS_DISPLAY_OFFSET;
    y = TETRIS_GRID_OFFSET + (y * TETRIS_SQUARE_SIZE);
    screen_fill_rect(x, y, TETRIS_SQUARE_SIZE, TETRIS_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
void TTRS_API tetris_draw_next_piece(void *ctx, int16_t x, int16_t y, TTRS_PIECE_TYPE piece) {
    x = TETRIS_NEXT_PIECE_OFFSET + (x * TETRIS_NEXT_PIECE_SQUARE_SIZE);
    y = TETRIS_NEXT_PIECE_OFFSET + (y * TETRIS_NEXT_PIECE_SQUARE_SIZE);
    screen_draw_rect(x, y, TETRIS_NEXT_PIECE_SQUARE_SIZE, TETRIS_NEXT_PIECE_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
void TTRS_API tetris_draw_block(void *ctx, int16_t x, int16_t y) {
    x = TETRIS_GRID_OFFSET + (x * TETRIS_SQUARE_SIZE) + TETRIS_DISPLAY_OFFSET;
    y = TETRIS_GRID_OFFSET + (y * TETRIS_SQUARE_SIZE);
    screen_fill_rect(x, y, TETRIS_SQUARE_SIZE, TETRIS_SQUARE_SIZE, SCREEN_COLOR_BLACK);
}
int16_t TTRS_API tetris_random(void *ctx, int16_t min, int16_t max) {
    return rng_random_s16(min, max);
}
void tetris_start();
void tetris_stop();
void tetris_game_return(void) {
    tetris_start();
}
void TTRS_API tetris_game_over(void *ctx, uint16_t score) {
    tetris_stop();
    score_upload(SCORE_CODE_TETRIS, score, tetris_game_return);
}
void tetris_draw_end() {
    screen_draw_rect(TETRIS_DISPLAY_OFFSET, 0, TETRIS_DISPLAY_WIDTH, TETRIS_DISPLAY_HEIGHT, SCREEN_COLOR_BLACK);
    screen_swap_fb();
}
void VINTC_API tetris_tick(void *ctx) {
    screen_draw_clear();
    if(!ttrs_tick(ttrs_mem)) {
        LOG_ERR(8,0);
    }
    if (INT_INVALID_HANDLE != ttrs_int_handle) {
        tetris_draw_end();
    }
}
void tetris_stop() {
    interrupts_remove(ttrs_int_handle);
    ttrs_int_handle = INT_INVALID_HANDLE;
    memset(ttrs_mem, 0, ttrs_mem_size);
}
void tetris_button_press(void *ctx, button_key_t key, button_state_t state) {
  if ((BUTTON_KEY_LEFT == key) && (BUTTON_STATE_HOLD == state)) {
      // return to menu
      tetris_stop();
      game_menu();
      return;
  }
  if ((BUTTON_KEY_LEFT == key) && (BUTTON_STATE_DOWN == state)) {
      if(!ttrs_key_left(ttrs_mem)) {
          LOG_ERR(8,1);
      }
      tetris_draw_end();
      return;
  }
  if ((BUTTON_KEY_RIGHT == key) && (BUTTON_STATE_DOWN == state)) {
      if(!ttrs_key_right(ttrs_mem)) {
          LOG_ERR(8,2);
      }
      tetris_draw_end();
      return;
  }
  if ((BUTTON_KEY_OK == key) && (BUTTON_STATE_UP == state)) {
      if(!ttrs_key_rotate(ttrs_mem)) {
          LOG_ERR(8,4);
      }
      tetris_draw_end();
      return;
  }
  if ((BUTTON_KEY_OK == key) && (BUTTON_STATE_HOLD == state)) {
      if(!ttrs_key_drop(ttrs_mem)) {
          LOG_ERR(8,3);
      }
      tetris_draw_end();
      return;
  }
}
void tetris_init(void *mem, size_t mem_size) {
    ttrs_mem = (uint8_t*)mem;
    ttrs_mem_size = mem_size;
}
void tetris_start() {
    // Tetris game
    if (!ttrs_init(ttrs_mem, ttrs_mem_size))
    {
        LOG_ERR(8,5);
    }
    if (!ttrs_set_grid(ttrs_mem, TETRIS_GRID_WIDTH, TETRIS_GRID_HEIGHT))
    {
        LOG_ERR(8,6);
    }

    // Tetris will draw on the screen
    if (!ttrs_set_draw_clear(ttrs_mem, tetris_draw_clear, &screen)) {
        LOG_ERR(8,7);
    }
    if (!ttrs_set_draw_piece(ttrs_mem, tetris_draw_piece, &screen)) {
        LOG_ERR(8,8);
    }
    if (!ttrs_set_draw_next_piece(ttrs_mem, tetris_draw_next_piece, &screen)) {
        LOG_ERR(8,9);
    }
    if (!ttrs_set_draw_block(ttrs_mem, tetris_draw_block, &screen)) {
        LOG_ERR(8,10);
    }
    if (!ttrs_set_random(ttrs_mem, tetris_random, NULL)) {
        LOG_ERR(8,11);
    }
    if (!ttrs_set_game_over(ttrs_mem, tetris_game_over, NULL)) {
        LOG_ERR(8,12);
    }

    // Tetris buttons
    button_set_callback(tetris_button_press, NULL);

    // Tetris frame rate
    ttrs_int_handle = interrupts_set(700, tetris_tick, ttrs_mem);

    // Tetris reset
    if (!ttrs_reset(ttrs_mem))
    {
        LOG_ERR(8,13);
    }
}

//-----------------------------------------------------------------------------
// Text viewer
//-----------------------------------------------------------------------------
#define TEXT_VIEWER_CHARS_PER_LINE    (SCREEN_WIDTH/SCREEN_FONT_WIDTH)
#define TEXT_VIEWER_MEM_SIZE          (VWRC_CALC_DATA_SIZE(TEXT_VIEWER_CHARS_PER_LINE))
static uint8_t *vwrc_mem = NULL;
static size_t vwrc_mem_size = 0;
static size_t vwrc_data_size = 0;
static const char *vwrc_c_str = 0;
static off_t vwrc_csv_offset = 0;
static ssize_t vwrc_csv_row_size = 0;
ssize_t VWRC_API viewer_read_c_str_api(void *ctx, size_t offset, char *buffer, size_t buffer_size) {
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
    return (ssize_t)buffer_size;
}
ssize_t VWRC_API viewer_read_csv_row(void *ctx, size_t offset, char *buffer, size_t buffer_size) {
    int fd = (int)ctx;
    if (buffer_size >= vwrc_csv_row_size) {
        // can't read more than the amount of data we have
        buffer_size = vwrc_data_size;
    }
    if (offset >= vwrc_csv_row_size) {
        // read out of bounds
        LOG_ERR(9,50);
        return -1;
    }
    if ((offset + buffer_size) > vwrc_csv_row_size) {
        // reading the last few bytes at the end
        buffer_size = vwrc_csv_row_size - offset;
    }
    if (0 == buffer_size) {
        // no data was requested
        LOG_ERR(9,51);
        return 0;
    }
    (void) lseek(fd, vwrc_csv_offset + offset, SEEK_SET);
    for (size_t i = 0; i < buffer_size; i++) {
        if (1 != read(fd, &(buffer[i]), 1)) {
            return (ssize_t)i;
        }
        if (buffer[i] == ',') {
            // makes the schedule look nicer
            if (offset < 10) {
                // put a '-' between the times
                buffer[i] = '-';
            } else {
                buffer[i] = '\n';
            }
        }
    }
    return (ssize_t)buffer_size;
}
ssize_t VWRC_API viewer_read_file_api(void *ctx, size_t offset, char *buffer, size_t buffer_size) {
    int fd = (int)ctx;
    lseek(fd, offset, SEEK_SET);
    return read(fd, buffer, buffer_size);
}
void VWRC_API viewer_calc_string_view_api(void *ctx, const char *str, size_t *width, size_t *height) {
    if (width) {
        *width = (size_t)(SCREEN_FONT_WIDTH * strlen(str));
    }
    if (height) {
        *height = SCREEN_FONT_HEIGHT;
    }
}
void VWRC_API viewer_draw_string_api(void *ctx, size_t x, size_t y, const char *str) {
    screen_draw_string(x, y, str, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
}
void viewer_draw() {
    size_t row = 0;
    size_t row_count = 0;
    size_t rows_per_view = 0;
    size_t y = 0;
    size_t h = 0;
    screen_draw_clear();
    if (!vwrc_draw_view(vwrc_mem)) {
        LOG_ERR(9,0);
    }
    if (!vwrc_get_row(vwrc_mem, &row)) {
        LOG_ERR(9,1);
    }
    if (!vwrc_get_row_count(vwrc_mem, &row_count)) {
        LOG_ERR(9,2);
    }
    if (!vwrc_get_rows_per_view(vwrc_mem, &rows_per_view)) {
        LOG_ERR(9,3);
    }
    if (rows_per_view < row_count) {
      // progress bars required
      y = (SCREEN_HEIGHT * row) / row_count;
      h = (SCREEN_HEIGHT * rows_per_view) / row_count;
      screen_fill_rect(SCREEN_WIDTH - 2, y, 2, h, SCREEN_COLOR_BLACK);
    }
}
typedef void (*ViewerFn)(void);
void viewer_button_press(void *ctx, button_key_t key, button_state_t state) {
  ViewerFn backfn = (ViewerFn)ctx;
  if ((BUTTON_KEY_LEFT == key) && (BUTTON_STATE_HOLD == state)) {
      backfn();
      return;
  }
  if (BUTTON_STATE_DOWN != state) {
    return;
  }
  switch(key) {
    case BUTTON_KEY_LEFT:
      if (!vwrc_scroll_up(vwrc_mem)) {
          LOG_ERR(9,4);
      }
      viewer_draw();
      screen_swap_fb();
      break;
    case BUTTON_KEY_OK:
      backfn();
      break;
    case BUTTON_KEY_RIGHT:
      if (!vwrc_scroll_down(vwrc_mem)) {
          LOG_ERR(9,5);
      }
      viewer_draw();
      screen_swap_fb();
      break;
    default:
      break;
  }
}
void viewer_init(void *mem, size_t mem_size) {
    vwrc_mem = (uint8_t*)mem;
    vwrc_mem_size = mem_size;
}
void viewer_start(void* backfn) {

    // reset
    vwrc_data_size = 0;
    vwrc_c_str = NULL;

    // init
    if (!vwrc_init(vwrc_mem, vwrc_mem_size)) {
        LOG_ERR(9,6);
    }

    // view size (2 pixels for scroll bar)
    if (!vwrc_set_view(vwrc_mem, SCREEN_WIDTH - 2, SCREEN_HEIGHT)) {
        LOG_ERR(9,7);
    }

    // how to calculate the pixel size of a string
    if (!vwrc_set_calc_string_view(vwrc_mem, viewer_calc_string_view_api, NULL)) {
        LOG_ERR(9,8);
    }

    // how to draw the text in the view
    if (!vwrc_set_draw_string(vwrc_mem, viewer_draw_string_api, NULL)) {
        LOG_ERR(9,9);
    }

    // Viewer buttons
    button_set_callback(viewer_button_press, backfn);
}
void viewer_c_str(const char *text, void* backfn) {
    viewer_start(backfn);
    vwrc_c_str = text;
    vwrc_data_size = (size_t)strlen(text);
    if (!vwrc_set_text(vwrc_mem, vwrc_data_size, viewer_read_c_str_api, NULL)) {
        LOG_ERR(9,10);
    }
    viewer_draw();
    screen_swap_fb();
}
void viewer_csv_row(int fd, size_t row, void* backfn) {
    viewer_start(backfn);
    vwrc_csv_offset = csv_lseek(fd, row, 0);
    if (vwrc_csv_offset < 0) {
      LOG_ERR(9,11);
    }
    vwrc_csv_row_size = csv_row_size(fd, row);
    if (0 == vwrc_csv_row_size) {
      LOG_ERR(9,12);
    }
    if (!vwrc_set_text(vwrc_mem, vwrc_csv_row_size, viewer_read_csv_row, (void*)fd)) {
        LOG_ERR(9,13);
    }
    viewer_draw();
    screen_swap_fb();
}
void viewer_file(int fd, void* backfn) {
    viewer_start(backfn);
    off_t offset = lseek(fd, 0, SEEK_END);
    if (offset < 0) {
        LOG_ERR(9,14);
        ((ViewerFn)backfn)();
        return;
    }
    size_t file_size = (size_t)offset;
    if (!vwrc_set_text(vwrc_mem, file_size, viewer_read_file_api, (void*)fd)) {
        LOG_ERR(9,15);
        ((ViewerFn)backfn)();
        return;
    }
    viewer_draw();
    screen_swap_fb();
}

//-----------------------------------------------------------------------------
// Dialer
//-----------------------------------------------------------------------------
typedef struct {
    uint8_t dialer_x;
    uint8_t dialer_y;
    char dialer_number[13];
    char expect_number[20];
} dialer_mem_t;
int dialder_fd = -1;
#define DIALER_MEM_SIZE (sizeof(dialer_mem_t))
dialer_mem_t *dialer = NULL;
#define DIALER_BUTTON_OFFSET_X  18
#define DIALER_BUTTON_OFFSET_Y  9
#define DIALER_BUTTON_WIDTH     15
#define DIALER_BUTTON_HEIGHT    9
#define DIALER_KEY_BACK         0
#define DIALER_KEY_ENTER        1
char dialer_char(uint8_t x, uint8_t y) {
  char value = '0';
  if (0 == x) {
    return DIALER_KEY_BACK;
  }
  if (4 == x) {
    return DIALER_KEY_ENTER;
  }
  x--;
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
  size_t btn_x = 0;
  size_t btn_y = 0;
  size_t current_y = 0;
  bool color = SCREEN_COLOR_BLACK;
  bool bg = SCREEN_COLOR_WHITE;
  char label[4] = {0};
  screen_draw_clear();
  screen_draw_string(DIALER_BUTTON_OFFSET_X, 1, dialer->dialer_number, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
  for (uint8_t x = 0; x < 5; x++) {
    for (uint8_t y = 0; y < 4; y++) {
          if (0 == x) {
              btn_x = 0;
              btn_y = DIALER_BUTTON_OFFSET_Y;
              label[0] = '<';
              label[1] = '-';
              label[2] = '-';
          } else if (4 == x) {
              btn_x = SCREEN_WIDTH - DIALER_BUTTON_WIDTH;
              btn_y = DIALER_BUTTON_OFFSET_Y;
              label[0] = 'E';
              label[1] = 'N';
              label[2] = 'T';
          } else {
              btn_x = DIALER_BUTTON_OFFSET_X + ((x - 1) * (DIALER_BUTTON_WIDTH+1));
              btn_y = DIALER_BUTTON_OFFSET_Y + (y * (DIALER_BUTTON_HEIGHT+1));
              label[0] = ' ';
              label[1] = dialer_char(x, y);
              label[2] = ' ';
          }
          if (((0 == x) && (0 == dialer->dialer_x)) || ((4 == x) && (4 == dialer->dialer_x)) || ((x == dialer->dialer_x) && (y == dialer->dialer_y))) {
            screen_fill_rect(btn_x, btn_y, DIALER_BUTTON_WIDTH, DIALER_BUTTON_HEIGHT, SCREEN_COLOR_BLACK);
            color = SCREEN_COLOR_WHITE;
            bg = SCREEN_COLOR_BLACK;
          } else {
            screen_draw_rect(btn_x, btn_y, DIALER_BUTTON_WIDTH, DIALER_BUTTON_HEIGHT, SCREEN_COLOR_BLACK);
            color = SCREEN_COLOR_BLACK;
            bg = SCREEN_COLOR_WHITE;
          }
          label[3] = '\0';
          screen_draw_pixel(btn_x, btn_y, SCREEN_COLOR_WHITE);
          screen_draw_pixel(btn_x + DIALER_BUTTON_WIDTH - 1, btn_y, SCREEN_COLOR_WHITE);
          screen_draw_pixel(btn_x, btn_y + DIALER_BUTTON_HEIGHT - 1, SCREEN_COLOR_WHITE);
          screen_draw_pixel(btn_x + DIALER_BUTTON_WIDTH - 1, btn_y + DIALER_BUTTON_HEIGHT - 1, SCREEN_COLOR_WHITE);
          screen_draw_string(btn_x + 2, btn_y + 2, label, color, bg);
    }
  }
}
void dialer_stop() {

}
void dialer_start(void);
void dialer_return(void) {
    if (dialder_fd >= 0) {
        close(dialder_fd);
        dialder_fd = -1;
    }
    dialer_start();
}
bool dialer_check_P(const char *number, void *expected_P) {
    memcpy_P(dialer->expect_number, expected_P, sizeof(dialer->expect_number));
    return 0 == strcmp(number, dialer->expect_number);
}
#define DIALER_CHECK(number,expected)  dialer_check_P(number, F(expected))
#define DIAL_FILE(number,expected,pathname) \
    if (DIALER_CHECK(number, expected)) { \
        dialer_stop(); \
        dialder_fd = open(pathname); \
        viewer_file(dialder_fd, dialer_return); \
        return; \
    }
void dialer_action(const char *number) {
    DIAL_FILE(number, "*#0000#", "/dev/version");
    DIAL_FILE(number, "*#06#", "/dev/imei");
    memset(dialer->dialer_number, 0, sizeof(dialer->dialer_number));
    dialer_draw();
    screen_swap_fb();
}
void dialer_button_press(void *ctx, button_key_t key, button_state_t state) {
  char dialer_key = DIALER_KEY_BACK;
  if ((BUTTON_KEY_LEFT == key) && (BUTTON_STATE_HOLD == state)) {
      dialer_stop();
      main_menu();
      return;
  }
  if (BUTTON_STATE_DOWN != state) {
      return;
  }
  switch(key) {
    case BUTTON_KEY_LEFT:
      dialer->dialer_y++;
      if (dialer->dialer_y >= 4) {
        dialer->dialer_y = 0;
      }
      dialer_draw();
      screen_swap_fb();
      break;
    case BUTTON_KEY_RIGHT:
      dialer->dialer_x++;
      if (dialer->dialer_x >= 5) {
        dialer->dialer_x = 0;
      }
      dialer_draw();
      screen_swap_fb();
      break;
    case BUTTON_KEY_OK:
      dialer_key = dialer_char(dialer->dialer_x, dialer->dialer_y);
      if (DIALER_KEY_BACK == dialer_key) {
          dialer_stop();
          main_menu();
          return;
      } else if (DIALER_KEY_ENTER == dialer_key) {
          // enter
          dialer_action(dialer->dialer_number);
          return;
      } else if (strlen(dialer->dialer_number) < (sizeof(dialer->dialer_number) - 1)) {
          dialer->dialer_number[strlen(dialer->dialer_number)] = dialer_key;
      }
      dialer_draw();
      screen_swap_fb();
      break;
    default:
      break;
  }
}
void dialer_init(void *mem, size_t mem_size) {
    dialer = (dialer_mem_t*)mem;
}
void dialer_start() {
    memset(dialer, 0, sizeof(dialer_mem_t));
    button_set_callback(dialer_button_press, NULL);
    dialer_draw();
    screen_swap_fb();
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
static size_t tmnu_mem_size = 0;
static size_t tmnu_int_handle = INT_INVALID_HANDLE;
static char menu_title[12];
void menu_set_title(void *title) {
  memcpy_P(menu_title, title, sizeof(menu_title));
  menu_title[sizeof(menu_title)-1] = '\0';
}
void VWRC_API menu_calc_string_view_api(void *ctx, const char *str, size_t *width, size_t *height) {
    if (width) {
        *width = (size_t)(SCREEN_FONT_WIDTH * strlen(str)); // 4x6 font used
        *width += (MENU_STR_OFFSET_X * 2); // padding on sides
    }
    if (height) {
        *height = SCREEN_FONT_HEIGHT;
        *height += ((MENU_STR_OFFSET_Y * 2) - 1); // padding top/bottom
    }
}
void VWRC_API menu_draw_string_api(void *ctx, size_t x, size_t y, const char *str, TMNU_BOOL selected) {
    bool color = SCREEN_COLOR_BLACK;
    bool bg = SCREEN_COLOR_WHITE;
    if (TMNU_TRUE == selected) {
        color = SCREEN_COLOR_WHITE;
        bg = SCREEN_COLOR_BLACK;
        screen_fill_rect(MENU_OFFSET_X + x, MENU_OFFSET_Y + y, MENU_VIEW_WIDTH - 2, 6 + ((MENU_STR_OFFSET_Y * 2) - 1), bg);
    }
    screen_draw_string(MENU_OFFSET_X + x + MENU_STR_OFFSET_X, MENU_OFFSET_Y + y + MENU_STR_OFFSET_Y, str, color, bg);
}
void menu_draw() {
    size_t item = 0;
    size_t item_count = 0;
    size_t items_per_view = 0;
    size_t y = 0;
    size_t h = 0;
    size_t line_offset_x = 0;
    size_t line_width = 0;
    screen_draw_clear();
    screen_draw_line(0, 5, SCREEN_WIDTH-1, 5, SCREEN_COLOR_BLACK);
    menu_calc_string_view_api(NULL, menu_title, &line_width, &line_offset_x);
    line_offset_x = (SCREEN_WIDTH - line_width) / 2;
    screen_fill_rect(line_offset_x, 0, line_width, 9, SCREEN_COLOR_WHITE);
    screen_draw_string(line_offset_x + MENU_STR_OFFSET_X, MENU_STR_OFFSET_Y, menu_title, SCREEN_COLOR_BLACK, SCREEN_COLOR_WHITE);
    dash_power_draw();
    dash_signal_draw();
    if (!tmnu_draw_view(tmnu_mem)) {
        LOG_ERR(10,0);
    }
    if (!tmnu_get_item(tmnu_mem, &item)) {
        LOG_ERR(10,1);
    }
    if (!tmnu_get_item_count(tmnu_mem, &item_count)) {
        LOG_ERR(10,2);
    }
    if (!tmnu_get_items_per_view(tmnu_mem, &items_per_view)) {
        LOG_ERR(10,3);
    }
    if (items_per_view < item_count) {
      // progress bars required
      y = (MENU_VIEW_HEIGHT * item) / item_count;
      h = (MENU_VIEW_HEIGHT * 1) / item_count;
      screen_fill_rect(MENU_OFFSET_X + MENU_VIEW_WIDTH - 2, MENU_OFFSET_Y + y, 2, h, SCREEN_COLOR_BLACK);
    }
    screen_swap_fb();
}
void menu_button_press(void *ctx, button_key_t key, button_state_t state) {
  if (BUTTON_STATE_DOWN != state) {
      return;
  }
  switch(key) {
    case BUTTON_KEY_LEFT:
      if (!tmnu_key_up(tmnu_mem)) {
          LOG_ERR(10,4);
      }
      menu_draw();
      break;
    case BUTTON_KEY_RIGHT:
      if (!tmnu_key_down(tmnu_mem)) {
          LOG_ERR(10,5);
      }
      menu_draw();
      break;
    case BUTTON_KEY_OK:
      if (!tmnu_key_enter(tmnu_mem)) {
          LOG_ERR(10,6);
      }
      break;
    default:
      break;
  }
}
void VINTC_API menu_update(void *ctx) {
    menu_draw();
}
void menu_init(void *mem, size_t mem_size) {
    tmnu_mem = (uint8_t*)mem;
    tmnu_mem_size = mem_size;
}
void menu_start() {
    // init
    if (!tmnu_init(tmnu_mem, tmnu_mem_size)) {
        LOG_ERR(10,7);
    }

    // view size (2 pixels for scroll bar)
    if (!tmnu_set_view(tmnu_mem, MENU_VIEW_WIDTH - 2, MENU_VIEW_HEIGHT)) {
        LOG_ERR(10,8);
    }

    // how to calculate the pixel size of a string
    if (!tmnu_set_calc_string_view(tmnu_mem, menu_calc_string_view_api, NULL)) {
        LOG_ERR(10,9);
    }

    // how to draw the text in the view
    if (!tmnu_set_draw_string(tmnu_mem, menu_draw_string_api, NULL)) {
        LOG_ERR(10,10);
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
void menu_set(size_t item_count, TmnuMenuItemStringFn items, TmnuMenuItemOnSelectFn action, void *ctx) {
    if (!tmnu_set_menu_item_string(tmnu_mem, item_count, items, ctx)) {
        LOG_ERR(10,11);
    }
    if (!tmnu_set_on_select(tmnu_mem, action, ctx)) {
        LOG_ERR(10,12);
    }
}
#define MENU_ITEM(name,buffer,size) (memcpy_P(buffer,F(name),(sizeof(name)-1) > size ? size : (sizeof(name)-1)))

//-----------------------------------------------------------------------------
// SCHEDULE MENU
//-----------------------------------------------------------------------------
int csv_fd = -1;
void schedule_menu_items(void *ctx, size_t item, char *buffer, size_t buffer_size) {
    if (0 == item) {
      MENU_ITEM("..", buffer, buffer_size);
      return;
    }
    item--;
    csv_read(csv_fd, item, 0, buffer, buffer_size);
}
void schedule_menu_return(void) {
    schedule_menu_hash(0);
}
void schedule_menu_action(void *ctx, size_t item) {
  if (0 == item) {
    menu_stop();
    close(csv_fd);
    csv_fd = -1;
    main_menu();
    return;
  }
  item--;
  menu_stop();
  viewer_csv_row(csv_fd, item, schedule_menu_return);
}
void schedule_menu_hash(uint32_t hash) {
    if ((csv_fd < 0) && (0 != hash)) {
        csv_fd = open_hash(hash);
    }
    size_t items = csv_row_count(csv_fd);
    menu_start();
    menu_set_title(F("Schedule"));
    menu_set(items + 1, schedule_menu_items, schedule_menu_action, NULL);
    menu_draw();
}
#define schedule_menu(pathname)   schedule_menu_hash(VFSC_HASH(pathname))

//-----------------------------------------------------------------------------
// LINK MENU
//-----------------------------------------------------------------------------
void link_menu_items(void *ctx, size_t item, char *buffer, size_t buffer_size) {
    if (0 == item) {
      MENU_ITEM("..", buffer, buffer_size);
      return;
    }
    item--;
    int fd = open("/text/links.csv");
    csv_read(fd, item, 0, buffer, buffer_size);
    close(fd);
}
void link_menu_return(void) {
    link_menu();
}
void link_menu_action(void *ctx, size_t item) {
  char url[200] = {0};
  if (0 == item) {
    menu_stop();
    main_menu();
    return;
  }
  item--;
  int fd = open("/text/links.csv");
  csv_read(fd, item, 1, url, sizeof(url));
  close(fd);
  menu_stop();
  qrcode_display(url, link_menu_return);
}
void link_menu() {
    int fd = open("/text/links.csv");
    size_t items = csv_row_count(fd);
    close(fd);
    menu_start();
    menu_set_title(F("Links"));
    menu_set(items + 1, link_menu_items, link_menu_action, NULL);
    menu_draw();
}

//-----------------------------------------------------------------------------
// GAME MENU
//-----------------------------------------------------------------------------
#define GAME_MENU_COUNT 3
void game_menu_items(void *ctx, size_t item, char *buffer, size_t buffer_size) {
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
void game_menu_action(void *ctx, size_t item) {
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
    menu_draw();
}

//-----------------------------------------------------------------------------
// MAIN MENU
//-----------------------------------------------------------------------------
#define MAIN_MENU_COUNT 6
void main_menu_items(void *ctx, size_t item, char *buffer, size_t buffer_size) {
    switch(item) {
        case 0:
            MENU_ITEM("Dial", buffer, buffer_size);
            break;
        case 1:
            MENU_ITEM("Sched. Day 1", buffer, buffer_size);
            break;
        case 2:
            MENU_ITEM("Sched. Day 2", buffer, buffer_size);
            break;
        case 3:
            MENU_ITEM("Links", buffer, buffer_size);
            break;
        case 4:
            MENU_ITEM("Games", buffer, buffer_size);
            break;
        case 5:
            MENU_ITEM("Photos", buffer, buffer_size);
            break;
    }
}
void main_menu_action(void *ctx, size_t item) {
  switch(item) {
        case 0:
            menu_stop();
            dialer_start();
            break;
        case 1:
            menu_stop();
            schedule_menu("/text/schedule-day1.csv");
            break;
        case 2:
            menu_stop();
            schedule_menu("/text/schedule-day2.csv");
            break;
        case 3:
            menu_stop();
            link_menu();
            break;
        case 4:
            menu_stop();
            game_menu();
            break;
        case 5:
            break;
    }
}
void main_menu() {
    menu_start();
    menu_set_title(F("NOPIA 1337"));
    menu_set(MAIN_MENU_COUNT, main_menu_items, main_menu_action, NULL);
    menu_draw();
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
#define APP_MEM_SIZE_6 (DIALER_MEM_SIZE > APP_MEM_SIZE_5 ? DIALER_MEM_SIZE : APP_MEM_SIZE_5)
#define APP_MEM_SIZE    APP_MEM_SIZE_6
static uint8_t app_mem[APP_MEM_SIZE];
const static size_t app_mem_size = APP_MEM_SIZE;

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
    dialer_init(app_mem, app_mem_size);
    snake_init(app_mem, app_mem_size);
    tetris_init(app_mem, app_mem_size);
    qrcode_init(app_mem, app_mem_size);
    viewer_init(app_mem, app_mem_size);
    menu_init(app_mem, app_mem_size);

    LOG("NOPIA 1337");

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

    // MENU
    main_menu();
}

void loop() {
    interrupts_tick();
}

