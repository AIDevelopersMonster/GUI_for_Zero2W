#include "lcd1602.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#define LCD_BACKLIGHT 0x08
#define ENABLE 0x04
#define LCD_CMD 0
#define LCD_DATA 1

static int i2c_fd = -1;
static uint8_t i2c_addr = 0x27;

// Прототипы внутренних функций
static void lcd_write_byte(uint8_t data, uint8_t mode);
static void lcd_toggle_enable(uint8_t data);
static void lcd_send_cmd(uint8_t cmd);
static void lcd_send_data(uint8_t data);
static void lcd_set_cursor(int col, int row);

int lcd1602_init(const char *i2c_dev, uint8_t addr) {
    i2c_addr = addr;

    if ((i2c_fd = open(i2c_dev, O_RDWR)) < 0) {
        perror("Unable to open I2C device");
        return -1;
    }

    if (ioctl(i2c_fd, I2C_SLAVE, i2c_addr) < 0) {
        perror("Unable to select I2C device");
        return -1;
    }

    // Инициализация LCD (4-bit mode)
    usleep(50000);
    lcd_send_cmd(0x33);
    lcd_send_cmd(0x32);
    lcd_send_cmd(0x28);
    lcd_send_cmd(0x0C);
    lcd_send_cmd(0x06);
    lcd_send_cmd(0x01);
    usleep(2000);

    return 0;
}

void lcd1602_clear() {
    lcd_send_cmd(0x01);
    usleep(2000);
}

void lcd1602_home() {
    lcd_send_cmd(0x02);
    usleep(2000);
}

void lcd1602_write(const char *line1, const char *line2) {
    lcd_set_cursor(0, 0);
    for (int i = 0; i < 16 && line1[i]; i++) {
        lcd_send_data(line1[i]);
    }

    lcd_set_cursor(0, 1);
    for (int i = 0; i < 16 && line2[i]; i++) {
        lcd_send_data(line2[i]);
    }
}

void lcd1602_close() {
    if (i2c_fd >= 0)
        close(i2c_fd);
}

// ===== Внутренние функции =====

static void lcd_send_cmd(uint8_t cmd) {
    lcd_write_byte(cmd & 0xF0, LCD_CMD);
    lcd_write_byte((cmd << 4) & 0xF0, LCD_CMD);
}

static void lcd_send_data(uint8_t data) {
    lcd_write_byte(data & 0xF0, LCD_DATA);
    lcd_write_byte((data << 4) & 0xF0, LCD_DATA);
}

static void lcd_write_byte(uint8_t data, uint8_t mode) {
    uint8_t buf = data | LCD_BACKLIGHT | (mode ? 0x01 : 0x00);
    if (write(i2c_fd, &buf, 1) != 1) return;
    lcd_toggle_enable(buf);
}

static void lcd_toggle_enable(uint8_t data) {
    uint8_t buf = data | ENABLE;
    write(i2c_fd, &buf, 1);
    usleep(500);
    buf = data & ~ENABLE;
    write(i2c_fd, &buf, 1);
    usleep(100);
}

static void lcd_set_cursor(int col, int row) {
    static const uint8_t row_offsets[] = {0x00, 0x40};
    lcd_send_cmd(0x80 | (col + row_offsets[row]));
}
