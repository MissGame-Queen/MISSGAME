//通用底層方法
#ifndef TEMPLATE_H
#define TEMPLATE_H

// #include <Interface_.h>
// #include <WiFiClient.h>
// #include <U8g2lib.h>
// #include <qrcode.h>
// #include <time.h>
// #include <AsyncWiFiManager.h>
// #include <esp_sleep.h>

/*************內部************/
#include "Init.h"
#include "Macros.h"
#include "taskTemplate.h"
#include "serverOn.h"
#include "myFunction.h"

// Json指令表相關操作實作
class JsonPTC
{
  /****私有成員****/
private:
  FS *_csFS;
  JsonDocument *_csDoc_CMD;
  JsonObject _csObj_CMD;
  String (*_classCMD)(JsonDocument *doc);
  bool _doCleanPtr;
  /****公有成員****/
public:
  static constexpr const char *csFileName_CMD = "/config.json";
  static constexpr const char *csFileName_SYSTEM = "/_SYSYTEM.json";

  JsonPTC();
  JsonPTC(JsonDocument *doc_CMD);
  ~JsonPTC();
  int8_t Begin(FS *fs = &SPIFFS, const char *fileName = csFileName_CMD, bool doFilter = true);
  void setCMD(String (*function)(JsonDocument *doc));
  JsonObject *getJsonObject();
  JsonDocument *getJsonDocument();
  int8_t RstConfig();
  int8_t SaveConfig();
  void Write(JsonDocument *obj);
  void Read(JsonDocument *obj);
  void Save(JsonDocument *obj);
  /*
  enum ERROR_CODE
  {
    Config_ERROR = -1,
    CommandTable_ERROR = -2,
    DeserializeJson_CommandTable_ERROR = -3,
    DeserializeJson_Config_ERROR = -4,
    JsonSize_ERROR = -5,
    Pointer_ERROR = -6
  };
  */
};

//***********************樣板資料*********************//

//?========================列舉========================
typedef enum configType_e
{
  _CONFIG_ERROR = -1,
  _CONFIG_SPIFFS,
  _CONFIG_SD,
} configType_e;
enum Template_Register
{
  _VER,            // 版本號碼
  _PIN_SD_CS,      // SD卡腳位
  _PIN_SET,        // 設定腳位
  _MODE_SET,       // 是否設定模式
  _FILE_CONFIG_FS, // 參數來源
  _FILE_DATA_FS,   // 資料儲存地
  _STATUSLED,      // 狀態燈參數
  _FIRMWAREURL,
};
 enum Function_e{
  FUNCTION_CODE_HAVE_IP=1,
  FUNCTION_CODE_HAVE_INTERNET=2
};
//?========================變數========================

// extern Interface DebugInterface;

extern uint8_t Template_WiFi_Reconnect_Time; // 重新連線記數次數
extern uint8_t Template_WiFi_Connect_Set;    // 重新連線設定次數
extern JsonPTC *Template_JsonPTC;            // JSON配置物件
extern SocketIOclient socketIO_Client;
extern WiFiMulti wifiMulti;



//?========================函數========================
struct SpiRamAllocator : ArduinoJson::Allocator
{
  void *allocate(size_t size) override
  {
    return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
  }

  void deallocate(void *pointer) override
  {
    heap_caps_free(pointer);
  }

  void *reallocate(void *ptr, size_t new_size) override
  {
    return heap_caps_realloc(ptr, new_size, MALLOC_CAP_SPIRAM);
  }
};
int8_t Init();
int8_t CheckFile(configType_e config);
void listFile(configType_e config);
bool readDate();
bool isConnect(uint8_t level = FUNCTION_CODE_HAVE_INTERNET);
bool waitConnect(uint16_t maxTime = 10, uint8_t level = FUNCTION_CODE_HAVE_INTERNET);
void WiFiReconnect();
FS *getFS(String usetype = _E2S(_FILE_CONFIG_FS));
void WiFiInit(void *pvParam);
bool WiFi_Connet(JsonObject *obj);
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info);
void deleteOldFile(size_t sdCardCapacityLimit, String format, String path = "/");
void MQTT_Callback(char *topic, byte *payload, unsigned int length);
void MQTT_Subscribe(PubSubClient *MQTTClient);
void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length);
//?=====================需另外定義的函數========================
int8_t myCmdTable(const char *address, const char *data);
// JSON格式的執行表
String myCmdTable_Json(JsonDocument *doc);
// MQTT的訂閱項目
void MQTT_Subscribe(PubSubClient *MQTTClient);

#endif