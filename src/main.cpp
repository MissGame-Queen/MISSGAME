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
void test()
{
  pinMode(pinSet, INPUT_PULLUP);
  if (digitalRead(pinSet) && false)
  {
    Adafruit_NeoPixel strip(1, pinLED, NEO_GRB + NEO_KHZ800);
    bool statusSet = 1;
    strip.begin();
    pinMode(pinTrig, OUTPUT);
    pinMode(PinEcho, INPUT);
    pinMode(pinRelay[0], OUTPUT);
    pinMode(pinRelay[1], OUTPUT);
    pinMode(pinDAC_R, OUTPUT);
    pinMode(pinDAC_L, OUTPUT);
    setSDCard();
    getSHT40();
    setOLED();
    while (true)
    {
      for (byte j = 0; j < 3; j++)
      {

        pinMode(pinSet, INPUT_PULLUP);
        statusSet = digitalRead(pinSet);
        pinMode(pinSet, OUTPUT);
        getSHT40();
        setOLED();
        // getSonicRanging();
        Serial.printf("SET=%d，MODE=%d，Analog=%d，tem:%f，hum:%f\r",
                      statusSet, analogRead(pinmode), analogRead(pinAnalog), temp.temperature, humidity.relative_humidity);

        if (statusSet)
        {                                                                                      // For each pixel in strip...
          strip.setPixelColor(0, strip.Color(j == 0 ? 5 : 0, j == 1 ? 5 : 0, j == 2 ? 5 : 0)); //  Set pixel's color (in RAM)
          strip.show();                                                                        //  Update strip to match                                                                        //  Pause for a moment
          digitalWrite(pinRelay[0], 1);
          digitalWrite(pinRelay[1], 0);
        }
        else
        {
          digitalWrite(pinRelay[0], 0);
          digitalWrite(pinRelay[1], 1);
          setDAC();
        }
        delay(500);
      }
    }
  }
  else
  {
    setAudio();
  }
}

void callback_MQTT(char *topic, byte *payload, unsigned int length)
{
  StaticJsonDocument<1024> doc;

  DeserializationError error = deserializeJson(doc, payload, length);
  String str(payload, length);
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "收到新訊息!Topic: %s\nMessage:%s\n", topic, str.c_str());
  if (error)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s，以字串模式運行!\n", error.c_str());
  }
  else
  {
    // serializeJsonPretty(doc, Serial);
    CmdTable_Json(doc.as<JsonObject>());
  }
}
void reconnect_MQTT()
{
  // 重新連線到 MQTT 代理
  while (!clientMQTT.connected())
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "正在嘗試MQTT連線...");
    String clientId = "esp32-" + WiFi.macAddress();
    if (clientMQTT.connect(clientId.c_str()))
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "已連結到MQTT代理");
      // 訂閱主題
      clientMQTT.subscribe("enter/space");
    }
    else
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "錯誤代碼:%d，5秒後重連....", clientMQTT.state());
      _DELAY_MS(5000);
    }
  }
}
/**
 * @brief
 *
 * @param obj
 * V:模組版本
 * S:
 * Q:PC+EPC+CRC16
 */

void testUHF()
{
  myFM505.Begin(&Serial2);
  /*
  StaticJsonDocument<64> doc5;
  doc5.createNestedObject("Command");
  doc5.createNestedArray("Data");
  doc5["Command"] = "R"; // 讀取TID
  doc5["Data"].add(2);
  doc5["Data"].add(0);
  doc5["Data"].add(4);
  myFM505.CMD(FM505::FM505_FirmwareVersion);
  _DELAY_MS(200);
  myFM505.CMD(FM505::FM505_Reader_ID);
  _DELAY_MS(200);
*/
  while (1)
  {
    myFM505.CMD(FM505::FM505_Tag_EPC_ID);
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, myFM505.getStringDecoder());
    if (myFM505.getStringDecoder().length() > 4)
    {
      StaticJsonDocument<1024> doc;
      doc.createNestedObject("MQTT");
      doc["MQTT"].createNestedObject("PUB");
      doc["MQTT"]["PUB"].createNestedObject("topic");
      doc["MQTT"]["PUB"].createNestedObject("message");
      doc["MQTT"]["PUB"]["topic"] = "wisdom/hunter";
      doc["MQTT"]["PUB"]["message"] = myFM505.getStringDecoder();
      serializeJsonPretty(doc, Serial);
      CmdTable_Json(doc.as<JsonObject>());
    }
    _DELAY_MS(200);
  }
}

void testDMX()
{
  Serial2.begin(115200, SERIAL_8N1, 17, 16);
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
  const uint8_t pin[]{19, 21, 18, 5};
  const uint8_t data[][8] = {
      {0x01, 0x06, 0x1F, 0x41, 0x00, 0x01, 0x1F, 0xCA},
      {0x01, 0x06, 0x1F, 0x41, 0x00, 0x02, 0x5F, 0xCB},
      {0x01, 0x06, 0x1F, 0x41, 0x00, 0x03, 0x9E, 0x0B},
      {0x01, 0x06, 0x1F, 0x41, 0x00, 0x04, 0xDF, 0xC9}};
  for (uint8_t i = 0; i < sizeof(pin); i++)
    pinMode(pin[i], INPUT_PULLUP);

  bool swLN = 0;
  while (1)
  {
    for (uint8_t i = 0; i < sizeof(pin); i++)
    {
      if (!digitalRead(pin[i]))
      {
        Serial2.write(data[i], sizeof(data[i]));
        Serial.printf("輸出%d號!\n", i + 1);
        _DELAY_MS(500);
      }
    }

    while (Serial.available())
    {
      Serial2.write(Serial.read());
    }
    while (Serial2.available())
    {
      Serial.write(Serial2.read());
    }
    /*
    while (Serial2.available())
    {
      Serial.printf("%02x,", Serial2.read());
      swLN = 1;
    }
    if (swLN)
    {
      swLN = 0;
      Serial.println();
    }
    */
  }
}
void testMQTT()
{
  waitConnect(0);
  // 設定 MQTT 伺服器
  clientMQTT.setServer(_E2JS(MQTT_SERVER).as<const char *>(), _E2JS(MQTT_PORT).as<uint16_t>());
  // 設定回呼函式
  clientMQTT.setCallback(callback_MQTT);
  StaticJsonDocument<1024> doc;
  String strDoc = "{\
          \"MQTT\":\
          {\
            \"PUB\" : {\
              \"topic\" : \"wisdom/hunter\",\
              \"message\" : 123\
            },\
          }\
        }";
  doc.createNestedObject("MQTT");
  doc["MQTT"].createNestedArray("PUB");
  JsonObject pubObj = doc["MQTT"]["PUB"].createNestedObject();
  pubObj["topic"] = "wisdom/hunter";
  pubObj["message"] = "123";
  JsonObject pubObj2 = doc["MQTT"]["PUB"].createNestedObject();
  pubObj2["topic"] = "realopen/device_push/d56f0631";
  pubObj2["message"] = "321";
  /*
    DeserializationError error = deserializeJson(doc, strDoc);
    if (error)
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s\n", error.c_str());
    }
    */
  while (true)
  {
    // 保持連線
    reconnect_MQTT();
    CmdTable_Json(doc.as<JsonObject>());
    _DELAY_MS(2000);
    // 做其他的事情...
  }
}
void DMX::CellBack()
{
  for (uint16_t i = 1; i <= 32; i++)
  {
    Serial.printf("%02x,", DMX::Read(i));
  }
  Serial.println();
}
void testReadDMX()
{
  int readcycle = 0;
  DMXConfig config;
  config.pinRX = GPIO_NUM_16;
  config.pinTX = GPIO_NUM_17;
  config.direction = DMX_DIR_INPUT;
  config.pinDR = GPIO_NUM_NC;
  config.serialNum = 2;
  config.coreNum = 1;
  DMX::Initialize(config);
  while (1)
  {
    if (millis() - readcycle > 100)
    {
      readcycle = millis();
      DMX::IsHealthy();
    }
  }
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
  Init(23001);
  Template_JsonPTC = new JsonPTC(Template_CMDSize);

  if (Template_JsonPTC->Begin(getFS(), "/CommandTable.json") < 0)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "資料庫初始化錯誤!重置資料庫中...");
    Template_JsonPTC->RstConfig();
    while (Template_JsonPTC->SaveConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "Json Save ERROR!");
      vTaskDelay(3000 / portTICK_RATE_MS);
      ESP.restart();
    }
  }
  if (_E2JS(_INFORMATION).as<uint16_t>() != _T_E2S(_VER) && false) // HACK 暫時略過版本刷新
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "Ver ERROR! Rset Config...");
    Template_JsonPTC->RstConfig();
    _E2JS(_INFORMATION) = _T_E2S(_VER);
    while (Template_JsonPTC->SaveConfig() < 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "Json Save ERROR!");
      vTaskDelay(3000 / portTICK_RATE_MS);
      ESP.restart();
    }
  }
}
/**
 * @brief 指令表
//!不知為何.as<const char*>()不管用，需使用.as<String>().c_str();
//!用std::to_string會怪怪的??
//! Json賦值時需轉換成String
 */
String CmdTable_Json(JsonObject obj)
{
  String rtStatus = "";
  for (JsonPair item : obj)
  { // FIXME
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "Function:%s\n", item.key().c_str());
    if (item.key() == "LED")
    {
      // 默認模式:輸入整條的RGB
      if (!item.value().containsKey("Type") || item.value()["Type"] == "Default")
      {
        if (item.value().containsKey("R") && item.value().containsKey("G") &&
            item.value().containsKey("B"))
        {
          uint8_t numR = item.value()["R"].as<uint8_t>();
          uint8_t numG = item.value()["G"].as<uint8_t>();
          uint8_t numB = item.value()["B"].as<uint8_t>();
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "R:%d,G:%d,B:%d\n", numR, numG, numB);

          for (uint8_t i = 0; i < numLEDMax; i++)
          {
            ledType[i] = 0;
            ledR[i] = numR;
            ledG[i] = numG;
            ledB[i] = numB;
            ledTime[i] = millis();
          }
        }
      }
      else if (item.value().containsKey("Type") && item.value().containsKey("Index"))
      {
        if ((item.value()["Type"] == "Flash" || item.value()["Type"] == "Breathe") &&
            item.value().containsKey("OFF") && item.value().containsKey("ON") &&
            item.value().containsKey("R") && item.value().containsKey("G") && item.value().containsKey("B"))
        {
          uint8_t index = item.value()["Index"].as<uint8_t>();
          ledType[index] = item.value()["Type"] == "Flash" ? 1 : 2;
          if (item.value().containsKey("random"))
            ledType[index] += item.value()["random"].as<bool>() ? 0x80 : 0;
          ledR[index] = item.value()["R"].as<uint8_t>();
          ledG[index] = item.value()["G"].as<uint8_t>();
          ledB[index] = item.value()["B"].as<uint8_t>();
          ledON[index] = item.value()["ON"].as<uint16_t>();
          ledOFF[index] = item.value()["OFF"].as<uint16_t>();
        }
      }
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
      StaticJsonDocument<1024> doc;
      myFM505.Decoder(&doc);

      serializeJsonPretty(doc, rtStatus);
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, rtStatus);
      if (doc["Command"].as<String>() == "R")
      {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "R");
        StaticJsonDocument<1024> docIsJson;
        DeserializationError error = deserializeJson(docIsJson, doc["Data"].as<String>());
        if (error)
        {
          _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "反序列化錯誤!以字串模式運作");
        }
        else
        {

          serializeJsonPretty(docIsJson, Serial);
          CmdTable_Json(docIsJson.as<JsonObject>());
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
  // testDMX();
  testReadDMX();
  ConfigInit();
  xTaskCreatePinnedToCore(WiFiInit,
                          "WiFiInit",
                          2048,
                          (void *)&(*Template_JsonPTC->getJsonObject()),
                          1,
                          NULL,
                          0);
  testMQTT();
  while (1)
    ;
  xTaskCreatePinnedToCore(task_MQTT,
                          "task_MQTT",
                          10240,
                          NULL,
                          1,
                          NULL,
                          0);
  myFM505.Begin(&Serial2);
  testUHF();
  while (1)
    ;
  xTaskCreatePinnedToCore(task_LED,
                          "task_LED",
                          1536,
                          NULL,
                          1,
                          NULL,
                          0);
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