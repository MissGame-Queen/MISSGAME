#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <Adafruit_MCP23X17.h>
#include <Audio.h>
#include <DFRobotDFPlayerMini.h>
extern QueueHandle_t queueBasketball;
extern QueueHandle_t queuePCM5102;
extern QueueHandle_t queuedoStyle4;
extern SemaphoreHandle_t xMutex;
const uint8_t pinDS = 23, pinSH = 18, pinST = 4;
const uint8_t pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26;
enum stripType_FlyingShip_e
{
    Standby = 1, // 待機狀態
    Start = 2,   // 遊戲開始狀態
    Correct = 4, // 步數正確
    Mistake = 8, // 步數錯誤
    Save = 16,   // 進度保存
    Finish = 32, // 完成
};
const uint8_t checkValue[]{7, 9, 17};
extern uint8_t saveIndex;
extern uint8_t index_FlyingShip;
struct doStyle4_s
{
    Adafruit_NeoPixel *strip;
    stripType_FlyingShip_e *stripType;
    uint32_t timerStrip;
    uint32_t timerStripType;
};

void Basketball(void *pvParam);
void FlyingShip(void *pvParam);
void set74HC595(String newTime);
void doStyle1(int value);
void taskPCM5102(void *pvParam);
void taskWS2812(void *pvParam);
void doStyle2(Adafruit_NeoPixel *strip, int value, uint32_t val);
void doStyle3(Adafruit_NeoPixel *strip, uint16_t valMax, uint16_t value);
void doStyle4(Adafruit_NeoPixel *strip, stripType_FlyingShip_e *stripType, uint32_t timerStrip, uint32_t timerStripType);
#endif