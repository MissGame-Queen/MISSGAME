#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <Audio.h>
extern QueueHandle_t queueBasketball;
extern const uint8_t pinDS, pinSH, pinST;
void Basketball(void *pvParam);
void FlyingShip(void *pvParam);
void set74HC595(String newTime);
void doStyle1(int value);
#endif