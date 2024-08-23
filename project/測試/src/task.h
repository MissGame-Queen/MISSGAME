#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <Adafruit_MCP23X17.h>
#include <Audio.h>
#include <DFRobotDFPlayerMini.h>
#include <MPU9250.h>
#include <SPI.h>
#include "epd1in54_V2.h"
#include "imagedata.h"
#include "epdpaint.h"
#include <stdio.h>
#include <Adafruit_NeoPixel.h>
extern QueueHandle_t queuePCM5102;
extern SemaphoreHandle_t xMutex;
extern MPU9250 mpu;
void taskPCM5102(void *pvParam);
void taskE_paper();
#endif