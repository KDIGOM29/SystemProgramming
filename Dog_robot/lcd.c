#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lcd.h"

int fd;

void slice(const char* str, char* result, size_t start, size_t end) {
    strncpy(result, str + start, end - start);
}

void ClrLcd(void) {
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

void lcdLoc(int line) {
    lcd_byte(line, LCD_CMD);
}

void typeChar(char val) {
    lcd_byte(val, LCD_CHR);
}

void typeln(const char *s) {
    while (*s) lcd_byte(*(s++), LCD_CHR);
}

void lcd_byte(int bits, int mode) {
    int bits_high;
    int bits_low;

    bits_high = mode | (bits & 0xF0) | LCD_BACKLIGHT;
    bits_low = mode | ((bits << 4) & 0xF0) | LCD_BACKLIGHT;

    wiringPiI2CWrite(fd, bits_high);
    lcd_toggle_enable(bits_high);

    wiringPiI2CWrite(fd, bits_low);
    lcd_toggle_enable(bits_low);
}

void lcd_toggle_enable(int bits) {
    delayMicroseconds(500);
    wiringPiI2CWrite(fd, (bits | ENABLE));
    delayMicroseconds(500);
    wiringPiI2CWrite(fd, (bits & ~ENABLE));
    delayMicroseconds(500);
}

void lcd_init() {
    lcd_byte(0x33, LCD_CMD);
    lcd_byte(0x32, LCD_CMD);
    lcd_byte(0x06, LCD_CMD);
    lcd_byte(0x0C, LCD_CMD);
    lcd_byte(0x28, LCD_CMD);
    lcd_byte(0x01, LCD_CMD);
    delayMicroseconds(500);
}

void* t_lcd_print(void* arg) {
    char* msg = (char*)arg;
    char msgtmp[16];

    if (wiringPiSetup() == -1) exit(1);
    fd = wiringPiI2CSetup(I2C_ADDR);
    lcd_init();

    lcdLoc(LINE1);
    if (strlen(msg) > 16) {
        slice(msg, msgtmp, 0, 16);
        typeln(msgtmp);
        lcdLoc(LINE2);
        slice(msg, msgtmp, 16, 32);
        typeln(msgtmp);
    } else {
        typeln(msg);
    }

    delay(5000);
    ClrLcd();
}