#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <ESP32Servo.h>
#include <Stepper.h>
#include <DFRobotDFPlayerMini.h>
extern Servo myServo; //   Create Servo object to control the servo
extern QueueHandle_t queueTimer;
extern String Timer_newSecond; 
extern   tm tmTimer;
extern uint16_t Timer_status; 
extern DFRobotDFPlayerMini myDFPlayer;
void CoinDispenser(uint16_t time = 0);
void BallDispenser(uint16_t time = 0);
void taskTimer(void *pvParam);
void SoundPlayer();
void set74HC595(String newTime);
void printDetail(uint8_t type, int value);
#endif