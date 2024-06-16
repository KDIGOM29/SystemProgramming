// gcc -o client client.c gpio.c motor.c lcd.c -lpthread -lwiringPi
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "gpio.h"
#include "lcd.h"
#include "motor.h"

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1
#define VALUE_MAX 256
#define DIRECTION_MAX 256

#define BUZZ_IN 21

void buzz(int frequency, int duration, int pin) {
    int period = 1000000 / frequency; 
    int half_period = period / 2; 
    int cycles = (duration * 1000) / period; 

    if (-1 == GPIOExport(pin)) {
        printf("gpio export err\n");
        return;
    }

    usleep(100000);

    if (-1 == GPIODirection(pin, OUT)) {
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

void *t_play_music() {
  int pin=21;
    // Note frequencies in Hz
    int C = 261;
    int D = 294;
    int E = 329;
    int F = 349;
    int G = 392;
    int A = 440;
    int B = 493;

    // Note durations in ms (quarter note, half note, etc.)
    int Q = 40; // Quarter note 원래 500
    int H = 2 * Q; // Half note
    int EIGHTH = Q / 2;

    // Twinkle Twinkle Little Star
    buzz(C, Q, pin); usleep(Q * 1000);
    buzz(C, Q, pin); usleep(Q * 1000);
    buzz(G, Q, pin); usleep(Q * 1000);
    buzz(G, Q, pin); usleep(Q * 1000);
    buzz(A, Q, pin); usleep(Q * 1000);
    buzz(A, Q, pin); usleep(Q * 1000);
    buzz(G, H, pin); usleep(H * 1000);

    buzz(F, Q, pin); usleep(Q * 1000);
    buzz(F, Q, pin); usleep(Q * 1000);
    buzz(E, Q, pin); usleep(Q * 1000);
    buzz(E, Q, pin); usleep(Q * 1000);
    buzz(D, Q, pin); usleep(Q * 1000);
    buzz(D, Q, pin); usleep(Q * 1000);
    buzz(C, H, pin); usleep(H * 1000);
}

void *t_bark(){
   buzz(1000,1,BUZZ_IN);
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

void *t_receive(void *arg) {
  int sock = *((int *)arg);
  char msg[10];
  int str_len;
  int thr_id;
  pthread_t buzz_thread, lcd_thread, motor_thread;
  int forward=0, backward=1;

  while (1) { //데이터 읽기 - 서버로부터 데이터를 읽는다
    str_len = read(sock, msg, sizeof(msg)); //msg 길이만큼 읽는다
    if (str_len == -1) error_handling("read() error");
    
    // receive CHECK1
    if(strncmp("CHECK1", msg, 6) == 0) {
      thr_id = pthread_create(&buzz_thread, NULL, t_play_music, NULL);
      if (thr_id < 0) {
        perror("thread create error : ");
        exit(0);
      }

      char *lcd_msg="Dog is barking";
      if (pthread_create(&lcd_thread, NULL, t_lcd_print, (void*)lcd_msg) != 0) {
        perror("thread create error : ");
        exit(0);
      }
      pthread_join(buzz_thread, NULL);
      pthread_join(lcd_thread, NULL);
    }
    // receive CHECK2
    else if(strncmp("CHECK2", msg, 6) == 0) {
      char *lcd_msg="Dog is moving forward";
      if (pthread_create(&lcd_thread, NULL, t_lcd_print, (void*)lcd_msg) != 0) {
        perror("thread create error : ");
        exit(0);
      }
      if (pthread_create(&motor_thread, NULL, t_dc_motor, (void*)&forward) != 0) {
        perror("thread create error : ");
        exit(0);
      }
      pthread_join(motor_thread, NULL);
      pthread_join(lcd_thread, NULL);
    }
    // receive CHECK3
    else if(strncmp("CHECK3", msg, 6) == 0) {
      char *lcd_msg="Dog is moving backward";
      if (pthread_create(&lcd_thread, NULL, t_lcd_print, (void*)lcd_msg) != 0) {
        perror("thread create error : ");
        exit(0);
      }
      if (pthread_create(&motor_thread, NULL, t_dc_motor, (void*)&backward) != 0) {
        perror("thread create error : ");
        exit(0);
      }
      pthread_join(motor_thread, NULL);
      pthread_join(lcd_thread, NULL);
    }
    else {
      printf("else\n");
    }
  }
}

int main(int argc, char *argv[]){
  int sock;
  int thr_id;
  struct sockaddr_in serv_addr;
  pthread_t receive_thread;

  if (argc != 3) {
    printf("Usage : %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect() error"); //서버에 연결 시도

  printf("Connection established\n");

  setup();

  thr_id = pthread_create(&receive_thread, NULL, t_receive, (void *)&sock);
  if (thr_id < 0) {
    perror("thread create error : ");
    exit(0);
  }
  
  pthread_join(receive_thread, NULL);
  close(sock);
  return 0; 
}