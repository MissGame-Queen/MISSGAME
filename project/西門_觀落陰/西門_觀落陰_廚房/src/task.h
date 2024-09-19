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
enum Status_e
{
    _Reset,           // RE
    _DismissalHand,   // 放斷手
    _ButtonRaised,    // 按鈕升起
    _PuzzleWait,      // 等待謎題完成
    _PuzzleCompleted, // 謎題完成
    _Finish,
    _DEBUG,
};

const uint8_t pinOutput[] = {12, 13, 14, 15};
const uint8_t pinInput[] = {36, 39, 34, 35};
const uint8_t pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26;
void task(void *pvParam);
void taskMCP230x7(void *pvParam);

void taskPCM5102(void *pvParam);
#endif