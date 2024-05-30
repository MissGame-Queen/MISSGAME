#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <Audio.h>
#include <SPIFFS.h>   //內建的空間儲存庫
extern Servo myServo; //   Create Servo object to control the servo
extern QueueHandle_t queueTimer;
extern String Timer_newSecond;
extern tm tmTimer;
extern uint16_t Timer_status;
extern JsonDocument docWeaponLight;
extern uint16_t SoundPlayerLevel[2];
extern String SoundPlayerName[2];
extern SemaphoreHandle_t xMutexPCM5102;
extern Audio *audioPCM5102;
extern QueueHandle_t queuePCM5102;
extern QueueHandle_t queueDFPlayer;
extern QueueHandle_t queueBallTime;
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
void taskPCM5102(void *pvParam);
void set74HC595(String newTime);
void printDetail(uint8_t type, int value);
void taskWeaponLight(void *pvParam);
void taskSignalExtender();
void myshiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val);
void taskFQ512(uint16_t cmd);
uint16_t CRC_16(byte *data, uint16_t len, CRC16_parameter_t *CRC16_parameter);

void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}
#endif