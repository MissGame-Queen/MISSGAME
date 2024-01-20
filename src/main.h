#ifndef MAIN_H
#define MAIN_H
#define _E2JS(x) (*Template_JsonPTC->getJsonObject())[#x]["Value"]
#include <Template.h>
#include "task.h"
#include "FM_505.h"
#include <Audio.h>
#include <WiFiMulti.h>
#include <Adafruit_NeoPixel.h>

#include <Adafruit_SHT4x.h>
#include <U8g2lib.h>
#include <PubSubClient.h>
#include <dmx.h>
enum StrRegister
{
  STA_SSID,      // 想連接的WIFI名稱{TTPunch}
  STA_PASSWORD,  // 想連接的WIFI密碼{033865819}
  AP_SSID,       // 連結到設備用的WIFI名稱{Ragic_QRCODE}
  AP_PASSWORD,   // 連結到設備用的WIFI密碼{00000000}
  WIFI_HOSTNAME, // 設備名稱{Ragic_QRCODE}
  WIFI_MDNSNAME, // 網域的主機名稱(後面加上.local即可透過瀏覽器瀏覽){Ragic_QRCODE}
};
enum ModbusRegister
{
  _INFORMATION,       // 版本資料{21001}
  _RESET,             // 回原廠設定
  _SAVE,              // 儲存設定
  _MODBUS_EN,         // Modbus啟用{1}
  _MODBUS_ID,         // 設備ID{1}
  _MODBUS_BAUD,       // 鮑率{1152}
  _MODBUS_PROTOCOL,   // 通訊協議編號{0}
  _MODULE_TYPE,       // 模組類型{1}
  _OLED_EN,           // OLED啟用{1}
  _OLED_TYPE,         // OLED型號設定 [方向3][型號3]… [方向0][型號0]{43520}
  _OLED_REFRESH,      // OLED刷新
  _OLED_PAGE,         // OLED顯示畫面{0}
  _WIFI_CONNECT_TIME, // 嘗試連線次數{20}
  _WIFI_TYPE,         // 連線模式(0:OFF,1:STA,2:AP,3:APSTA){3}
  _RFID_EN,           // 是否啟用RFID{0}
  _TH_TEMPERATURE,    // 溫度
  _TH_HUMIDITY,       // 濕度
  _DATANUM
};
const int8_t pinSet = 33,
pinLED = 14,
pinTone=25,
             pinmode = 39,
             pinAnalog = 36,
             pinTrig = 4,
             PinEcho = 35,
             pinRelay[2]{0, 2},
             pinSD_CS = 5,
             pinOLED_CS = 14,
             pinOLED_RST = 32,
             pinOLED_DC = 27,
             pinDAC_R = 26,
             pinDAC_L = 25,
             pinAudio_BCK = 15,
             pinAudio_DO = 13,
             pinAudio_WS = 12;
// Audio audio(true, I2S_DAC_CHANNEL_BOTH_EN);
extern Audio audio;
const uint8_t numLEDMax = 8;
extern Adafruit_NeoPixel strip;
extern uint16_t ledON[numLEDMax], ledOFF[numLEDMax];
extern uint8_t ledR[numLEDMax], ledG[numLEDMax], ledB[numLEDMax], ledType[numLEDMax];
extern size_t ledTime[numLEDMax];
extern FM505 myFM505;
extern WiFiClient espClient;
extern PubSubClient clientMQTT;
void reconnect_MQTT();
#endif