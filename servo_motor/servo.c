#include <stdio.h>       // stdio.h 파일 포함( printf() 사용하기 위해 )
#include <wiringPi.h>    // wiringPi.h 파일 포함
#include <softPwm.h>

#define SERVO 1          // wiringPi GPIO 1번  = Python(BCM) 18번

int main()
{
   char str;

   if(wiringPiSetup() == -1)
      return 1;

   softPwmCreate(SERVO, 0, 200);  // 주기를 200으로 하면 20ms 주기를 가지며, 20ms 마다
                                  // 신호가 나오므로 50hz로 볼 수 있다

   while(1)
   {
      fputs("select c(Center)/r(Right)/l(Left)/q(Quit) : ", stdout);
      scanf("%c", &str);
      getchar();

      if(str == 'C' || str == 'c') 
         softPwmWrite(SERVO, 15); // 0 degree 
      else if(str == 'R' || str == 'r') 
         softPwmWrite(SERVO, 24); // 90 degree
      else if(str == 'L' || str == 'l') 
         softPwmWrite(SERVO, 5);  // -90 degree
      else if(str == 'Q' || str == 'q') 
         return 0;
   }

   return 0;
}
