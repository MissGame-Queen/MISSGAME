#include "main.h"
//?=============樣板=============================
/**
 * @brief 初始化並讀取設定檔
 *
 */
void ConfigInit()
{
  JsonDocument *ptrDoc = new JsonDocument;
  Template_System_Obj = ptrDoc->to<JsonObject>();
  //?=================================硬體參數============================================
  _T_E2JS(_VER) = 20240724;
  _T_E2JS(_PIN_SD_CS) = 12;
  _T_E2JS(_PIN_SET) = 14;
  _T_E2JS(_FILE_CONFIG_FS) = "AUTO";
  _T_E2JS(_FILE_DATA_FS) = "AUTO";
  _T_E2JS(_FIRMWAREURL) = "";

  _T_E2JS(_STATUSLED)
  ["Pin"] = 14;
  _T_E2JS(_STATUSLED)
  ["Type"] = _Monochrome + _Breathe;
  _T_E2JS(_STATUSLED)
  ["Color_R"] = 255;
  _T_E2JS(_STATUSLED)
  ["Color_G"] = 255;
  _T_E2JS(_STATUSLED)
  ["Color_B"] = 255;
  _T_E2JS(_STATUSLED)
  ["DelayTime"] = 50;
  _T_E2JS(_STATUSLED)
  ["Brightness"] = 255;
  _T_E2JS(_STATUSLED)
  ["Brightness_Cycel_ON"] = 1000;
  _T_E2JS(_STATUSLED)
  ["Brightness_Cycel_OFF"] = 1000;
  _T_E2JS(Color_Cycle)
  ["Brightness_Cycel_OFF"] = 1000;

  uint8_t pin = _T_E2JS(_PIN_SET).as<uint8_t>();
  pinMode(pin, INPUT_PULLUP);
  (!digitalRead(pin)) ? _T_E2JS(_MODE_SET) = 1 : _T_E2JS(_MODE_SET) = 0;

  Init();
  xTaskCreatePinnedToCore(taskStatusLED,
                          "taskStatusLED",
                          4096,
                          (void *)&Template_System_Obj,
                          1,
                          NULL,
                          0);

  if (_T_E2JS(_MODE_SET).as<bool>())
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "配置模式!");
    _T_E2JS(_STATUSLED)
    ["Type"] = _Monochrome + _Flashing;
    _T_E2JS(_STATUSLED)
    ["Brightness"] = 50;
  }
  else
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "一般模式!");
    _T_E2JS(_STATUSLED)
    ["Color_R"] = 0;
    _T_E2JS(_STATUSLED)
    ["Color_B"] = 0;
  }
  JsonObject obj = _T_E2JS(_STATUSLED);
  xQueueSend(queueStatusLED, &obj, portMAX_DELAY);

  Template_JsonPTC = new JsonPTC();

  if (Template_JsonPTC->Begin(getFS()) < 0)
  {
    bool isOK = true;
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "資料庫初始化錯誤!重置設定檔中...");
    if (Template_JsonPTC->RstConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "設定檔重置失敗!");
      isOK = false;
    }
    else if (isOK)
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "設定檔重置成功!生成設定檔中...");

    if (Template_JsonPTC->SaveConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "生成設定檔失敗!");
      isOK = false;
    }
    else if (isOK)
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "生成設定檔成功!資料庫初始化中...");
    if (Template_JsonPTC->Begin(getFS()) < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "設生成設定檔失敗!");
      isOK = false;
    }
    if (!isOK)
    {
      _T_E2JS(_STATUSLED)
      ["Color_R"] = 255;
      _T_E2JS(_STATUSLED)
      ["Color_G"] = 0;
      _T_E2JS(_STATUSLED)
      ["Color_B"] = 0;
      JsonObject obj = _T_E2JS(_STATUSLED);
      xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
      while (true)
      {
        _DELAY_MS(10000);
      }
    }
  }
  //?=================================覆寫樣板參數============================================
  _T_E2JS(_FIRMWAREURL).set(_E2JS(FIRMWAREURL).as<String>());  
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "資料庫正常運作!佔用RAM大小:[ %d ]\n", Template_JsonPTC->getJsonObject()->memoryUsage());
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
  // 自動更新固件任務
  if (_E2JS(FIRMWAREURL).as<String>() != "")
  {
    JsonDocument pvParam;
    pvParam["Path"].set("D:/Project/Code/MissGame");
    xTaskCreatePinnedToCore(taskUpdateFirmware,
                            "taskUpdateFirmware",
                            10240,
                            (void *)&(pvParam),
                            1,
                            NULL,
                            0);
  }
}
void Module_CMD(JsonDocument *doc)
{
  JsonArray args = doc->as<JsonArray>();
  uint16_t id = 0;
  for (JsonVariant item : args[1]["ids"].as<JsonArray>())
  {
    id = item.as<uint16_t>();
    // FIXME 補上自製模組的運作方法
    /**
     @brief id表
     1~4 出幣機
     5~6 出球機
     7~9 計時器
     10~19 bricks
     20~29 音效播放模組
     30~39 杖
     40~49 矛
     50~59 槌
     60~69 鍊
     70~79 劍
     80~89 斧
     90~99 DMX512模組
     100 訊號延長器
     */
    if (id == _E2JS(_MODULE_ID).as<uint16_t>())
    {
      switch (id)
      {
      default:
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "無定義此ID: %d\n", id);
        break;
      }
    }
  }
}
void Module_Setup(uint16_t id)
{
  switch (id)
  {

  break;
  default:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "id尚未定義! : %d\n", id);
    break;
  }
}

/**
 * @brief 指令表
//!不知為何.as<const char*>()不管用，需使用.as<String>().c_str();
//!用std::to_string會怪怪的??
//! Json賦值時需轉換成String
[tpic,data]
 */
String myCmdTable_Json(JsonDocument *doc)
{
  String rtStatus = "{\"State\":\"OK\"}";
  String errStatus = "{\"State\":\"ERROR\"}";
  /*
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
*/
  JsonArray args = doc->as<JsonArray>();
  String eventName = args[0];

  // serializeJsonPretty(*doc, Serial);

  if (eventName == "MissGame" || eventName == _E2JS(_MODULE_ID).as<String>())
  {

    uint16_t id = 0;
    // 如果封包不包含ids自動補進id
    if (!args[1].containsKey("ids") && args[1].containsKey("id"))
    {
      args[1]["ids"].add(args[1]["id"].as<uint16_t>());
    }
    // 或者event本身就是id則補進id
    else if (eventName == _E2JS(_MODULE_ID).as<String>())
    {
      args[1]["ids"].add(_E2JS(_MODULE_ID).as<uint16_t>());
    }

    Module_CMD(doc);
  }
  else if (eventName == "Alive")
    ;
  else
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "無定義此事件: %s\n", eventName.c_str());
  return rtStatus;
}

void setup()
{
  Serial.begin(115200);
  Module_Setup(0);
  Wire.begin();
  ConfigInit();
  RTOS();
  uint16_t id = _E2JS(_MODULE_ID).as<uint16_t>();
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID=%d\n", id);
  Module_Setup(id);
}
void loop()
{
}
