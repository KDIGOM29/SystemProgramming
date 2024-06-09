#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define BUFFER_MAX 3
#define DIRECTION_MAX 256
#define VALUE_MAX 256

#define IN 0
#define OUT 1

#define LOW 0
#define HIGH 1

#define PIN 17
#define PIN2 18 //추가한 버튼
#define PIN3 27 //추가한 버튼

#define POUT 21 //일단 두고 진행한다

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}
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

int main(int argc, char *argv[]) {
  int state = 1;
  int prev_state = 1;
  int state2 = 1; //버튼 증가에 따른 조건 추가
  int prev_state2 = 1;
  int state3 = 1;
  int prev_state3 = 1;

  int light = 0;

  int serv_sock, clnt_sock = -1;
  struct sockaddr_in serv_addr, clnt_addr;
  socklen_t clnt_addr_size;
  char msg[10] = {0}; //출력 메세지 크기 변경

  if (argc != 2) {
    printf("Usage : %s <port>\n", argv[0]);
  }

  // Enable GPIO pins
  if (-1 == GPIOExport(PIN) || -1 == GPIOExport(POUT)) return (1);
  if (-1 == GPIOExport(PIN2) || -1 == GPIOExport(POUT)) return (1);
  if (-1 == GPIOExport(PIN3) || -1 == GPIOExport(POUT)) return (1);

  // Set GPIO directions
  if (-1 == GPIODirection(PIN, IN) || -1 == GPIODirection(POUT, OUT))
    return (2);
  if (-1 == GPIODirection(PIN2, IN) || -1 == GPIODirection(POUT, OUT))
    return (2);
  if (-1 == GPIODirection(PIN3, IN) || -1 == GPIODirection(POUT, OUT))
    return (2);

  if (-1 == GPIOWrite(POUT, 1)) return (3); //추후 다른 아웃풋이 사용되면 추가

  serv_sock = socket(PF_INET, SOCK_STREAM, 0);
  if (serv_sock == -1) error_handling("socket() error");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(atoi(argv[1]));

  if (bind(serv_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    error_handling("bind() error");

  if (listen(serv_sock, 5) == -1) error_handling("listen() error"); //클라이언트 요청 대기

  if (clnt_sock < 0) { //클라이언트 연결 수락
    clnt_addr_size = sizeof(clnt_addr);
    clnt_sock =
        accept(serv_sock, (struct sockaddr *)&clnt_addr, &clnt_addr_size);
    if (clnt_sock == -1) error_handling("accept() error");
  }

  printf("Connection established\n");

  while (1) {
    state = GPIORead(PIN); //버튼의 변화를 감지
    if (prev_state == 0 && state == 1) {
      light = (light + 1) % 2;
      snprintf(msg, 10, "%s", "CHECK1");
      write(clnt_sock, msg, sizeof(msg));
      printf("msg = %s\n", msg);
    }
    prev_state = state;
    usleep(500 * 100);
    state2 = GPIORead(PIN2); //버튼2의 변화를 감지
    if (prev_state2 == 0 && state2 == 1) {
      light = (light + 1) % 2;
      snprintf(msg, 10, "%s", "CHECK2");
      write(clnt_sock, msg, sizeof(msg));
      printf("msg = %s\n", msg);
    }
    prev_state2 = state2;
    usleep(500 * 100);

    state3 = GPIORead(PIN3); //버튼3의 변화를 감지
    if (prev_state3 == 0 && state3 == 1) {
      light = (light + 1) % 2;
      snprintf(msg, 10, "%s", "CHECK3");
      write(clnt_sock, msg, sizeof(msg));
      printf("msg = %s\n", msg);
    }
    prev_state3 = state3;
    usleep(500 * 100);
  }

  close(clnt_sock);
  close(serv_sock); //서버는 클라이언트와 서버 모두 소켓을 닫고 종료한다

  return (0);
}

