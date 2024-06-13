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
        case 1 ... 4:
        {
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID= %d , 出幣機_%d執行%d次\n", id, id, args[1]["value"].as<uint16_t>());
          uint16_t value = args[1]["value"].as<uint16_t>();
          if (value > 0) // 如果是0會變成迴圈
                         // CoinDispenser(value);
            xQueueSend(queueBallTime, &value, portMAX_DELAY);
        }
        break;
        case 5 ... 6:
        {
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID= %d , 出球機執行%d次\n", id, args[1]["value"].as<uint16_t>());
          uint16_t value = args[1]["value"].as<uint16_t>();
          if (value > 0) // 如果是0會變成迴圈
            xQueueSend(queueBallTime, &value, portMAX_DELAY);

          break;
        }
        case 7 ... 9:
        {
          if (args[1].containsKey("value"))
          {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID= %d , 計時器倒數%s\n", id, args[1]["value"].as<const char *>());
            // xQueueSend(queueTimer, &seconds, 0);
            Timer_newSecond = args[1]["value"].as<String>();
          }
          if (args[1].containsKey("status"))
          {
            Timer_status = args[1]["status"].as<uint8_t>();
          }
          break;
        }
        case 10 ... 19:
          break;
        case 20 ... 29:
          if (args[1].containsKey("value"))
          {
            /*
                        uint16_t value = args[1]["value"].as<uint16_t>();
                        if (args[1].containsKey("level"))
                        {
                          uint8_t type = value >= 10000 ? 1 : 0;
                          if (args[1]["level"].as<uint16_t>() > SoundPlayerLevel[type])
                          {
                            SoundPlayerLevel[type] = args[1]["level"].as<uint16_t>();
                            if (type)
                              value -= 10000;
                            SoundPlayer(value);
                          }
                        }
                        else if (SoundPlayerLevel[value >= 10000 ? 1 : 0] == 0)
                        {
                          SoundPlayer(value);
                        }
                      */
            uint16_t soundName = args[1]["value"].as<uint16_t>();
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放:%d\n", soundName);
            xQueueSend(queueDFPlayer, &soundName, portMAX_DELAY);
          }
          if (args[1].containsKey("name"))
          {
            if (args[1].containsKey("level"))
            {
              uint8_t level = args[1]["level"].as<uint8_t>();
              _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "目前等級為:%d\n", SoundPlayerLevel[0]);
              if (level >= SoundPlayerLevel[0])
              {
                SoundPlayerLevel[0] = level;
                String soundName = args[1]["name"].as<String>();
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "覆蓋播放:%s\n", soundName.c_str());
                xQueueSend(queuePCM5102, &soundName, portMAX_DELAY);
              }
            }
            else if (SoundPlayerLevel[0] == 0)
            {
              String soundName = args[1]["name"].as<String>();
              _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放:%s,目前內存剩餘:%d\n", soundName.c_str(), ESP.getFreeHeap());
              xQueueSend(queuePCM5102, &soundName, portMAX_DELAY);
            }
          }
          if (args[1].containsKey("volume"))
          {
            uint8_t volume = args[1]["volume"].as<uint8_t>();
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "調整音量:%s\n", volume);
            audioPCM5102->setVolume(volume);
          }

          break;
        case 90 ... 99:
        {
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID= %d , DMX播放%d號\n", id, args[1]["value"].as<uint16_t>());
          taskFQ512(args[1]["value"].as<uint16_t>());
          break;
        }
        case 30 ... 89:
          if (args[1].containsKey("value"))
          {
           uint8_t level = args[1]["value"].as<uint8_t>();
              xQueueSend(queueWeaponLight, &level, portMAX_DELAY);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID: %d,Level=%d\n", id, args[1]["value"].as<uint8_t>());
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
  const uint8_t pinOut[]{25, 26, 27, 33};
  for (size_t i = 0; i < sizeof(pinOut); i++)
  {
    ledcSetup(i, 300000, 8);
    ledcAttachPin(pinOut[i], i);
    ledcWrite(i, 0);
  }
  Serial.begin(115200);
  Wire.begin();
  ConfigInit();

  uint16_t id = _E2JS(_MODULE_ID).as<uint16_t>();
  RTOS();
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ID=%d\n", id);

  switch (id)
  {
  case 1 ... 4:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機模式~");
    CoinDispenser(0);
    break;
  case 5 ... 6:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出球機模式~");
    xTaskCreatePinnedToCore(taskBallDispenser,
                            "taskBallDispenser",
                            40960,
                            NULL,
                            1,
                            NULL,
                            0);
    break;
  case 7 ... 9:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "計時器模式~");
    xTaskCreatePinnedToCore(taskTimer,
                            "taskTimer",
                            10240,
                            NULL,
                            1,
                            NULL,
                            0);
    break;
  case 10 ... 19:
    break;
  case 20 ... 29:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "MP3撥放模式~");
    // SoundPlayer(0);
    taskPCM5102((void *)audioPCM5102);
    break;
  case 90 ... 99:
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "FQ512控制模式~");
    Serial2.begin(115200);
    /*
    while (1)
    {
      while (Serial.available())
      {
        Serial2.write(Serial.read());
      }
      while (Serial2.available())
      {
        Serial.write(Serial2.read());
      }
    }
    */
    // const uint8_t pinIn[] = {13, 12, 14, 247, 26, 25, 33, 32, 35, 34, 39, 36};
    // const uint8_t pinIn[] = {13, 21, 19, 18, 5};
    const uint8_t pinIn[] = {36, 39, 34, 35, 32};
    for (uint8_t i = 0; i < sizeof(pinIn); i++)
      pinMode(pinIn[i], INPUT_PULLUP);
    while (1)
    {
      for (uint8_t i = 0; i < sizeof(pinIn); i++)
        if (analogRead(pinIn[i]) < 512)
        {
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "按鈕觸發[%d]\n", i);
          taskFQ512(i);
        }
      _DELAY_MS(100);
    }
  }
  break;
  case 100:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "訊號延長器模式~");
    taskSignalExtender();//投幣機
    break;
      case 101:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "訊號延長器模式~");
    taskLINE_POST();
    break;
  case 30 ... 89:
  {
    JsonDocument *doc = new JsonDocument;
    if (id >= 50 && id <= 59)
      (*doc)["Length"] = 35;
    else
      (*doc)["Length"] = 11;
    (*doc)["Pin"] = 15;
    // 是否開機進入測試模式
    if ((*Template_JsonPTC->getJsonObject()).containsKey("_TESTMODE"))
      _E2JS(_TESTMODE).as<bool>() ? (*doc)["Level"] = 99 : (*doc)["Level"] = 0;
    else
      (*doc)["Level"] = 99;
    // 是否套用自定義亮度
    if (!(*Template_JsonPTC->getJsonObject()).containsKey("_LIGHT_0"))
      (*doc)["_LIGHT_0"] = 0;
    else
      (*doc)["_LIGHT_0"] = _E2JS(_LIGHT_0).as<uint16_t>();
    if (!(*Template_JsonPTC->getJsonObject()).containsKey("_LIGHT_1"))
      (*doc)["_LIGHT_1"] = 0;
    else
      (*doc)["_LIGHT_1"] = _E2JS(_LIGHT_1).as<uint16_t>();
    if (!(*Template_JsonPTC->getJsonObject()).containsKey("_LIGHT_2"))
      (*doc)["_LIGHT_2"] = 0;
    else
      (*doc)["_LIGHT_2"] = _E2JS(_LIGHT_2).as<uint16_t>();
    if (!(*Template_JsonPTC->getJsonObject()).containsKey("_LIGHT_3"))
      (*doc)["_LIGHT_3"] = 0;
    else
      (*doc)["_LIGHT_3"] = _E2JS(_LIGHT_3).as<uint16_t>();

    (*doc)["DelayTime"] = 50;
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "武器模式~");
    xTaskCreatePinnedToCore(taskWeaponLight,
                            "taskWeaponLight",
                            40960,
                            (void *)doc,
                            10,
                            NULL,
                            0);
  }
  break;
  default:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "id尚未定義! : %d\n", id);
    break;
  }
}
void loop()
{
}
