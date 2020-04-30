#include <wiringPi.h>

#include <iostream>
#include <cstdio>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#define SERVO_PIN_1 18
#define SERVO_PIN_2 19
#define OFFSET_1 8
#define OFFSET_2 0

#define ANGLE_1_MAX 90
#define ANGLE_1_MIN -90
#define ANGLE_2_MAX 60
#define ANGLE_2_MIN -45


#define CAP_WIDTH 640
#define CAP_HEIGHT 480

//Green
#define H_MAX 90
#define H_MIN 40
#define S_MAX 255
#define S_MIN 60
#define V_MAX 255
#define V_MIN 50

using namespace std;
using namespace cv;

int angle_1 = 0;
int angle_2 = 0;

float k_x = 10.0/140.0; // x
float k_y = 10.0/240.0; // y

float alpha_x = 0.5;
float alpha_y = 0.5;


void servo_angle(int, int);
void visual_fb(int, int);

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

  angle_1 -= OFFSET_1;
  angle_2 -= OFFSET_2;

  servo_angle(SERVO_PIN_1, angle_1);
  servo_angle(SERVO_PIN_2, angle_2);

  return 0;
}

void servo_angle(int pin, int angle){
  // -90 < angle < 90, 24-115
  printf("angle: %d\n",angle);
  int duty = ((115.0-24.0)/180.0*angle + (115.0+24.0)/2.0);
  //printf("duty: %d\n",duty);
  pwmWrite(pin, duty);
}

void visual_fb(int x, int y){
  int move_angle_1 = k_x*x;
  int move_angle_2 = k_y*y;
  if(abs(move_angle_1) < 5) move_angle_1 = 0;
  if(abs(move_angle_2) < 3) move_angle_2 = 0;
  //cout << "move angle 1: " << move_angle_1 << "move angle 2: " << move_angle_2 << endl;
  angle_1 -= move_angle_1;
  angle_2 += move_angle_2;

  if(angle_1 < ANGLE_1_MIN) angle_1 = ANGLE_1_MIN;
  else if(angle_1 > ANGLE_1_MAX) angle_1 = ANGLE_1_MAX;
  if(angle_2 < ANGLE_2_MIN) angle_2 = ANGLE_2_MIN;
  else if(angle_2 > ANGLE_2_MAX) angle_2 = ANGLE_2_MAX;

  servo_angle(SERVO_PIN_1, angle_1);
  servo_angle(SERVO_PIN_2, angle_2);
}

int main(){
  init();

  VideoCapture cap(0);//デバイスのオープン
  if(!cap.isOpened())//カメラデバイスが正常にオープンしたか確認．
  {
      std::cerr << "Camera cannot open" << std::endl;
      //return -1;
  }

  //キャプチャサイズの設定
  cap.set(CV_CAP_PROP_FRAME_WIDTH, CAP_WIDTH);
  cap.set(CV_CAP_PROP_FRAME_HEIGHT, CAP_HEIGHT);

  Mat frame; //取得したフレーム

  int x_0_pre,y_0_pre,x_0_fil,y_0_fil = 0;

  int count_delay = 0;


  while(cap.read(frame))//無限ループ
    {

      namedWindow("raw");
      imshow("raw", frame);//画像を表示．
      
      
      // BGRからHSVへ変換
	    Mat hsv_frame, mask_frame, output_frame, mask_frame_fil;
	    cvtColor(frame, hsv_frame, CV_BGR2HSV);

      Scalar s_min = Scalar(H_MIN, S_MIN, V_MIN);
	    Scalar s_max = Scalar(H_MAX, S_MAX, V_MAX);
	    inRange(hsv_frame, s_min, s_max, mask_frame);

      morphologyEx(mask_frame, mask_frame_fil, MORPH_OPEN, getStructuringElement(MORPH_RECT, Size(5, 1))); //オープニング処理

      Moments mu = moments( mask_frame_fil, false );
      Point2f mc = Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );
      circle( mask_frame_fil, mc, 4, Scalar(100), 2, 4);

      float x_0, y_0;
      x_0 = 1*(mc.x - CAP_WIDTH/2.0);
      y_0 = 1*(mc.y - CAP_HEIGHT/2.0);
      x_0_fil = static_cast<int>((1-alpha_x)*x_0 + alpha_x*x_0_fil);
      y_0_fil = static_cast<int>((1-alpha_y)*y_0 + alpha_x*y_0_fil);
      //cout << "Window size  x: " << mask_frame_fil.shape[1] << " y: " <<  mask_frame_fil.shape[0] << endl;
      cout << "x0: " << x_0_fil << "  y0: " << y_0_fil << endl; 

      namedWindow("mask");
	    imshow("mask", mask_frame_fil);

      if(count_delay%4==0) visual_fb(x_0_fil, y_0_fil);


      x_0_pre = x_0_fil;
      y_0_pre = y_0_fil;

      if(count_delay==20000) count_delay = 0;
      count_delay++;
      const int key = cv::waitKey(1);
      
    }
    cv::destroyAllWindows();
    pwmWrite(SERVO_PIN_1,0);
    pwmWrite(SERVO_PIN_2,0);

/*
  while(1){ //EOF使う？
    
    printf("Input angle of servo 1\n");
    scanf("%f", &angle_1);
    angle_1 =- OFFSET_1;
    servo_angle(SERVO_PIN_1, angle_1);
    delay(500);
  }
  */

  return 0;
}