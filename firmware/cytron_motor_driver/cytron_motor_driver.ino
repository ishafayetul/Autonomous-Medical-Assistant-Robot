#define USE_USBCON  // Include this for Due before ROS
#include <ros.h>
#include <std_msgs/Int32.h>
#include <std_msgs/Float32.h>
#include <Encoder.h>
#include <geometry_msgs/Twist.h>
#include <PID_v1.h>

#include <std_msgs/Int16MultiArray.h>
#include <std_msgs/Int64MultiArray.h>
#include <std_msgs/Float64MultiArray.h>
#include <std_msgs/Float32MultiArray.h>
#include <std_msgs/Int16.h>


class BTSMotor{
  protected:
    int pwmf,pwmr;
  public:
    BTSMotor(int p1,int p2){
      pwmf=p1;pwmr=p2;
      pinMode(pwmf,OUTPUT);
      pinMode(pwmr,OUTPUT);
    }
    void setSpeed(int val){
      int spd;
      if(val<0) {
        spd=val*-1;
      }
      else {
        spd=val;
      }
      if(val>=255){
        spd=255;
      }
      else if(val<=-255){
        spd=255;
      }
      
      if(val>=0){
        for(int i=0;i<=spd;i++){
          analogWrite(pwmf,i);
          analogWrite(pwmr,0);
          
        } 
      }
      else if(val<0){
        for(int i=0;i<=spd;i++){
          analogWrite(pwmr,i);
          analogWrite(pwmf,0);
          
        } 
      }
    }
};
// Initialize PID paramaters

Encoder encoder_bright(43, 42);//BR
Encoder encoder_bleft(45, 44); //BL
Encoder encoder_fright(47, 46);//FR
Encoder encoder_fleft(49, 48);//FL


double Setpoint_fl, Input_fl, Output_fl;
double Setpoint_fr, Input_fr, Output_fr;
double Setpoint_bl, Input_bl, Output_bl;
double Setpoint_br, Input_br, Output_br;

double aggKp = 750, aggKi = 1500, aggKd = 0;


PID myPID_fl(&Input_fl, &Output_fl, &Setpoint_fl, aggKp, aggKi, aggKd, DIRECT);
PID myPID_fr(&Input_fr, &Output_fr, &Setpoint_fr, aggKp, aggKi, aggKd, DIRECT);
PID myPID_bl(&Input_bl, &Output_bl, &Setpoint_bl, aggKp, aggKi, aggKd, DIRECT);
PID myPID_br(&Input_br, &Output_br, &Setpoint_br, aggKp, aggKi, aggKd, DIRECT);



BTSMotor motor1(10,11); // left front //3,2
BTSMotor motor2(4,5); // right front //5,4
BTSMotor motor3(12,13); // left back // 6,7
BTSMotor motor4(2,3); // right back //9,8

bool wtf;

ros::NodeHandle nh;

std_msgs::Int64MultiArray enc_ticks;
std_msgs::Float64MultiArray vel_wheels;
ros::Publisher enc_ticks_pub("encoder_ticks", &enc_ticks);

void onPid_cb(const std_msgs::Int16MultiArray& cmd_msg)
{
  int p = cmd_msg.data[0];
  int i = cmd_msg.data[1];
  int d = cmd_msg.data[2];
  myPID_fl.SetTunings(p, i, d);
  myPID_fr.SetTunings(p, i, d);
  myPID_bl.SetTunings(p, i, d);
  myPID_br.SetTunings(p, i, d);
}


void onTwist(const std_msgs::Float32MultiArray& msg)
{

  float front_left_speed = msg.data[0];
  float front_right_speed = msg.data[1];
  float back_left_speed = msg.data[2];
  float back_right_speed = msg.data[3];
  Setpoint_fl = front_left_speed;
  Setpoint_fr = front_right_speed;
  Setpoint_bl = back_left_speed;
  Setpoint_br = back_right_speed;
  if (Setpoint_fl == 0 && Setpoint_fr == 0 && Setpoint_bl == 0 && Setpoint_br == 0) {
    wtf = true;
  }
  else {
    wtf = false;
  }

}


ros::Subscriber<std_msgs::Float32MultiArray> cmd_sub("set_vel", &onTwist);

ros::Subscriber<std_msgs::Int16MultiArray> pid_sub("pid_set", &onPid_cb);

// Move any motor function with speed_pwm value and pin numbers

void Move_motor(int speed_pwm, BTSMotor motor) {
  motor.setSpeed(speed_pwm);
}







// Initialize pins for forward movement

void setpins()
{

}


//void reset Integral error when we stop
void reset_pid_Ki()
{
  myPID_fl.SetMode(MANUAL);
  myPID_fr.SetMode(MANUAL);
  myPID_bl.SetMode(MANUAL);
  myPID_br.SetMode(MANUAL);
  Output_fl = 0;
  Output_fr = 0;
  Output_bl = 0;
  Output_br = 0;

  myPID_fl.SetMode(AUTOMATIC);
  myPID_fr.SetMode(AUTOMATIC);
  myPID_bl.SetMode(AUTOMATIC);
  myPID_br.SetMode(AUTOMATIC);


}

void setup() {

  SerialUSB.begin(115200);
  // 115200 baud rate
  nh.getHardware()->setBaud(115200);

  // Pid setup

  myPID_fl.SetOutputLimits(-255, 255);
  myPID_fr.SetOutputLimits(-255, 255);
  myPID_bl.SetOutputLimits(-255, 255);
  myPID_br.SetOutputLimits(-255, 255);

  myPID_fl.SetMode(AUTOMATIC);
  myPID_fr.SetMode(AUTOMATIC);
  myPID_bl.SetMode(AUTOMATIC);
  myPID_br.SetMode(AUTOMATIC);

  myPID_fl.SetSampleTime(20);
  myPID_fr.SetSampleTime(20);
  myPID_bl.SetSampleTime(20);
  myPID_br.SetSampleTime(20);


  // setup pins and fix encoders
  setpins();

  // ros node setup

  nh.initNode();

  //encoder ticks array initialiazation
  char dim0_label[] = "encoder_ticks";
  enc_ticks.layout.dim = (std_msgs::MultiArrayDimension *) malloc(sizeof(std_msgs::MultiArrayDimension) * 2);
  enc_ticks.layout.dim[0].label = dim0_label;
  enc_ticks.layout.dim[0].size = 4;
  enc_ticks.layout.dim[0].stride = 1 * 4;
  enc_ticks.data = (long long int *)malloc(sizeof(long long int) * 4);
  enc_ticks.layout.dim_length = 0;
  enc_ticks.data_length = 4;

  nh.advertise(enc_ticks_pub);
  //nh.advertise(vel_pub);
  nh.subscribe(cmd_sub);
  nh.subscribe(pid_sub);

}

unsigned long prev = 0;
long old_ct1 = 0;
long old_ct2 = 0;
long old_ct3 = 0;
long old_ct4 = 0;

float ticks_per_meter = 34000;

void loop() {

  // count encoder ticks
  long ct1 = encoder_fleft.read();
  long ct2 = encoder_fright.read();
  long ct3 = encoder_bleft.read();
  long ct4 = encoder_bright.read();

  enc_ticks.data[0] = ct1;
  enc_ticks.data[1] = ct2;
  enc_ticks.data[2] = ct3;
  enc_ticks.data[3] = ct4;
  // Publish encoder ticks to calculate odom on Jetson Nano side
  enc_ticks_pub.publish(&enc_ticks);


  // calculate time and current velocity

  unsigned long now = millis();
  Input_fl = (float(ct1 - old_ct1) / ticks_per_meter) / ((now - prev) / 1000.0);
  Input_fr = (float(ct2 - old_ct2) / ticks_per_meter) / ((now - prev) / 1000.0);
  Input_bl = (float(ct3 - old_ct3) / ticks_per_meter) / ((now - prev) / 1000.0);
  Input_br = (float(ct4 - old_ct4) / ticks_per_meter) / ((now - prev) / 1000.0);

  // Compute  Pid
  myPID_fl.Compute();
  myPID_fr.Compute();
  myPID_bl.Compute();
  myPID_br.Compute();


  if (wtf) {
    reset_pid_Ki();
  }

  // Move the motors with the output of the pid

  Move_motor(Output_fl, motor1);  // left front
  Move_motor(Output_fr, motor2);  // right front
  Move_motor(Output_bl, motor3);  // left back
  Move_motor(Output_br, motor4);  // right back

  // spin the ros node

  nh.spinOnce();
  // take the old encoder ticks and time for calculating velocity
  old_ct1 = ct1;
  old_ct2 = ct2;
  old_ct3 = ct3;
  old_ct4 = ct4;

  prev = now;

  delay(25);

}
