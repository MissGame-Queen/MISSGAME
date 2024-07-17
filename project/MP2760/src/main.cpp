


#include "main.h"
#include "Interface.h"
//?=============樣板=============================
/**
 * @brief 初始化並讀取設定檔
 *
 * @param mode {
 * 0:正常初始化
 * 1:格式化
 * 2:僅移除設定保留指令集
 * }
 */

void ConfigInit(int mode = 0)
{
  switch (mode)
  {
  case 1:
    SPIFFS.begin(true);
    SPIFFS.format();
    break;
  case 2:
    SPIFFS.begin(true);
    SPIFFS.remove("/config.json");
    break;
  }
  // 先判斷是否為配置模式，並修改樣板參數

  Init(24001);
  uint8_t pin = _T_E2JS(_PIN_SET).as<uint8_t>();
  pinMode(pin, INPUT_PULLUP);
  (!digitalRead(pin)) ? _T_E2JS(_TYPE_SET) = 1 : _T_E2JS(_TYPE_SET) = 0;

  xTaskCreatePinnedToCore(taskStatusLED,
                          "taskStatusLED",
                          2048,
                          (void *)&Template_System_Obj,
                          1,
                          NULL,
                          0);

  if (_T_E2JS(_TYPE_SET).as<bool>())
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "配置模式!");
    _T_E2JS(_StatusLED)
    ["Type"] = _Monochrome + _Brightness;
    _T_E2JS(_StatusLED)
    ["Color_B"] = 0;
    _T_E2JS(_StatusLED)
    ["Brightness"] = 50;
  }
  else
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "一般模式!");
  }
  Template_JsonPTC = new JsonPTC();

  if (Template_JsonPTC->Begin(getFS()) < 0)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "資料庫初始化錯誤!重置資料庫中...");
    if (Template_JsonPTC->RstConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "資料庫初始化失敗!");
      _T_E2JS(_StatusLED)
      ["Type"] = _Monochrome + _Brightness;
      _T_E2JS(_StatusLED)
      ["Color_R"] = 255;
      _T_E2JS(_StatusLED)
      ["Color_G"] = 0;
      _T_E2JS(_StatusLED)
      ["Color_B"] = 0;
      _T_E2JS(_StatusLED)
      ["Brightness"] = 50;
      while (1)
        ;
    }
    if (Template_JsonPTC->SaveConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "資料庫存檔失敗!");
      _T_E2JS(_StatusLED)
      ["Type"] = _Monochrome + _Brightness;
      _T_E2JS(_StatusLED)
      ["Color_R"] = 255;
      _T_E2JS(_StatusLED)
      ["Color_G"] = 0;
      _T_E2JS(_StatusLED)
      ["Color_B"] = 0;
      _T_E2JS(_StatusLED)
      ["Brightness"] = 50;
      while (1)
        ;
      // ESP.restart();
    }
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "重置資料庫成功!");
  }
  //?=================================覆寫樣板參數============================================
  _T_E2JS(_FIRMWAREURL).set(_E2JS(FIRMWAREURL).as<String>());
  _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "資料庫正常運作....");
}
/**
 * @brief 判斷參數執行任務
 *
 */
void RTOS()
{
  // WiFi連線任務
  xTaskCreatePinnedToCore(WiFiInit,
                          "WiFiInit",
                          4096,
                          (void *)Template_JsonPTC->getJsonObject(),
                          1,
                          NULL,
                          0);

  // 自動更新固件任務
  JsonDocument pvParam;
  pvParam["Path"].set("D:/Project/Code/MissGame");
  if (_E2JS(FIRMWAREURL).as<String>() != "")
    xTaskCreatePinnedToCore(taskUpdateFirmware,
                            "taskUpdateFirmware",
                            10240,
                            (void *)&(pvParam),
                            1,
                            NULL,
                            0);
  // MQTT任務
  if (_E2JS(MQTT_BROKER_URL).as<String>() != "")
    xTaskCreatePinnedToCore(taskMQTT,
                            "taskMQTT",
                            10240,
                            (void *)Template_JsonPTC->getJsonObject(),
                            1,
                            NULL,
                            0);
  // SocketIO任務
  if (_E2JS(SOCKETIO_URL).as<String>() != "")
    xTaskCreatePinnedToCore(taskSocketIO,
                            "taskSocketIO",
                            10240,
                            (void *)Template_JsonPTC->getJsonObject(),
                            1,
                            NULL,
                            0);
}
/**
 * @brief 指令表
//!不知為何.as<const char*>()不管用，需使用.as<String>().c_str();
//!用std::to_string會怪怪的??
//! Json賦值時需轉換成String
 */
String myCmdTable_Json(JsonObject obj)
{
  String rtStatus = "{\"State\":\"OK\"}";
  String errStatus = "{\"State\":\"ERROR\"}";
  for (JsonPair item : obj)
  { // FIXME 記得補上所需的功能
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "Function:%s\n", item.key().c_str());
    if (1 == 2)
      ;
    else
    {
      // 判斷是否包含在指令表，且透過Function參數執行的
      // 如果是就覆寫鍵值
      for (JsonPair item : obj)
      {
        const char *key = item.key().c_str();
        if ((*Template_JsonPTC->getJsonObject()).containsKey(key) && item.value().containsKey("Function"))
        {
          serializeJson(item.value()["Function"], Serial);
          Serial.println();
          serializeJson((*Template_JsonPTC->getJsonObject())[key], Serial);
          Serial.println();
          (*Template_JsonPTC->getJsonObject())[key]["Value"].set(item.value()["Function"]);
          serializeJson((*Template_JsonPTC->getJsonObject())[key], Serial);
          Serial.println();
        }
      }
    }
  }
  return rtStatus;
}
void myMQTTsubscribe(PubSubClient *MQTTClient) { ; }
void mysocketIOEvent(JsonDocument *doc)
{
  ;
}
void setup()
{
  const uint8_t pinOut[]{25, 26, 27, 33};
  for (size_t i = 0; i < sizeof(pinOut); i++)
  {
    ledcSetup(i, 100, 12);
    ledcAttachPin(pinOut[i], i);
    ledcWrite(i, 0);
  }
  Serial.begin(115200);
  Interface itfI2C;
  Interface itfSerial;
  JsonDocument docConfig;
  JsonObject objConfig = docConfig.to<JsonObject>();
  objConfig["Address"] = 0x21;
  objConfig["setClock"] = 400000;
  objConfig["baud"] = 115200;
  // itfI2C.Begin(&Wire, &objConfig);
  //  itfI2C.write(0x06);
  // delay(100);
  // Serial.println(itfI2C.read());
  Wire.begin();
  Wire.beginTransmission(0x21);
  Wire.write(7);
  Wire.write(16);
  Wire.endTransmission();
  Wire.requestFrom(0x21, 1);
  Wire.beginTransmission(0x21);
  Wire.write(7);
  Wire.endTransmission();
  Wire.requestFrom(0x21, 1);
  Serial.println(Wire.read());
  while (1)
    ;
  Wire.begin();
  // I2CScanner();
  //  taskTimer(1010);
  Wire.requestFrom(0x21, 1);
  ConfigInit();
  uint16_t id = _E2JS(_MODULE_ID).as<uint16_t>();
  RTOS();
}
void loop()
{
}
