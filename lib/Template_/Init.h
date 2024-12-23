// 若內部有共用的變數放這裡，統一呼叫維持檔案獨立性
#ifndef INIT_H
#define INIT_H
/*************固定************/
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
/*************常使用************/
#include <ESPmDNS.h>           //mDNS功能
#include <ArduinoJson.h>       //JSON庫
#include <SPIFFS.h>            //內建的空間儲存庫
#include <SPIFFSEditor.h>      //線上SPIFFS編輯庫
#include <Adafruit_NeoPixel.h> //WS2812庫
#include <ArduinoOTA.h>        //線上燒寫韌體用
/*************待測試************/
#include <PubSubClient.h>     //MQTT用
#include <SocketIOclient.h>   //SocketIO用
#include <WebSocketsClient.h> //WebSockets用
#include <esp_task_wdt.h>     //控制看門夠用
extern SemaphoreHandle_t rmtMutex;
extern SemaphoreHandle_t SPIMutex;
extern SemaphoreHandle_t I2CMutex;
extern const char *Template_HTTP_ContentType_Text;
extern const char *Template_HTTP_ContentType_Json;
extern struct tm _T_localtm;
extern AsyncWebServer Template_server; // 網站伺服器物件
extern JsonObject Template_System_Obj; // 樣板系統參數句柄
#endif