#ifndef MOTOR_H
#define MOTOR_H

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <linux/i2c-dev.h>
#include <wiringPi.h>
#include <softPwm.h>

// Function to initialize motor pins
void setup();
void* t_dc_motor(void* arg);

#endif