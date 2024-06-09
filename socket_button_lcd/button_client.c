
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "lcd_sensor.h"

#define BUFFER_MAX 3
#define DIRECTION_MAX 256
#define VALUE_MAX 256

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define POUT 21

static int GPIOExport(int pin) {
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int fd;

  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open export for writing!\n");
    return (-1);
  }

  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return (0);
}

static int GPIODirection(int pin, int dir) {
  static const char s_directions_str[] = "in\0out";

  char path[DIRECTION_MAX] = "/sys/class/gpio/gpio%d/direction";
  int fd;

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);

  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio direction for writing!\n");
    return (-1);
  }

  if (-1 ==
      write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) {
    fprintf(stderr, "Failed to set direction!\n");
    return (-1);
  }

  close(fd);
  return (0);
}

static int GPIORead(int pin) {
  char path[VALUE_MAX];
  char value_str[3];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for reading!\n");
    return (-1);
  }

  if (-1 == read(fd, value_str, 3)) {
    fprintf(stderr, "Failed to read value!\n");
    return (-1);
  }

  close(fd);

  return (atoi(value_str));
}

static int GPIOWrite(int pin, int value) {
  static const char s_values_str[] = "01";
  char path[VALUE_MAX];
  int fd;

  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) {
    fprintf(stderr, "Failed to open gpio value for writing!\n");
    close(fd);
    return (-1);
  }

  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) {
    fprintf(stderr, "Failed to write value!\n");
    close(fd);
    return (-1);
  }
  close(fd);
  return (0);
}



void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

int main(int argc, char *argv[]) {
  int sock;
  struct sockaddr_in serv_addr;
  char msg[10]; //메세지 출력 크기 변경
  char on[2] = "1";
  int str_len;
  int light = 0;

  if (argc != 3) {
    printf("Usage : %s <IP> <port>\n", argv[0]);
    exit(1);
  }

  // Enable GPIO pins
  if (-1 == GPIOExport(POUT)) return (1);

  // Set GPIO directions
  if (-1 == GPIODirection(POUT, OUT)) return (2);

  sock = socket(PF_INET, SOCK_STREAM, 0);
  if (sock == -1) error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
  serv_addr.sin_port = htons(atoi(argv[2]));

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("connect() error"); //서버에 연결 시도

  printf("Connection established\n");

  while (1) { //데이터 읽기 - 서버로부터 데이터를 읽는다
    str_len = read(sock, msg, sizeof(msg)); //msg 길이만큼 읽는다
    if (str_len == -1) error_handling("read() error");

    printf("Receive message from Server : %s\n", msg); 


    if (strncmp( "CHECK2", msg, 6) == 0){
        fd = wiringPiI2CSetup(I2C_ADDR);
        lcd_init(); // LCD 초기화
        light = 1;
        lcdLoc(LINE1);
        typeln("Hello World!");
    }
    else{
        fd = wiringPiI2CSetup(I2C_ADDR);
        lcd_init(); // LCD 초기화ClrLcd();
        light = 0;
        lcdLoc(LINE1);
        typeln("Bark! Bark! Bark");
        lcdLoc(LINE2);
        typeln("Hungry");

    }

    GPIOWrite(POUT, light);
  }
  close(sock);

  return (0);
}

