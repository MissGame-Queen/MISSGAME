#ifndef TASK_H
#define TASK_H
#include "main.h"

void task_Audio(void *pvParam);
void task_LED(void *pvParam);
void task_MQTT(void *pvParam);
void callback_MQTT(char *topic, byte *payload, unsigned int length);
#endif
