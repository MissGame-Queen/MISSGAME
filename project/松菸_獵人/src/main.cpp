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
  // 先判斷是否為配置模式，並修改樣板參數

  Init();
  uint8_t pin = _T_E2JS(_PIN_SET).as<uint8_t>();
  pinMode(pin, INPUT_PULLUP);
  (!digitalRead(pin)) ? _T_E2JS(_TYPE_SET) = 1 : _T_E2JS(_TYPE_SET) = 0;
  /*
    xTaskCreatePinnedToCore(taskStatusLED,
                            "taskStatusLED",
                            2048,
                            (void *)&Template_System_Obj,
                            1,
                            NULL,
                            0);
  */
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
  /*
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
                              */
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
String myCmdTable_Json(JsonDocument *doc)
{
  String rtStatus = "{\"State\":\"OK\"}";
  String errStatus = "{\"State\":\"ERROR\"}";

  JsonArray args = doc->as<JsonArray>();
  String eventName = args[0];
  // serializeJsonPretty(args, Serial);
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

    for (JsonVariant item : args[1]["ids"].as<JsonArray>())
    {
      id = item.as<uint16_t>();
      // FIXME 補上自製模組的運作方法
      /**
       @brief id表
       1~10 籃球機
       11~20 飛行船
       */
      static String str = "";
      if (id == _E2JS(_MODULE_ID).as<uint16_t>())
      {
        switch (id)
        {
        case 1 ... 10:
        {
          str = "";
          serializeJson(args[1], str);
          xQueueSend(queueBasketball, &str, portMAX_DELAY);
        }
        break;
        case 11 ... 20:
        {
          str = "";
          serializeJson(args[1], str);
          xQueueSend(queuePCM5102, &str, portMAX_DELAY);
        }
        break;
        default:
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "無定義此ID: %d\n", id);
          break;
        }
      }
    }
  }
  else if (eventName == "Alive")
    ;
  else
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "無定義此事件: %s\n", eventName.c_str());
  return rtStatus;
}
void myMQTTsubscribe(PubSubClient *MQTTClient)
{
  MQTTClient->subscribe("MissGame");
  MQTTClient->subscribe(_E2JS(_MODULE_ID).as<String>().c_str());
}

void setup()
{
    Serial.begin(115200);
  uint8_t testBypass = 0;

  xTaskCreatePinnedToCore(FlyingShip,
                          "FlyingShip",
                          2048,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
  xTaskCreatePinnedToCore(taskWS2812,
                          "taskWS2812",
                          2048,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);

  taskPCM5102((void *)&testBypass);


  Wire.begin();
  ConfigInit();

  uint16_t id = _E2JS(_MODULE_ID).as<uint16_t>();
  RTOS();
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID=%d\n", id);
  uint8_t test = 0;
  /*
  _DELAY_MS(5000);
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "剩餘內存%d\n", esp_get_free_heap_size());
  if (Template_JsonPTC != nullptr)
  {
    delete Template_JsonPTC;
    // 設為 nullptr 避免懸空指標
    Template_JsonPTC = nullptr;
  }
*/
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "剩餘內存%d\n", esp_get_free_heap_size());

  switch (id)
  {
  case 1 ... 9:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "投籃機模式~");
    xTaskCreatePinnedToCore(Basketball,
                            "Basketball",
                            2048,
                            (void *)&test,
                            1,
                            NULL,
                            0);
    taskPCM5102((void *)&test);
    break;
  case 10 ... 19:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "飛行船模式~");

    xTaskCreatePinnedToCore(FlyingShip,
                            "FlyingShip",
                            2048,
                            (void *)&test,
                            1,
                            NULL,
                            0);
    xTaskCreatePinnedToCore(taskWS2812,
                            "taskWS2812",
                            2048,
                            (void *)&test,
                            1,
                            NULL,
                            0);

    taskPCM5102((void *)&test);
    break;
  default:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "id尚未定義! : %d\n", id);
    break;
  }
}
void loop()
{
}
