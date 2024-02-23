#ifndef TASK_H
#define TASK_H
#include "main.h"
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>
void task_Audio(void *pvParam);
void task_LED(void *pvParam);
void task_MQTT(void *pvParam);
void task_WebSocket(void *pvParam);
void callback_MQTT(char *topic, byte *payload, unsigned int length);
#endif
