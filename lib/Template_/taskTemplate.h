#ifndef TEMPLATE_TASK_H
#define TEMPLATE_TASK_H
#include "Template.h"
//?========================ROTS========================
extern QueueHandle_t queueStatusLED;

void taskReadDate(void *pvParam);
void taskUpdateFirmware(void *pvParam);
void taskMQTT(void *pvParam);
void taskSocketIO(void *pvParam);
void taskStatusLED(void *pvParam);
#endif
