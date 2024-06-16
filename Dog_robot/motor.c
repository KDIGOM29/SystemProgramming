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
#include "motor.h"

#define LOW 0
#define HIGH 1
// Define motor pins
#define IN1 2 // GPIO 17 (WiringPi pin 0)
#define IN2 3 // GPIO 27 (WiringPi pin 2)
#define ENA 0 // GPIO 22 (WiringPi pin 3)

// Function to initialize motor pins
void setup() {
    // WiringPi 초기화
    wiringPiSetup();

    // 핀 모드 설정
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    // 소프트웨어 PWM 설정
    softPwmCreate(ENA, 0, 100); // ENA 핀, 초기값 0, 범위 0-100
    usleep(1000);

}

// void* t_set_motor(void* arg) {
//     int speed = (int)arg;
//     // 모터 속도 설정
//     digitalWrite(IN1, HIGH);
//     digitalWrite(IN2, LOW);
//     softPwmWrite(ENA, speed);
// }

void* t_dc_motor(void* arg) {
    int f_b = *(int*) arg;
    
    if(f_b == 0){
        printf("전진\n");
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        softPwmWrite(ENA, 100);

        delay(3000);
    }
    else{
        printf("후진\n");
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        softPwmWrite(ENA, 100);

        delay(3000);
    }

    printf("정지\n");
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    softPwmWrite(ENA, 0);

    delay(1000);
}
