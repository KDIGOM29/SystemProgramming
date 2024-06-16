#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdlib.h>
#include <stdio.h>

// LCD 주소 설정
#define I2C_ADDR 0x27

// mode 설정
#define LCD_CHR 1 // 데이터 보내기
#define LCD_CMD 0 // 명령 보내기
#define LINE1 0x80
#define LINE2 0xC0

#define LCD_BACKLIGHT 0x08 // 백라이트 켜기(08) 끄기(00)

#define ENABLE 0b00000100 // 활성화 비트

void lcd_init(void);
void lcd_byte(int bits, int mode);
void lcd_toggle_enable(int bits);

void ClrLcd(void);
void lcdLoc(int line);
void typeln(const char *s);
void typeChar(char val);
int fd;

// LCD 지우기 및 홈 위치로 이동
void ClrLcd(void) {
    lcd_byte(0x01, LCD_CMD);
    lcd_byte(0x02, LCD_CMD);
}

// LCD의 위치로 이동
void lcdLoc(int line) {
    lcd_byte(line, LCD_CMD);
}

// 현재 위치에서 LCD에 문자 출력
void typeChar(char val) {
    lcd_byte(val, LCD_CHR);
}

// LCD에 문자열 출력
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
