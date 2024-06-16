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

extern float wave(int in_pin, int out_pin);                 // 초음파 센서 작동
extern void buzz(int frequency, int duration, int pin);     //스피커 작동
extern void servo();
extern void dc_motor(int f_b);

int sock;
int count_snack = 3;

void snack()
{
  usleep(1000);

  printf("칭찬\n");

  buzz(392,2,21);
  buzz(440,2,21);
  buzz(659,2,21);
  buzz(587,2,21);
  buzz(523,2,21);
  buzz(494,2,21);

  printf("간식제공\n");

  if(count_snack > 0)
  {
    servo(count_snack);

    count_snack--;

  }
  else
  {
    printf("간식이 없습니다. 리필이 필요합니다.\n");
    servo(count_snack);
    count_snack += 3;
    printf("간식을 채웁니다. 남은 간식 = %d개\n", count_snack);
  }

  printf("남은 간식 = %d개", count_snack);

  if(count_snack == 3)
  {
    write(sock, "snack3", 6);
  }
  else if(count_snack == 2)
  {
    write(sock, "snack2", 6);
  }
  else if(count_snack == 1)
  {
    write(sock, "snack1", 6);
  }
  else if(count_snack == 0)
  {
    write(sock, "snack0", 6);
  }
}

void happy()
{
  float distance, temp;

  printf("놀이 모드 실행\n");

  printf("이리와 출력\n");
  buzz(523,3,21);
  buzz(587,3,21);
  buzz(659,3,21);

  for(int i = 0; i < 10; i++)
  {
    distance = wave(24, 23);
    printf("거리 측정 = %.2f\n", distance);
    usleep(1000*1000);

    temp = wave(24,23);
    printf("거리 측정 = %.2f\n", temp);

    if((distance - temp) >= 5)
    {
      dc_motor(0);
    }
    else if((distance - temp) <= 10)
    {
      printf("이리와 출력\n");

      buzz(523,3,21);
      buzz(587,3,21);
      buzz(659,3,21);
    }

    printf("%d번\n", i);
    usleep(1000);
  }

  printf("놀이모드 종료\n");
}

void stop()
{
    float distance, temp;

    printf("멈춰 모드\n");
    
    printf("멈춰 출력\n");
    buzz(150,5,21);
    buzz(200,5,21);

    distance = wave(24, 23);
    printf("거리 측정 = %.2f\n", distance);

    printf("2초 대기\n");

    for(int i = 0; i < 2; i++)
    {
      printf("%d초\n", 2-i);
      usleep(1000*1000);
    }

    temp = wave(24,23);
    printf("거리 측정 = %.2f\n", temp);

    if((distance - temp) < 5 && (distance - temp) > -5)
    {
        snack();    
    }
    else
    {
      printf("다시 멈추라고 명령하기\n");
      stop();
    }
}

void error_handling(char *message) {
  fputs(message, stderr);
  fputc('\n', stderr);
  exit(1);
}

int main(int argc, char *argv[])
{
  struct sockaddr_in serv_addr;
  char msg[10]; //메세지 출력 크기 변경
  char on[2] = "1";
  int str_len;

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

  while (1) { //데이터 읽기 - 서버로부터 데이터를 읽는다
    str_len = read(sock, msg, sizeof(msg)); //msg 길이만큼 읽는다
    if (str_len == -1) error_handling("read() error");
    printf("Receive message from Server : %s\n", msg); 
    
    if(strncmp( "CHECK1", msg, 6) == 0)         // 멈춰 모드     버튼 1
    {
      stop();
    }
    else if(strncmp( "CHECK2", msg, 6) == 0)   // 놀이 모드    버튼 2
    {
      happy();
    }
    else if(strncmp( "CHECK3", msg, 6) == 0)   // 앞으로 이동  버튼 3
    {
      dc_motor(0);
    }
    else if(strncmp( "CHECK4", msg, 6) == 0)   // 뒤로 이동   버튼 4
    { 
      dc_motor(1);
    }
    //5번 버튼 신호 들어올 경우 사진 촬영
    else if (strncmp( "CHECK5", msg, 6) == 0) {
      printf("Button pressed! 촬영중...\n");
      system("libcamera-still --nopreview -o test.jpg");
      printf("test.jpg로 사진이 저장되었습니다. \n");
  }

  close(sock);

 return (0);
}
