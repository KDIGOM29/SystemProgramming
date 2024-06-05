//돌아가는 코드
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>
#include <unistd.h>  // for sleep function

#define SERVO 1  // WiringPi pin 1, which corresponds to GPIO 18

enum RotateServer {
    RotateToLeft = 0,
    RotateToMiddle = 1,
    RotateToRight = 2,
};

void moveServo(int position) {
    softPwmWrite(SERVO, position);
}

double ServoPulseTime(enum RotateServer rotateServer) {
    switch (rotateServer) {
        case RotateToLeft:
            return 5;  // Adjusted value for 0 degrees
        case RotateToMiddle:
            return 15;  // Adjusted value for 90 degrees
        case RotateToRight:
            return 24;  // Adjusted value for 180 degrees
        default:
            return -1;  // Invalid position
    }
}

int main() {
    if (wiringPiSetup() == -1)
        return 1;

    softPwmCreate(SERVO, 0, 200);  // Create a software PWM pin

    while (1) {
        // Move to middle (90 degrees)
        moveServo(ServoPulseTime(RotateToMiddle));
        sleep(1);  // Wait for 1 second

        // Move to right (180 degrees)
        moveServo(ServoPulseTime(RotateToRight));
        sleep(1);  // Wait for 1 second

        // Move to left (0 degrees)
        moveServo(ServoPulseTime(RotateToLeft));
        sleep(1);  // Wait for 1 second
    }

    return 0;
}
