#include <Arduino.h>

// 0:W600主控,1:提燈,2:蠟燭
#define _TYPE 1
#if _TYPE == 0
#ifdef __AVR__
#error "板子或_TYPE錯誤!"
#endif
#else
#ifndef __AVR__
#error "板子或_TYPE錯誤!"
#endif
#endif
// #define _DEBUG 1
#include <ArduinoJson.h>

#if _TYPE == 0
#ifdef ESP32
#include <WiFi.h>
#else
#include <W600WiFi.h>
#endif
#include <PubSubClient.h>
#define _MQTT_IPPORT "192.168.0.8:1888" //"broker.MQTTGO.io:1883"
#define _WIFI_SSID "MissGAME_SOG"       //"MissGame_B2"
#define _WIFI_PASSWORD "missgame"
#define _MODULE_ID "100"
String clientId = "Lantern_" + String(_MODULE_ID);
const uint8_t pinColor[] = {11, 13, 12};
#else
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <SD.h>
const uint8_t pinSoftwareSerial_RX = 2;
const uint8_t pinSoftwareSerial_TX = 3;
SoftwareSerial Player_Serial(pinSoftwareSerial_RX, pinSoftwareSerial_TX);
bool havemp4Player = false;
DFRobotDFPlayerMini myDFPlayer;
const uint8_t pinOutput[] = {5, 6};
const uint8_t pinInput[] = {4, 7, 8, 9};
uint8_t intType = 0;
JsonDocument docArduino;
#endif

#if _TYPE == 1
const uint16_t analogMaxValue = 177;
#else
const uint16_t analogMaxValue = 177;
#endif
const bool onSide = 1;
#if _TYPE == 0
void MQTT_Callback(char *topic, uint8_t *payload, unsigned int length)
{
    JsonDocument doc;
    JsonArray args = doc.to<JsonArray>();
    args.add(topic);
    String eventName = args[0].as<const char *>();
    DeserializationError error = deserializeJson(args[1], payload, length);
    // String str(payload, length);
    String str = "";
    for (uint16_t i = 0; i < length; i++)
    {
        str += char(payload[i]);
    }
    if (error)
    {
        Serial.print("反序列化失敗:");
        Serial.print(error.c_str());
        Serial.print("，以字串模式運行!\n");
    }
    else
    {
        // serializeJsonPretty((*doc), Serial);

        if (eventName == "MissGame" || eventName == _MODULE_ID)
        {

            // 如果封包不包含ids自動補進id
            if (!args[1]["ids"].isNull() && args[1]["id"].isNull())
            {
                args[1]["ids"].add(args[1]["id"].as<uint16_t>());
            }
            // 或者event本身就是id則補進id
            else if (eventName == _MODULE_ID)
            {
                args[1]["ids"].add(String(_MODULE_ID).toInt());
            }

            JsonArray args = doc.as<JsonArray>();
            uint16_t id = 0;

            for (JsonVariant item : args[1]["ids"].as<JsonArray>())
            {
                id = item.as<uint16_t>();
                // FIXME 補上自製模組的運作方法

                if (id == String(_MODULE_ID).toInt())
                {
                    switch (id)
                    {
                    case 100 ... 110:
                    {
                        args[1].remove("ids");
                        JsonObject obj = args[1].as<JsonObject>();

                        // myCMD(&obj);
                        char tt[100];
                        serializeJson(obj, tt);
                        String trst = tt;
                        // 透過Serial傳輸Json控制從機
                        // W600不支援直接輸出Serial
                        Serial.print(trst);
                    }
                    break;
                    default:
                        Serial.print("無定義此ID:");
                        Serial.println(id);
                        break;
                    }
                }
            }

        } /*
                  else if (eventName == "Alive")
                    ;
                  else {
                    Serial.print("無定義此事件:");
                    Serial.println(eventName.c_str());
                  }
                  */
    }
    doc.clear();
}

void WiFi_Connect()
{

    static uint32_t timer = 0;
    static bool first = true;
    uint8_t status = WiFi.status();
    if (timer == 0 && status != WL_CONNECTED)
    {
        timer = millis();
        WiFi.mode(WIFI_STA);
        WiFi.begin(_WIFI_SSID, _WIFI_PASSWORD);
        Serial.print(millis());
        Serial.print(",嘗試連線到:");
        Serial.println(_WIFI_SSID);
    }
    else if (timer != 0 && millis() > timer + 1000 && status != WL_CONNECTED)
    {
        digitalWrite(pinColor[0], onSide);
        digitalWrite(pinColor[1], !onSide);
        digitalWrite(pinColor[2], !onSide);
        Serial.print(".");
        timer = millis();
    }
    else if (timer != 0 && status == WL_CONNECTED)
    {
        Serial.println("");
        Serial.println("WiFi已接回:");
        Serial.println(WiFi.localIP());
        digitalWrite(pinColor[0], !onSide);
        timer = 0;
    }
    else if (first && status == WL_CONNECTED)
    {
        digitalWrite(pinColor[0], !onSide);
        first = false;
        Serial.println();
        Serial.println("初次WiFi連線:");
        Serial.println(WiFi.localIP());
    }
}
void MQTT_Connect()
{
    String ip_port = _MQTT_IPPORT;
    static int colonIndex = ip_port.lastIndexOf(':'); // 找到最後一個冒號的位置
    if (colonIndex == -1)
    {
        Serial.print("MQTT網址錯誤!");
        Serial.print(ip_port.c_str());
        Serial.println();
        while (1)
        {
            delay(100);
        }
    }
    // 變數不能放條件內 不知為何會連不到
    static String ipStr = ip_port.substring(0, colonIndex);
    static String portStr = ip_port.substring(colonIndex + 1); // 提取冒號後的子字符串
    static uint16_t port = portStr.toInt();                    // 將字符串轉換為uint16_t類型
    static WiFiClient WiFiClient;
    static PubSubClient MQTTClient(WiFiClient);
    static bool first = true;
    if (first)
    {
        first = false;
        MQTTClient.setServer(ipStr.c_str(), port);
        // 設定回呼函式
        MQTTClient.setCallback(MQTT_Callback);
    };
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!MQTTClient.connected())
        {
            static uint32_t timer = 0;
            Serial.print("正在嘗試MQTT連線...{");
            Serial.print(ipStr.c_str());
            Serial.print(",");
            Serial.print(port);
            Serial.print("}\n");
            digitalWrite(pinColor[1], !onSide);
            digitalWrite(pinColor[2], onSide);
            int8_t status = MQTTClient.connect((clientId != "" ? clientId.c_str() : String("W600-" + String(WiFi.macAddress())).c_str()));
            if (status > 0)
            {
                Serial.println("已連結到MQTT代理!重新訂閱主題");
                digitalWrite(pinColor[1], onSide);
                digitalWrite(pinColor[2], !onSide);
                MQTTClient.subscribe("MissGame");
                MQTTClient.subscribe(_MODULE_ID);
                timer = 0;
                // MQTTClient.publish("test/topic", String("{\"data\":\"Hello!I am " + clientId + "\"}").c_str());
            }
            else if (status <= 0 && timer == 0)
            {
                timer = millis();
            }
            else if (status <= 0 && timer != 0 && millis() > timer + 5000)
            {
                timer = millis();
                Serial.print("錯誤代碼:");
                Serial.print(MQTTClient.state());
                Serial.print("，5秒後重連....\n");
            }
        }
        MQTTClient.loop();
    }
}
#else
/**
 * @brief 默認讀取遙控器訊號以變更參數
 * 也可手動輸入強制變更參數
 *
 * @param valueIn 0xFF=默認,其他以二進制表示輸入的訊號
 */
void RemoteControl(uint8_t valueIn = 0xFF)
{
    uint8_t value = 0;
    static uint8_t lastValue = 0;
    if (0xFF == valueIn)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            value += (digitalRead(pinInput[i]) ? 1 << i : 0);
        }
    }
    else
    {
        value = valueIn;
        lastValue = 0;
    }

    // 如果遙控器被按下則變更類型
    if (lastValue != value)
    {
        for (uint8_t i = 0; i < 4; i++)
        {
            if (value & (1 << i))
            {
                intType = i;
                lastValue = value;
                return;
            }
        }
        lastValue = value;
    }
}
/**
 * @brief 處理Serial收到的json
 *
 * @param ptrObj
 */
void myCMD(JsonObject *ptrObj)
{
    JsonObject obj = (*ptrObj);
    serializeJsonPretty(obj, Serial);
    bool haveLog = !obj["log"].isNull();
    if (haveLog)
    {
        File file;
        file = SD.open("/log.log", FILE_WRITE);
        if (!file)
        {
            Serial.println("log開啟錯誤");
        }
        else
        {
            file.print(obj["log"].as<const char *>());
            file.print(",");
            file.println(obj["value"].as<uint8_t>());
            // serializeJsonPretty(obj, Serial);
            // file.println();
        }
        file.close();
    }
    if (!obj["value"].isNull())
    {
        RemoteControl(obj["value"].as<uint8_t>());
    }
    /*
          // 燈光控制
          if (!obj["lingth"].isNull())
          {
              uint16_t value = obj["lingth"].as<uint16_t>();
      #ifdef _DEBUG
              Serial.print("燈光變更為:");
              Serial.println(value);
      #endif
              intLingth = value;
          }
          // 震動控制
          if (!obj["shock"].isNull())
          {
              bool value = obj["shock"].as<bool>();
      #ifdef _DEBUG
              Serial.print("震動變更為:");
              Serial.println(value);
      #endif
              intShock = value;
              // intShock = value;
          }
          // 音效控制
          if (!obj["sound"].isNull()&&havemp4Player)
          {
              if (!obj["sound"]["value"].isNull())
              {
                  uint16_t value = obj["sound"]["value"].as<uint16_t>();
      #ifdef _DEBUG
                  Serial.print("音效播放:");
                  Serial.println(value);
      #endif
                  if (obj["sound"]["loop"].isNull() || !obj["sound"]["loop"].as<bool>())
                  {
                      myDFPlayer.play(value);
                      delay(20);
                  }
                  else
                  {
                      myDFPlayer.loop(value);
                  }
                  delay(20);
              }
              if (!obj["sound"]["volume"].isNull())
              {
                  myDFPlayer.volume(obj["sound"]["volume"].as<uint8_t>());
                  delay(20);
              }
              // myDFPlayer.loop(mp3Value);
          }
      */
}
/**
 * @brief 依類型做出相應變化
 *
 */
void runType()
{
    static uint8_t lastValue = 0xFF;
    static uint8_t intLingth = 0;
    static uint32_t timer = 0;
    // 初始化紀錄位
    const uint8_t setBin = B10000000;
    // 如果狀態被改變且未被初始化
    if (lastValue != intType)
    {
        // Serial.println(intType);
        switch (intType)
        {
            // A:燈ON/閃爍
        case 0:
        {
            timer = millis();
#if _TYPE == 1
            intLingth = (intLingth > 0 ? 0 : 1);
            // intLingth > 0 ? digitalWrite(pinOutput[0], onSide) : digitalWrite(pinOutput[0], !onSide);
            if (intLingth > 0)
                analogWrite(pinOutput[0], analogMaxValue);
            digitalWrite(pinOutput[1], !onSide);
            if (havemp4Player)
                myDFPlayer.stop();
#else
            analogWrite(pinOutput[0], analogMaxValue);
            intLingth = 1;
#endif
        }
        break;

            // B:燈OFF
        case 1:
        {
            intLingth = 0;
            digitalWrite(pinOutput[0], !onSide);
            digitalWrite(pinOutput[1], !onSide);
            if (havemp4Player)
                myDFPlayer.stop();
        }
        break;
            // C:呼吸由慢到快閃共10S後全暗,播音效,含震動
        case 2:
            intLingth = 0;
            timer = millis();
            digitalWrite(pinOutput[1], onSide);
            if (havemp4Player)
                myDFPlayer.play(1);
            break;
            // D:
        case 3:
            break;
        }
        intType = intType | setBin;
        lastValue = intType;
    }
    // 否則執行完初始化後的程式
    else
    {
        switch (intType)
        {
            // A:燈ON/閃爍
        case 0 + setBin:
        {
            if (intLingth > 0)
                ;
            else
            {
                uint16_t val = (millis() % 200);
                if (val <= 100)
                    analogWrite(pinOutput[0], map(val, 0, 100, 0, analogMaxValue));
                else
                    analogWrite(pinOutput[0], map(val, 101, 200, analogMaxValue, 0));
            }
        }
        break;
            // B:燈OFF
        case 1 + setBin:
        {
        }
        break;
            // C:呼吸由慢到快閃共10S後全暗,播音效,含震動
        case 2 + setBin:
        {
            const uint16_t intMax = 1000, intMin = 200;
            const uint32_t setTimer = 10000; // 10S
            uint32_t valueTimer = millis() - timer;
            if (valueTimer > 500)
            {
                digitalWrite(pinOutput[1], onSide);
            }
            if (valueTimer < setTimer)
            {
                uint16_t value = map(valueTimer, 0, setTimer, intMax, intMin);
                valueTimer = valueTimer % value;
                if (valueTimer < value / 2)
                    analogWrite(pinOutput[0], map(valueTimer, 0, value / 2, 0, analogMaxValue));
                else
                    analogWrite(pinOutput[0], map(valueTimer, value / 2, value, analogMaxValue, 0));
            }
            // 動畫結束後
            else
            {
                digitalWrite(pinOutput[0], !onSide);
                digitalWrite(pinOutput[1], !onSide);
                if (havemp4Player)
                    myDFPlayer.stop();
            }
        }
        break;
            // D:
        case 3 + setBin:
            break;
        }
    }
}

#endif
void task()
{
    Serial.begin(115200);

// 初始化
#if _TYPE == 0
#ifndef ESP32
    delay(3000); // 用來等待上傳用

    for (uint8_t i = 0; i < 3; i++)
    {
        pinMode(pinColor[i], OUTPUT);
        digitalWrite(pinColor[i], !onSide);
    }
    digitalWrite(pinColor[0], onSide);
#endif
#else

    for (uint8_t i = 0; i < sizeof(pinInput); i++)
    {
        pinMode(pinInput[i], INPUT);
    }
    for (uint8_t i = 0; i < sizeof(pinOutput); i++)
    {
        pinMode(pinOutput[i], OUTPUT);
        digitalWrite(pinOutput[i], !onSide);
    }
    uint16_t delayTime = 20;
    Player_Serial.begin(9600);
    havemp4Player = myDFPlayer.begin(Player_Serial, /*isACK = */ true, /*doReset = */ true);
    if (havemp4Player)
    {
#ifdef _DEBUG
        Serial.println("DFPlayer Mini 已連線\n");
#endif
        myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms
        delay(delayTime);
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
        delay(delayTime);
        myDFPlayer.volume(30);
        delay(delayTime);
        myDFPlayer.disableLoop();
        delay(delayTime);
        myDFPlayer.stop();
    }
    if (SD.begin(10))
    {
#ifdef _DEBUG
        Serial.println("SD卡 已連線\n");
#endif
    };
#endif
    Serial.println("開始執行!");
    uint16_t dalayTime = 10;
    while (true)
    {

#if _TYPE == 0
        // 保持WiFi連線
        WiFi_Connect();
        // 保持MQTT連線
        MQTT_Connect();
#else
        if (Serial.available())
        {
            delay(10);
            docArduino.clear();
            DeserializationError error = deserializeJson(docArduino, Serial);
            if (error)
            {
#ifdef _DEBUG
                Serial.print("反序列化失敗:");
                Serial.println(error.c_str());
#endif
            }
            else
            {
                JsonObject obj = docArduino.as<JsonObject>();
                myCMD(&obj);
            }
        }
        RemoteControl();
        runType();

#endif
        delay(dalayTime);
    }
}

void setup()
{
    task();
}

void loop() {}
