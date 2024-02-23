#include "main.h"

FM505 myFM505;
// MQTT 代理資訊
const int wssPort = 8084;
// MQTT 主題

WiFiClient espClient;
PubSubClient clientMQTT(espClient);

Audio audio;
Adafruit_NeoPixel strip(numLEDMax, pinLED, NEO_GRB + NEO_KHZ800);
uint16_t ledON[numLEDMax], ledOFF[numLEDMax];
uint8_t ledR[numLEDMax], ledG[numLEDMax], ledB[numLEDMax], ledType[numLEDMax];
size_t ledTime[numLEDMax];

WiFiMulti wifiMulti;
String ssid = "MissGAME_office";
String password = "missgame";
Adafruit_SHT4x sht40 = Adafruit_SHT4x();
sensors_event_t humidity, temp;
U8G2_SSD1306_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, pinOLED_CS, pinOLED_DC);
bool boolAudioTast = true;
void getSonicRanging()
{
  // 發送超聲波脈衝
  digitalWrite(pinTrig, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTrig, HIGH);
  delayMicroseconds(10);
  digitalWrite(pinTrig, LOW);

  // 測量超聲波回波的時間
  long duration = pulseIn(PinEcho, HIGH);

  // 將時間轉換為距離（使用聲速343m/s）
  float distance = duration * 0.0343 / 2;

  // 輸出距離到串口監視器
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // 等待一段時間再進行下一次測量
  delay(1000);
}
void setAudio()
{
  /**
   * @brief 某些版本不能用內置DAC輸出
   * https://github.com/espressif/arduino-esp32/issues/5938
   */
  _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "連線成功!");
  waitConnect();
  audio.setPinout(pinAudio_BCK, pinAudio_WS, pinAudio_DO);
  audio.setVolume(21); // 0...21
  String msg = "開機完成!準備撥放音樂!";
  //?依網路情況可能要很久才會播放
  audio.connecttospeech(msg.c_str(), "zh");

  /*
    bool useSD = false;
    if (useSD)
    {
      SD.begin(pinSD_CS);
      audio.connecttoFS(SD, "/test.mp3");
    }
    else
    {
      SPIFFS.begin(true);
      audio.connecttoFS(SPIFFS, "/test.mp3");
    };
  */
  while (true)
    audio.loop();
}
void setSDCard()
{
  SPI.begin();
  SD.begin(pinSD_CS);
  Serial.println("檔案清單:");

  File root = SD.open("/");
  if (!root)
  {
    Serial.println("無法開啟根目錄");
    return;
  }

  while (true)
  {
    File entry = root.openNextFile();
    if (!entry)
    {
      // 沒有更多檔案
      break;
    }

    Serial.print("  ");
    Serial.println(entry.name());

    entry.close();
  }

  root.close();
}
void getSHT40()
{
  static bool init = false;
  if (!init)
  {
    Wire.begin();
    Serial.println("Adafruit SHT4x test");
    if (!sht40.begin())
    {
      Serial.println("Couldn't find SHT4x");
      while (1)
        delay(1);
    }
    Serial.println("Found SHT4x sensor");
    Serial.print("Serial number 0x");
    Serial.println(sht40.readSerial(), HEX);

    // You can have 3 different precisions, higher precision takes longer
    sht40.setPrecision(SHT4X_HIGH_PRECISION);
    switch (sht40.getPrecision())
    {
    case SHT4X_HIGH_PRECISION:
      Serial.println("High precision");
      break;
    case SHT4X_MED_PRECISION:
      Serial.println("Med precision");
      break;
    case SHT4X_LOW_PRECISION:
      Serial.println("Low precision");
      break;
    }

    // You can have 6 different heater settings
    // higher heat and longer times uses more power
    // and reads will take longer too!
    sht40.setHeater(SHT4X_NO_HEATER);
    switch (sht40.getHeater())
    {
    case SHT4X_NO_HEATER:
      Serial.println("No heater");
      break;
    case SHT4X_HIGH_HEATER_1S:
      Serial.println("High heat for 1 second");
      break;
    case SHT4X_HIGH_HEATER_100MS:
      Serial.println("High heat for 0.1 second");
      break;
    case SHT4X_MED_HEATER_1S:
      Serial.println("Medium heat for 1 second");
      break;
    case SHT4X_MED_HEATER_100MS:
      Serial.println("Medium heat for 0.1 second");
      break;
    case SHT4X_LOW_HEATER_1S:
      Serial.println("Low heat for 1 second");
      break;
    case SHT4X_LOW_HEATER_100MS:
      Serial.println("Low heat for 0.1 second");
      break;
    }
    init = true;
  }
  else
  {
    sht40.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data
  }
}
void setOLED()
{
  static bool init = false;
  if (!init)
  {
    pinMode(pinOLED_CS, OUTPUT);
    pinMode(pinOLED_RST, OUTPUT);
    digitalWrite(pinOLED_RST, 0);
    vTaskDelay(10 / portTICK_PERIOD_MS);
    digitalWrite(pinOLED_RST, 1);
    u8g2.begin();
    u8g2.setFont(u8g2_font_pxplustandynewtv_8f);
    u8g2.setFontRefHeightExtendedText();
    u8g2.setDrawColor(1);
    u8g2.setFontPosTop();
    u8g2.setFontDirection(0);
    u8g2.enableUTF8Print();
    init = true;
  }
  else
  {
    u8g2.clearBuffer();
    u8g2.setCursor(10, 30);
    u8g2.print(millis());
    u8g2.sendBuffer();
  }
}
void setDAC()
{
  tone(pinDAC_R, 2093, 300);
  tone(pinDAC_L, 261, 300);
}

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
    /*
    {
      "LED":{
        "Start":0,
        "Length":8,
        "R":255,
        "G":255,
        "B":255
      }
    }
    */
    if (item.key() == "LED")
    {
      // 默認模式:輸入整條的RGB
      if ((!item.value().containsKey("Type") || item.value()["Type"] == "Default"))
      {
        if (item.value().containsKey("R") && item.value().containsKey("G") && item.value().containsKey("B") &&
            item.value().containsKey("Start") && item.value().containsKey("Length"))
        {
          uint8_t numR = item.value()["R"].as<uint8_t>();
          uint8_t numG = item.value()["G"].as<uint8_t>();
          uint8_t numB = item.value()["B"].as<uint8_t>();
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "R:%d,G:%d,B:%d\n", numR, numG, numB);

          for (uint8_t i = item.value()["Start"].as<uint8_t>(); i < item.value()["Start"].as<uint8_t>() + item.value()["Length"].as<uint8_t>(); i++)
          {
            ledType[i] = 0;
            ledR[i] = numR;
            ledG[i] = numG;
            ledB[i] = numB;
            ledTime[i] = millis();
          }
        }
        else
          rtStatus = errStatus;
      }
      /*
              {
      "LED":{
        "Start":0,
        "Length":13,
        "Type":"Breathe",
        "R":255,
        "G":255,
        "B":255,
        "ON":2000,
        "OFF":2000
      }
    }
      */
      else if (item.value().containsKey("Type") && item.value().containsKey("Start") && item.value().containsKey("Length"))
      {
        if ((item.value()["Type"] == "Flash" || item.value()["Type"] == "Breathe") &&
            item.value().containsKey("OFF") && item.value().containsKey("ON") &&
            ((item.value().containsKey("R") && item.value().containsKey("G") && item.value().containsKey("B")) ||
             item.value().containsKey("Random")))
        {
          for (uint8_t index = item.value()["Start"].as<uint8_t>(); index < item.value()["Start"].as<uint8_t>() + item.value()["Length"].as<uint8_t>(); index++)
          {
            ledType[index] = item.value()["Type"] == "Flash" ? 1 : 2;
            if (item.value().containsKey("Random"))
              ledType[index] += 0x80;
            else
            {
              ledR[index] = item.value()["R"].as<uint8_t>();
              ledG[index] = item.value()["G"].as<uint8_t>();
              ledB[index] = item.value()["B"].as<uint8_t>();
            }
            ledON[index] = item.value()["ON"].as<uint16_t>();
            ledOFF[index] = item.value()["OFF"].as<uint16_t>();
          }
        }
        else
          rtStatus = errStatus;
      }
    }
    else if (item.key() == "Battery")
    {

      float myFloat = analogRead(pinBattery_Vol) * 0.001611328125;
      char formattedString[20]; // 假設字串足夠大，可以容納格式化的結果
      // 使用 snprintf 格式化浮點數，將結果寫入字串中，只顯示小數點後兩位
      snprintf(formattedString, sizeof(formattedString), "%.2f", myFloat);
      rtStatus = formattedString;
    }
    else if (item.key() == "Audio" && item.value().containsKey("Type"))
    {
      if (item.value()["Type"].as<String>() == "Speech" && item.value().containsKey("Message"))
      {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放語音:%s\n", item.value()["Message"].as<const char *>());
        audio.connecttospeech(item.value()["Message"].as<const char *>(), "zh");
      }
    }
    else if (item.key() == "Relay" && item.value().is<JsonArray>())
    {
      JsonArray arr = item.value().as<JsonArray>();
      for (byte i = 0; i < arr.size() && i < 2; i++)
        digitalWrite(pinRelay[i], arr[i] > 0 ? true : false);
    }
    else if (item.key() == "Tone" && item.value().containsKey("Type"))
    {
      if (item.value()["Type"].as<bool>() == 1)
      {
        tone(pinTone, 1024, 300);
        tone(pinTone, 2048, 300);
        tone(pinTone, 4098, 300);
      }
    }
    // 以MQTT回傳資料
    /*
        {
          "MQTT":
          {
            "PUB" : [{
              "topic" : "wisdom/hunter",
              "message" : 123
            },{
              "topic" : "wisdom/hunter",
              "message" : 321
            }]
          }
        }
    */
    else if (item.key() == "MQTT")
    { // 向伺服器發送資料
      String rtStatus = "{\"status\":\"OK\"}";
      if (item.value().containsKey("PUB"))
      {
        JsonObject obj = item.value()["PUB"];
        uint8_t iserror = 0, timeNum = 0;
        for (JsonVariant itemPUB : item.value()["PUB"].as<JsonArray>())
        {
          if (!clientMQTT.publish(itemPUB["topic"].as<const char *>(), itemPUB["message"].as<const char *>()))
          {
            iserror++;
          }
          timeNum++;
        }
        if (iserror > 0)
        {
          _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "MQTT訊息發送失敗!");
          rtStatus = "{\"status\":\"ERROR\",\"Time\":" + String(iserror) + "}";
        }
        else
        {
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "Publish OK\n");
        }
      }

      //?回傳資料但底層已支持
      if (item.value().containsKey("SUB"))
        JsonObject obj = item.value()["SUB"];
      // 訂閱topic
      if (item.value().containsKey("subscription"))
        JsonObject obj = item.value()["subscription"];
      // 取消訂閱topic
      if (item.value().containsKey("unsubscribe"))
        JsonObject obj = item.value()["unsubscribe"];
    }
    /*
    {
      "UHF":
      {
        "Command" : "R",
                    "Data" : [ 2, 0, 4 ]
      }
    }
     */
    else if (item.key() == "UHF")
    {
      myFM505.Encoder(item.value().as<JsonObject>());
      JsonDocument doc;
      myFM505.Decoder(&doc);

      serializeJsonPretty(doc, rtStatus);
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, rtStatus);
      if (doc["Command"].as<String>() == "R")
      {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "R");
        JsonDocument docIsJson;
        DeserializationError error = deserializeJson(docIsJson, doc["Data"].as<String>());
        if (error)
        {
          _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "反序列化錯誤!以字串模式運作");
        }
        else
        {

          serializeJsonPretty(docIsJson, Serial);
          myCmdTable_Json(docIsJson.as<JsonObject>());
        }
      }
    }
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
  // HACK
  while (1)
  {
    Serial.println(analogRead(pinBattery_Vol) * 0.001611328125, 2);
    delay(100);
  }
  /*
  Adafruit_NeoPixel state(1, 14, NEO_GRB + NEO_KHZ800);
  Adafruit_NeoPixel strip(1, 15, NEO_GRB + NEO_KHZ800);
  state.begin();
  strip.begin();
  while (1)
  {
    for (int8_t j = 0; j < 3; j++)
    {
      for (byte i = 0; i < 250; i++)
      {
        state.setPixelColor(0, state.Color(j == 0 ? i : 0, j == 1 ? i : 0, j == 2 ? i : 0)); //  Set pixel's color (in RAM)
        strip.setPixelColor(0, strip.Color(j == 0 ? i : 0, j == 1 ? i : 0, j == 2 ? i : 0)); //  Set pixel's color (in RAM)
        state.show();
        strip.show();
        delay(10);
      }
      for (byte i = 250; i > 1; i--)
      {
        state.setPixelColor(0, state.Color(j == 0 ? i : 0, j == 1 ? i : 0, j == 2 ? i : 0)); //  Set pixel's color (in RAM)
        strip.setPixelColor(0, strip.Color(j == 0 ? i : 0, j == 1 ? i : 0, j == 2 ? i : 0)); //  Set pixel's color (in RAM)
        state.show();
        strip.show();
        delay(10);
      }
      // delay(1000);
    }
  }
*/
  // testDMX();
  // testReadDMX();
  ConfigInit();
  /**********************ROTS************************/
  xTaskCreatePinnedToCore(task_LED,
                          "task_LED",
                          4096,
                          NULL,
                          1,
                          NULL,
                          1);

  xTaskCreatePinnedToCore(WiFiInit,
                          "WiFiInit",
                          4096,
                          (void *)&(*Template_JsonPTC->getJsonObject()),
                          1,
                          NULL,
                          0);
  xTaskCreatePinnedToCore(task_WebSocket,
                          "task_WebSocket",
                          4096,
                          (void *)&(*Template_JsonPTC->getJsonObject()),
                          1,
                          NULL,
                          1);

  /*
  testMQTT();
  xTaskCreatePinnedToCore(task_MQTT,
                          "task_MQTT",
                          10240,
                          NULL,
                          1,
                          NULL,
                          0);
  myFM505.Begin(&Serial2);
  testUHF();
*/

  while (1)
    ;
  pinMode(pinTone, OUTPUT);
  pinMode(pinRelay[0], OUTPUT);
  pinMode(pinRelay[1], OUTPUT);
  task_Audio(NULL);
}

void loop()
{
}
void audio_info(const char *info)
{
  Serial.print("info        ");
  Serial.println(info);
}