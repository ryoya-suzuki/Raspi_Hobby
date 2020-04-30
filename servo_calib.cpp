#include <stdio.h>
#include <wiringPi.h>

#include <iostream>

#define SERVO_PIN_1 18
#define SERVO_PIN_2 19
#define OFFSET_1 0
#define OFFSET_2 0


using namespace std;

float angle_1 = 0;
float angle_2 = 0;

void servo_angle(int, float);

int init(void){
  if (wiringPiSetupGpio() == -1) {
     printf("cannot setup gpio.");
    return -1;
  }

  pinMode(SERVO_PIN_1, PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);
  pwmSetClock(400);
  pwmSetRange(1024);

  pinMode(SERVO_PIN_2, PWM_OUTPUT);
  pwmSetMode(PWM_MODE_MS);
  pwmSetClock(400);
  pwmSetRange(1024);

  servo_angle(SERVO_PIN_1, 0.0);
  servo_angle(SERVO_PIN_2, 0.0);

  return 0;
}

void servo_angle(int pin, float angle){
  // -90 < angle < 90, 24-115
  printf("angle: %f\n",angle);
  int duty = ((115.0-24.0)/180.0*angle + (115.0+24.0)/2.0);
  printf("duty: %d\n",duty);
  pwmWrite(pin, duty);
}

int main(){
  init();

  while(1){ //EOF使う？
    
    printf("Input angle of servo 1, servo 2\n");
    scanf("%f", &angle_1);
    scanf("%f", &angle_2);
    servo_angle(SERVO_PIN_1, angle_1);
    servo_angle(SERVO_PIN_2, angle_2);
    delay(500);
  }

  return 0;
}