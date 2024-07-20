#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <Audio.h>
#include <SPIFFS.h>   //內建的空間儲存庫
extern uint16_t SoundPlayerLevel[2];
extern String SoundPlayerName[2];
extern Audio *audioPCM5102;
extern QueueHandle_t queueJson;
extern QueueHandle_t queueTimer;
extern QueueHandle_t queuePCM5102;
extern QueueHandle_t queueDFPlayer;
extern QueueHandle_t queueBallTime;
extern QueueHandle_t queueWeaponLight;
extern QueueHandle_t queueLINE_POST;
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
//void taskTimer(void *pvParam);
void taskPCM5102(void *pvParam);
void set74HC595(String newTime);
void printDetail(uint8_t type, int value);
void taskWeaponLight(void *pvParam);
void taskLINE_POST();
void taskSignalExtender();
void taskSpider(void *pvParam);
void taskFQ512(uint16_t cmd);
uint16_t CRC_16(byte *data, uint16_t len, CRC16_parameter_t *CRC16_parameter);


#endif