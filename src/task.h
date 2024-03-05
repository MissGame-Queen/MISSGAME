#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <ESP32Servo.h>
extern Servo myServo; //   Create Servo object to control the servo
void CoinDispenser(uint16_t time=0);

#endif