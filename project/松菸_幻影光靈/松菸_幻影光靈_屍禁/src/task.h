#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <MissGame.h>
#include <Adafruit_PN532.h>
enum Status_e
{
_Reset,
_1,
_2,
_3,
_4,
_Finish,
_DEBUG,
};


const uint8_t pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26;
void task(void *pvParam);
void taskMCP230x7(void *pvParam);

void taskPCM5102(void *pvParam);
#endif