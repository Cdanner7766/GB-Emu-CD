#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/clocks.h"
#include "mk_ili9225.h"
#include "sdcard.h"

/* LCD pins */
#define GPIO_CS   17
#define GPIO_CLK  18
#define GPIO_SDA  19
#define GPIO_RS   20
#define GPIO_RST  21
#define GPIO_LED  22

/* Button pins (active low with pull-up) */
#define GPIO_UP     2
#define GPIO_DOWN   3
#define GPIO_LEFT   4
#define GPIO_RIGHT  5
#define GPIO_A      6
#define GPIO_B      7
#define GPIO_SELECT 8
#define GPIO_START  9

/* RGB565 colours */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define RED     0xF800
#define GREEN   0x07E0
#define BLUE    0x001F
#define YELLOW  0xFFE0
#define ORANGE  0xFC00
#define GREY    0x7BEF

/* Required callbacks for mk_ili9225 */
void mk_ili9225_set_rst(bool s) { gpio_put(GPIO_RST, s); }
void mk_ili9225_set_rs(bool s)  { gpio_put(GPIO_RS,  s); }
void mk_ili9225_set_cs(bool s)  { gpio_put(GPIO_CS,  s); }
void mk_ili9225_set_led(bool s) { gpio_put(GPIO_LED, s); }
void mk_ili9225_spi_write16(const uint16_t *hw, size_t len) {
    spi_write16_blocking(spi0, hw, len);
}
void mk_ili9225_delay_ms(unsigned ms) { sleep_ms(ms); }

static void gpio_init_all(void)
{
    /* LCD */
    gpio_set_function(GPIO_CS,  GPIO_FUNC_SIO); gpio_set_dir(GPIO_CS,  true);
    gpio_set_function(GPIO_CLK, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_SDA, GPIO_FUNC_SPI);
    gpio_set_function(GPIO_RS,  GPIO_FUNC_SIO); gpio_set_dir(GPIO_RS,  true);
    gpio_set_function(GPIO_RST, GPIO_FUNC_SIO); gpio_set_dir(GPIO_RST, true);
    gpio_set_function(GPIO_LED, GPIO_FUNC_SIO); gpio_set_dir(GPIO_LED, true);
    gpio_set_slew_rate(GPIO_CLK, GPIO_SLEW_RATE_FAST);
    gpio_set_slew_rate(GPIO_SDA, GPIO_SLEW_RATE_FAST);

    /* Buttons */
    const uint btns[] = { GPIO_UP, GPIO_DOWN, GPIO_LEFT, GPIO_RIGHT,
                           GPIO_A,  GPIO_B,    GPIO_SELECT, GPIO_START };
    for (int i = 0; i < 8; i++) {
        gpio_set_function(btns[i], GPIO_FUNC_SIO);
        gpio_set_dir(btns[i], false);
        gpio_pull_up(btns[i]);
    }
}

/* Draw a fixed-width label then a status value on the same row */
static void draw_row(uint8_t y, const char *label, const char *value, uint16_t val_color)
{
    mk_ili9225_text((char *)label, 4,  y, WHITE, BLACK);
    mk_ili9225_fill_rect(80, y, 96, 8, BLACK);
    mk_ili9225_text((char *)value, 80, y, val_color, BLACK);
}

/* ---- Phase 1: colour flash --- */
static void phase_color_flash(void)
{
    const uint16_t cols[]   = { RED,   GREEN,  BLUE  };
    const char    *names[]  = { "RED", "GREEN","BLUE" };

    for (int i = 0; i < 3; i++) {
        mk_ili9225_fill(cols[i]);
        /* Centre-ish text so we can confirm pixel addressing works */
        mk_ili9225_text((char *)names[i], 68, 104, BLACK, cols[i]);
        sleep_ms(1000);
    }
}

/* ---- Phase 2: SD card ----- */
static bool phase_sdcard(void)
{
    sd_card_t *pSD = sd_get_by_num(0);
    FRESULT fr = f_mount(&pSD->fatfs, pSD->pcName, 1);
    if (fr == FR_OK) {
        f_unmount(pSD->pcName);
        return true;
    }
    return false;
}

/* ---- Phase 3: button monitor (runs forever) ---- */
static void phase_buttons(void)
{
    const uint    gpios[] = { GPIO_UP, GPIO_DOWN, GPIO_LEFT, GPIO_RIGHT,
                               GPIO_A,  GPIO_B,    GPIO_SELECT, GPIO_START };
    const char   *names[] = { "UP:    ", "DOWN:  ", "LEFT:  ", "RIGHT: ",
                               "A:     ", "B:     ", "SELECT:", "START: " };
    const uint8_t ys[]    = { 100, 112, 124, 136, 148, 160, 172, 184 };

    /* Draw static labels once */
    mk_ili9225_text("BUTTONS (press each):", 4, 88, YELLOW, BLACK);
    for (int i = 0; i < 8; i++)
        mk_ili9225_text((char *)names[i], 4, ys[i], WHITE, BLACK);

    while (true) {
        for (int i = 0; i < 8; i++) {
            bool pressed = !gpio_get(gpios[i]);
            mk_ili9225_fill_rect(72, ys[i], 72, 8, BLACK);
            if (pressed)
                mk_ili9225_text("PRESSED", 72, ys[i], GREEN, BLACK);
            else
                mk_ili9225_text("       ", 72, ys[i], GREY,  BLACK);
        }
        sleep_ms(40);
    }
}

int main(void)
{
    stdio_init_all();
    gpio_init_all();

    /* SPI at 4 MHz — safe for jumper wires */
    clock_configure(clk_peri, 0,
        CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLK_SYS,
        125 * 1000 * 1000, 125 * 1000 * 1000);
    spi_init(spi0, 4 * 1000 * 1000);
    spi_set_format(spi0, 16, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    mk_ili9225_init();

    /* Phase 1 — colour flash to confirm LCD + SPI are alive */
    phase_color_flash();

    /* Status screen */
    mk_ili9225_fill(BLACK);
    mk_ili9225_fill_rect(0, 0, 176, 12, BLUE);
    mk_ili9225_text("=== GB DIAGNOSTIC ===", 4, 2, WHITE, BLUE);

    draw_row(20, "LCD:   ", "OK", GREEN);

    /* Phase 2 — SD card */
    draw_row(36, "SD:    ", "testing...", YELLOW);
    bool sd_ok = phase_sdcard();
    draw_row(36, "SD:    ", sd_ok ? "OK" : "FAIL", sd_ok ? GREEN : RED);

    mk_ili9225_fill_rect(0, 52, 176, 1, GREY);   /* divider */

    /* Phase 3 — live button monitor */
    phase_buttons();

    /* unreachable */
    return 0;
}
