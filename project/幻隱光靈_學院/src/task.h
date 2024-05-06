#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <ESP32Servo.h>
#include <Stepper.h>
#include <DFRobotDFPlayerMini.h>
extern Servo myServo; //   Create Servo object to control the servo
extern QueueHandle_t queueTimer;
extern String Timer_newSecond;
extern tm tmTimer;
extern uint16_t Timer_status;
extern DFRobotDFPlayerMini myDFPlayer[2];
extern JsonDocument docWeaponLight;
extern int16_t BallTime;
extern uint16_t SoundPlayerLevel[2];
typedef struct CRC16_parameter_t
{
    int16_t Polynomial = 0x8005;   // 多項式
    int16_t Initialvalue = 0xFFFF; // 初始值
    int16_t XORvalue = 0x0000;     // 結果異或值
    bool Inputinversion = true;    // 輸入反轉
    bool Outputinversion = true;   // 輸出反轉
} CRC16;
void CoinDispenser(uint16_t time = 0);
void taskBallDispenser(void *pvParam);
void taskTimer(void *pvParam);
void SoundPlayer(uint16_t playerNumber);
void set74HC595(String newTime);
void printDetail(uint8_t type, int value);
void taskWeaponLight(void *pvParam);
void taskSignalExtender();
void myshiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
void taskFQ512(uint16_t cmd);
uint16_t CRC_16(byte *data, uint16_t len, CRC16_parameter_t *CRC16_parameter);
#endif