#ifndef SERVERON_H
#define SERVERON_H
//#include "Template.h"
#include "Init.h"
#include "Macros.h"
void ServerInit();
void firmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void spiffsUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
void configUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final);
#endif