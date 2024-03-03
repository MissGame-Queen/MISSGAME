#include "main.h"

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
  Init(24001);
  Template_System_Obj["StatusLED_Pin"] = 14;
  Template_System_Obj["StatusLED_Type"] = 1 | 4;
  Template_System_Obj["StatusLED_Color_R"] = 255;
  Template_System_Obj["StatusLED_Color_G"] = 0;
  Template_System_Obj["StatusLED_Color_B"] = 0;
  Template_System_Obj["StatusLED_Brightness"] = 50;
  xTaskCreatePinnedToCore(taskStatusLED,
                          "taskStatusLED",
                          2048,
                          (void *)&Template_System_Obj,
                          1,
                          NULL,
                          0);
  while (1)
    _DELAY_MS(1000);
  Template_JsonPTC = new JsonPTC(Template_CMDSize, Template_CFGSize);

  if (Template_JsonPTC->Begin(getFS()) < 0)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "資料庫初始化錯誤!重置資料庫中...");
    if (Template_JsonPTC->RstConfig() < 0)
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "資料庫初始化失敗!");
    while (Template_JsonPTC->SaveConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "資料庫存檔失敗!");
      vTaskDelay(3000 / portTICK_RATE_MS);
      ESP.restart();
    }
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "重置資料庫成功!");
  }
  //?=================================覆寫樣板參數============================================
  _T_E2JS(_FIRMWAREURL).set(_E2JS(FIRMWAREURL).as<String>());
  _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "資料庫正常運作....");
}
/**
 * @brief MQTT重連函數
 *
 * @return int
 */
/*
int reconnect_MQTT(){
  ;
}
*/
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
  { // FIXME
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

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  ConfigInit();
  /**********************ROTS************************/
  xTaskCreatePinnedToCore(WiFiInit,
                          "WiFiInit",
                          4096,
                          (void *)&(*Template_JsonPTC->getJsonObject()),
                          1,
                          NULL,
                          0);
}

void loop()
{
}
