#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <Adafruit_MCP23X17.h>
#include <Audio.h>
#include <DFRobotDFPlayerMini.h>
extern QueueHandle_t queueBasketball;
extern QueueHandle_t queuePCM5102;

extern const uint8_t pinDS, pinSH, pinST;
void Basketball(void *pvParam);
void FlyingShip(void *pvParam);
void set74HC595(String newTime);
void taskPCM5102(void *pvParam);
#endif