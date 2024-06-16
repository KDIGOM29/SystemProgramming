#ifndef GPIO_H
#define GPIO_H

#define IN 0
#define OUT 1
#define LOW 0
#define HIGH 1
#define VALUE_MAX 256
#define DIRECTION_MAX 256

int GPIOExport(int pin);
int GPIODirection(int pin, int dir);
int GPIOWrite(int pin, int value);
int GPIOUnexport(int pin);

#endif