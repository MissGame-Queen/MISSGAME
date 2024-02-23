#include "test.h"
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
  JsonDocument doc5;
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
      JsonDocument doc;
      doc["MQTT"].to<JsonObject>();
      doc["MQTT"]["PUB"].to<JsonObject>();
      doc["MQTT"]["PUB"]["topic"].to<JsonObject>();
      doc["MQTT"]["PUB"]["message"].to<JsonObject>();
      /*
      doc.createNestedObject("MQTT");
      doc["MQTT"].createNestedObject("PUB");
      doc["MQTT"]["PUB"].createNestedObject("topic");
      doc["MQTT"]["PUB"].createNestedObject("message");
      */
      doc["MQTT"]["PUB"]["topic"] = "wisdom/hunter";
      doc["MQTT"]["PUB"]["message"] = myFM505.getStringDecoder();
      serializeJsonPretty(doc, Serial);
      myCmdTable_Json(doc.as<JsonObject>());
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
  JsonDocument doc;
  String strDoc = "{\
          \"MQTT\":\
          {\
            \"PUB\" : {\
              \"topic\" : \"wisdom/hunter\",\
              \"message\" : 123\
            },\
          }\
        }";

  DeserializationError error = deserializeJson(doc, strDoc);
  if (error)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s\n", error.c_str());
  }

  while (true)
  {
    // 保持連線
    reconnect_MQTT();
    myCmdTable_Json(doc.as<JsonObject>());
    _DELAY_MS(2000);
    // 做其他的事情...
  }
}
/*
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
*/