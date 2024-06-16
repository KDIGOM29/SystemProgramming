#ifndef LCD_H
#define LCD_H

#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define I2C_ADDR 0x27
#define LCD_CHR 1 
#define LCD_CMD 0
#define LINE1 0x80
#define LINE2 0xC0
#define LCD_BACKLIGHT 0x08 
#define ENABLE 0b00000100 

void slice(const char* str, char* result, size_t start, size_t end);

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

void ClrLcd(void);
void lcdLoc(int line);
void typeln(const char *s);
void typeChar(char val);

void* t_lcd_print(void* arg);

#endif