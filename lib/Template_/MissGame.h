#ifndef MISSGAME_H
#define MISSGAME_H
#include <Arduino.h>
#include "Macros.h"
#include "Init.h"
#include <ArduinoJson.h>
#include <Adafruit_MCP23X17.h>
#include <Audio.h>
#include <DFRobotDFPlayerMini.h>
// 若是需要Audio函式庫的輸出程式就取消BYPASS
// #define DEBUG_PCM5102 1

extern QueueHandle_t queuePCM5102;
extern QueueHandle_t queueMCP230x7_Input;
extern QueueHandle_t queueMCP230x7_Output;




void taskPCM5102(void *pvParam);
void taskMCP230x7(void *pvParam);
void taskExamples(void *pvParam);
#endif