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
#include "project_lib.h"
#include <wiringPi.h>
#include <softPwm.h>

#define IN 0
#define OUT 1

#define IN1 2 // GPIO 17 (WiringPi pin 0)
#define IN2 3 // GPIO 27 (WiringPi pin 2)
#define ENA 0 // GPIO 22 (WiringPi pin 3)

void buzz(int frequency, int duration, int pin)   // 듀레이션 0.1초 단위
{
    int period = 1000000 / frequency; // 주기 계산 (마이크로초 단위)
    int half_period = period / 2; // 반 주기
    int cycles = (duration * 100000) / period; // 주어진 시간 동안의 주기 수

    if (-1 == GPIOExport(pin)) {
    printf("gpio export err\n");
    return;
  }

  usleep(100000);

  if (-1 == GPIODirection(pin, OUT)){
    printf("gpio direction err\n");
    return;
  }

  for (int i = 0; i < cycles; i++) {
    GPIOWrite(pin, 1);
    usleep(half_period);
    GPIOWrite(pin, 0);
    usleep(half_period);
  }
}

float wave(int in_pin, int out_pin)
{
    clock_t start_t, end_t;
    double time;
    // Enable GPIO pins
    if (-1 == GPIOExport(out_pin) || -1 == GPIOExport(in_pin)) {
        printf("gpio export err\n");
        return (1);
    }
    // wait for writing to export file
    usleep(100000);

    // Set GPIO directions
    if (-1 == GPIODirection(out_pin, OUT) || -1 == GPIODirection(in_pin, IN)) {
        printf("gpio direction err\n");
        return (2);
    }

    GPIOWrite(out_pin, 0);
    usleep(10000);

    if (-1 == GPIOWrite(out_pin, 1)) {
      printf("gpio write/trigger err\n");
      return (3);
    }

    // 1sec == 1000000ultra_sec, 1ms = 1000ultra_sec
    usleep(10);
    GPIOWrite(out_pin, 0);
    while (GPIORead(in_pin) == 0) {
      start_t = clock();
    }
    while (GPIORead(in_pin) == 1) {
      end_t = clock();
    }

    time = (double)(end_t - start_t) / CLOCKS_PER_SEC;  // ms
    usleep(1000000);
    
    if (-1 == GPIOUnexport(out_pin) || -1 == GPIOUnexport(in_pin)) return (4);
    if (-1 == GPIOUnexport(21)) return (4);
    printf("complete\n");

    return (time / 2 * 34000);
}

void set_servo_angle(int angle) {
    // 각도를 펄스 폭(듀티 사이클)으로 변환 (1ms에서 2ms 사이)
    int duty_cycle = (1000000 + (angle * 1000));
    pwm_set_duty_cycle(duty_cycle);  // 듀티 사이클을 설정합니다.
}

void servo(int count) {
    pwm_export();  // PWM을 활성화합니다.
    usleep(100000);  // 잠시 대기하여 시스템이 PWM을 설정할 시간을 줍니다.
    pwm_set_period(20000000);  // 20ms 주기를 설정합니다.
    pwm_enable();  // PWM 출력을 활성화합니다.

    // 서보모터를 다양한 각도로 이동
    if(count == 3)
    {
      for (int angle = -1000; angle <= 0; angle += 10) {
        set_servo_angle(angle);  // 각도를 설정합니다.
        usleep(5000);  // 0.5초 대기합니다.
        }
    }
    else if(count == 2)
    {
      for (int angle = 0; angle <= 1000; angle += 10) {
        set_servo_angle(angle);  // 각도를 설정합니다.
        usleep(5000);  // 0.5초 대기합니다.
        }
    }
    else if(count == 1)
    {
      for (int angle = 1000; angle <= 3000; angle += 10) {
        set_servo_angle(angle);  // 각도를 설정합니다.
        usleep(5000);  // 0.5초 대기합니다.
        }
    }
    else if(count == 0)
    {
      for (int angle = 3000; angle >= -1000; angle -= 10) {
      set_servo_angle(angle);  // 각도를 설정합니다.
      usleep(5000);  // 0.5초 대기합니다.
      }
    }

    // PWM 비활성화
    pwm_disable();  // PWM 출력을 비활성화합니다.
    pwm_unexport();  // PWM을 비활성화합니다.

    return;
}

void setup() {
    // WiringPi 초기화
    wiringPiSetup();

    // 핀 모드 설정
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    // 소프트웨어 PWM 설정
    softPwmCreate(ENA, 0, 100); // ENA 핀, 초기값 0, 범위 0-100
}


void dc_motor(int f_b) {
  setup();

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
  return;
}
