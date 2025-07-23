#ifndef LCD1602_H
#define LCD1602_H

#include <stdint.h>

int lcd1602_init(const char *i2c_dev, uint8_t addr);
void lcd1602_clear();
void lcd1602_home();
void lcd1602_write(const char *line1, const char *line2);
void lcd1602_close();

#endif // LCD1602_H
