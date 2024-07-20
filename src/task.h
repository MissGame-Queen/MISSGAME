#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <Adafruit_MCP23X17.h>
#include <Audio.h>
#include <DFRobotDFPlayerMini.h>
extern QueueHandle_t queueJson;
extern QueueHandle_t queuePCM5102;
extern QueueHandle_t queueMCP230x7_Input;
extern QueueHandle_t queueMCP230x7_Output;

extern SemaphoreHandle_t xMutex;
enum stripType_Game_e
{
    Standby = 1, // 待機狀態
    Start = 2,   // 遊戲開始狀態
    Correct = 4, // 步數正確
    Mistake = 8, // 步數錯誤
    Save = 16,   // 進度保存
    Finish = 32, // 完成
};
const uint8_t pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26;
void task(void *pvParam);
void taskMCP230x7(void *pvParam);

void taskPCM5102(void *pvParam);
#endif