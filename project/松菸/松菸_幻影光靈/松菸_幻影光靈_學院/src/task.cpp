
#include <Arduino.h>
#define IR_RECEIVE_PIN 34  // D15
#define IR_SEND_PIN 14     // D4
#define TONE_PIN 27        // D27 25 & 26 are DAC0 and 1
#define APPLICATION_PIN 16 // RX2 pin
// #define LED_BUILTIN 16     // RX2 pin
#define DECODE_NEC
#include <IRremote.hpp> // include the library
//?不知為何要放.cpp最上面

#include "task.h"

uint16_t SoundPlayerLevel[2] = {0, 0};
String SoundPlayerName[2] = {"", ""};
Audio *audioPCM5102 = new Audio();
QueueHandle_t queueJson = xQueueCreate(1, sizeof(String));
QueueHandle_t queueDFPlayer = xQueueCreate(1, sizeof(uint16_t));
QueueHandle_t queueBallTime = xQueueCreate(1, sizeof(uint16_t));
QueueHandle_t queueTimer = xQueueCreate(1, sizeof(String));
QueueHandle_t queueWeaponLight = xQueueCreate(1, sizeof(uint8_t));
QueueHandle_t queueLINE_POST = xQueueCreate(1, sizeof(String));

/**
 * @brief 出幣機程式
 *
 */
void CoinDispenser(uint16_t time)
{
    const uint8_t pinServo = 15 /*13*/, pinLED = 4, pinTrigger = 36; // 15;
    const uint16_t waittim = 600;
    const int16_t startdeg = _E2JS(_SEVER_DEG_START).as<uint8_t>();
    const int16_t enddeg = _E2JS(_SEVER_DEG_END).as<uint8_t>();
    Servo myServo;            //   Create Servo object to control the servo
    myServo.attach(pinServo); //   Servo is connected to digital pin 9
    myServo.write(startdeg);
    pinMode(pinLED, OUTPUT);
    pinMode(pinTrigger, INPUT_PULLUP);
    uint16_t BallTime = 0;
    uint16_t BallTimeAdd = 0;
    while (1)
    {

        bool isTrigger = analogRead(pinTrigger) > 512;
        if (!isTrigger)
        {
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被實體按鈕觸發了~");
            BallTime++;
            // Serial.println(BallTime);
        }
        if (xQueueReceive(queueBallTime, &BallTimeAdd, 0) == pdPASS)
            BallTime += BallTimeAdd;
        while (BallTime > 0)
        {
            BallTime--;
            digitalWrite(pinLED, 0);
            myServo.write(enddeg);
            _DELAY_MS(waittim);
            digitalWrite(pinLED, 1);
            myServo.write(startdeg);
            _DELAY_MS(waittim);
        }
        _DELAY_MS(50);
    }
}

/**
 * @brief 出球機程式
 *
 */
void taskBallDispenser(void *pvParam)
{
    Serial1.begin(9600, SERIAL_8N1, 22, 21);

    pinMode(13, INPUT_PULLUP);
    uint16_t BallTime = 0;
    uint16_t BallTimeAdd = 0;
    while (1)
    {
        if (!digitalRead(13))
        {
            BallTime++;
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出球機被實體按鈕觸發了~");
        }
        if (xQueueReceive(queueBallTime, &BallTimeAdd, 0) == pdPASS)
            BallTime += BallTimeAdd;

        if (BallTime > 0)
        {
            bool dir = 0;
            uint16_t speed = 50;
            uint8_t acc = 200;
            uint32_t pulses = 0x42A * BallTime;
            uint16_t dalayTime = 400;
            uint8_t parameter[6];
            parameter[0] = (dir << 7) + ((speed >> 8) & 0xFF);
            parameter[1] = speed & 0xFF;
            for (uint8_t i = 0; i < 4; i++)
                parameter[2 + i] = (pulses >> (8 * (3 - i))) & 0xFF;
            uint8_t dataWrite[] = {0xFA, 0x01, 0xFD, parameter[0], parameter[1], acc, parameter[2],
                                   parameter[3], parameter[4], parameter[5], 0x00};
            uint16_t crc = 0;
            for (uint8_t i = 0; i < sizeof(dataWrite); i++)
                crc += dataWrite[i];
            dataWrite[sizeof(dataWrite) - 1] = crc & 0xFF;
            Serial1.write(dataWrite, sizeof(dataWrite));
            _DELAY_MS(dalayTime);
            if (Serial1.available())
            {
                uint8_t recallData[12];
                for (uint8_t i = 0; i < 12 && Serial1.available(); i++)
                {
                    recallData[i] = Serial1.read();
                    Serial.printf("0x%02x,", recallData[i]);
                    _DELAY_MS(5);
                }
                Serial.println();
                if (recallData[0] == 0xFB &&
                    recallData[1] == 0x01 &&
                    recallData[2] == 0xFD &&
                    recallData[3] == 0x01 &&
                    recallData[4] == 0xFA)
                {
                    BallTime = 0;
                }
            }
        }
        _DELAY_MS(100);

        if (Serial1.available())
        {
            while (Serial1.available())
                Serial.printf("0x%02X,", Serial1.read());
            Serial.println();
        }
    }
}

/**
 * @brief 計時器程式
 *
 */
/*
void taskTimer(void *pvParam)
{
    // uint16_t second = 0;
    String nowsecond = "00:00";
    uint8_t status[2] = {0, 0};
    const uint8_t pinDS = 23, pinSH = 18, pinST = 4;
    const uint8_t pinOut[2]{22, 21};
    const uint8_t pinin[]{36, 39, 34, 35, 32};
    enum eStatus
    {
        STOP,  // 停止(不輸出)
        RUN,   // 倒數中
        PAUSE, // 暫停倒數
        END,   // 停止(輸出訊號)
        TEST,
    };
    SPI.begin(pinSH, 21, pinDS);
    pinMode(pinDS, OUTPUT); // MO
    pinMode(pinSH, OUTPUT); // SCK
    pinMode(pinST, OUTPUT); // CS
    pinMode(pinOut[0], OUTPUT);
    pinMode(pinOut[1], OUTPUT);

    Timer_status = TEST;
    time_t time = 0;
    tmTimer = *localtime(&time);
    /*
    while (xQueueReceive(queueTimer, &second, 100) != pdPASS)
        _DELAY_MS(1000);
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已收到秒數!%d\n", second);
    nowsecond = second;
    status[i] = RUN;

    uint8_t timeNum = 0;
    bool isNeedPrint = 1;
    bool isSet = false;
    while (1)
    {
        status[0] = Timer_status;
        // 如果狀態改變或處於倒數狀態
        if (status[0] != status[1] || status[0] == RUN)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "狀態為:%d,%d\n", status[0], status[1]);
            status[1] = status[0];
            switch (status[0])
            {
            case STOP:
                digitalWrite(pinOut[0], 1);
                digitalWrite(pinOut[1], 0);
                nowsecond = Timer_newSecond;
                isNeedPrint = 1;
                isSet = true;
                tmTimer.tm_min = Timer_newSecond.substring(0, 2).toInt();
                tmTimer.tm_sec = Timer_newSecond.substring(Timer_newSecond.length() - 2).toInt();
                char buffer[10];
                if (millis() % 1000 < 500)
                    sprintf(buffer, "%02d:%02d", tmTimer.tm_min, tmTimer.tm_sec);
                else
                    sprintf(buffer, "%02d%02d", tmTimer.tm_min, tmTimer.tm_sec);
                nowsecond = buffer;
                set74HC595(nowsecond);
                break;
            case END:
                digitalWrite(pinOut[0], 0);
                digitalWrite(pinOut[1], 1);
                set74HC595("00:00");
                break;
            case PAUSE:
                break;
            case RUN:
            {
                char buffer[10];
                if (isNeedPrint)
                {

                    if (!isSet)
                    {
                        // 將 tm 轉換為 time_t 型別
                        time_t time = mktime(&tmTimer);
                        if (tmTimer.tm_min != 0 || tmTimer.tm_sec != 0)
                        {
                            // 扣 1 秒
                            time--;
                            // 將 time 轉換為 tm 型別
                            tmTimer = *localtime(&time);
                            if (tmTimer.tm_min == 0 && tmTimer.tm_sec == 0)
                                Timer_status = END;
                        }
                        else
                            Timer_status = END;
                    }
                    else
                        isSet = false;
                    sprintf(buffer, "%02d:%02d", tmTimer.tm_min, tmTimer.tm_sec);
                    isNeedPrint = !isNeedPrint;
                }
                else
                {
                    sprintf(buffer, "%02d%02d", tmTimer.tm_min, tmTimer.tm_sec);
                    isNeedPrint = !isNeedPrint;
                }
                nowsecond = buffer;
                set74HC595(nowsecond);
            }
            break;
            case TEST:
                for (byte i = 0; i < 10; i++)
                {
                    set74HC595(String(i) + String(i) + (i % 2 == 0 ? ":" : "") + String(i) + String(i));
                    delay(500);
                }
                Timer_status = END;
                break;
            }
        }
        _DELAY_MS(500);
    }
}
/*
/**
 * @brief
 *
 */
void taskFQ512(uint16_t cmd)
{
    // Tx:000514-01 06 1F 41 00 01 1F CA
    // Tx:000515-01 06 1F 41 00 00 DE 0A

    CRC16_parameter_t Modbus_CRC;
    uint8_t dataWrite[] = {0x01, 0x06, 0x1F, 0x41, uint8_t((cmd >> 8) & 0xFF), uint8_t(cmd & 0xFF), 0x00, 0x00};
    uint8_t dataLength = sizeof(dataWrite);
    uint16_t crc = CRC_16(dataWrite, dataLength - 2, &Modbus_CRC);
    dataWrite[dataLength - 1] = crc >> 8;
    dataWrite[dataLength - 2] = crc & 0xFF;
    for (uint8_t i = 0; i < dataLength; i++)
    {
        Serial.printf("0x%02X,", dataWrite[i]);
        Serial2.write(dataWrite[i]);
    }
    Serial.println();

    _DELAY_MS(100);
    /*
        for (uint8_t i = 0; i < sizeof(dataWrite); i++)
            Serial.printf("%02x,", dataWrite[i]);
        Serial.println();
        */
}

/**
 * @brief 武器燈
 *
 */
void taskWeaponLight(void *pvParam)
{

    JsonDocument *doc = (JsonDocument *)pvParam;
    const uint8_t pinOut[]{25, 26, 27, 33};
    const uint8_t pinCHG = 5, pinCHG_Invert = 4;
    const uint8_t pinBattery = 32;
    for (size_t i = 0; i < sizeof(pinOut); i++)
    {
        ledcSetup(i, 1000, 12);
        ledcAttachPin(pinOut[i], i);
        ledcWrite(i, 0);
    }
    pinMode(pinCHG, INPUT_PULLUP);
    pinMode(pinCHG_Invert, INPUT_PULLDOWN);
    Adafruit_NeoPixel strip((*doc).containsKey("Length") ? (*doc)["Length"] : 11,
                            (*doc).containsKey("Pin") ? (*doc)["Pin"] : 15,
                            NEO_GRB + NEO_KHZ800);
    strip.begin();
    if (xSemaphoreTake(rmtMutex, portMAX_DELAY))
    {
        strip.show();
        xSemaphoreGive(rmtMutex);
    }

    uint16_t id = _E2JS(_MODULE_ID).as<uint16_t>();
    uint32_t color = 0;
    uint8_t num = 0;
    uint32_t batterTimer = 0;
    uint8_t level = 0;
    uint16_t limitPWM[] = {50, 50, 50, 0};

    bool isAutoPam = true;
    if (
        (*doc).containsKey("_LIGHT_0") &&
        (*doc).containsKey("_LIGHT_1") &&
        (*doc).containsKey("_LIGHT_2") &&
        (*doc).containsKey("_LIGHT_3"))
    {
        if ((*doc)["_LIGHT_0"].as<uint16_t>() != 0 ||
            (*doc)["_LIGHT_1"].as<uint16_t>() != 0 ||
            (*doc)["_LIGHT_2"].as<uint16_t>() != 0 ||
            (*doc)["_LIGHT_3"].as<uint16_t>() != 0)
        {
            isAutoPam = false;
        }
    }
    if (isAutoPam)
    {
        switch (id)
        {
        case 40 ... 49:
            limitPWM[0] = 50;
            limitPWM[1] = 50;
            limitPWM[2] = 50;
            break;
        case 50 ... 59:
            limitPWM[0] = 50;
            limitPWM[1] = 4000;
            break;
        case 60 ... 69:
            limitPWM[0] = 4000;
            limitPWM[1] = 4000;
            limitPWM[2] = 4000;
            break;
        case 70 ... 79:
            limitPWM[0] = 4000;
            limitPWM[1] = 4000;
            limitPWM[2] = 4000;
            break;
        case 80 ... 89:
            limitPWM[0] = 50;
            limitPWM[1] = 50;
            limitPWM[2] = 50;
            break;
        }
    }
    else
    {
        limitPWM[0] = (*doc)["_LIGHT_0"].as<uint16_t>();
        limitPWM[1] = (*doc)["_LIGHT_1"].as<uint16_t>();
        limitPWM[2] = (*doc)["_LIGHT_2"].as<uint16_t>();
        limitPWM[3] = (*doc)["_LIGHT_3"].as<uint16_t>();
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%d,%d,%d,%d\n", limitPWM[0], limitPWM[1], limitPWM[2], limitPWM[3]);
    }
    while (1)
    {
        if (xQueueReceive(queueWeaponLight, &level, 0) == pdPASS)
        {
            (*doc)["Level"].set(level);
            //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "等級為%d\n", (*doc)["Level"].as<uint8_t>());
        }

        /*
 30~39 杖
 40~49 矛
 50~59 錘
 60~69 鍊
 70~79 劍
 80~89 斧
 */

        switch ((*doc)["Level"].as<uint8_t>())
        {
        case 1:
        {
            ledcWrite(1, 0);
            ledcWrite(2, 0);
            ledcWrite(3, 0);
            switch (num)
            {
            case 1 ... 5:
            case 16 ... 20:
                for (uint16_t i = 0; i < strip.numPixels(); i++)
                    strip.setPixelColor(i, strip.Color(255, 0, 0));
                ledcWrite(0, limitPWM[0]);
                break;
            case 6 ... 15:
            case 21 ... 60:
                for (uint16_t i = 0; i < strip.numPixels(); i++)
                    strip.setPixelColor(i, 0);
                ledcWrite(0, 0);
                break;
            default:
                num = 0;
                break;
            }
            num++;
        }
        break;
        case 2:
        {
            ledcWrite(2, 0);
            ledcWrite(3, 0);
            int16_t cycle = 2000;
            int16_t cycle_color = 1500;
            int16_t brightness = 155;
            uint16_t val = map(millis() % cycle, 0, cycle, 0, strip.numPixels() * 2);
            color = setRainbowRGB(map(millis() % cycle_color, 0, cycle_color, 0, 1536));
            for (uint16_t i = 0; i < strip.numPixels(); i++)
            {
                uint32_t ws2812color = color;
                uint8_t b = (i <= val && val - i <= strip.numPixels() ? map(val - i, 0, strip.numPixels(), brightness, 0) : 0);
                ws2812color = setBrightnessRGB(ws2812color, b);
                strip.setPixelColor(i, ws2812color);
            }
            uint16_t ledval = millis() % cycle;
            for (uint8_t i = 0; i < 2; i++)
            {
                if (ledval <= cycle / 2)
                    brightness = map(ledval, 0, cycle / 2, 0, limitPWM[i]);
                else
                    brightness = map(ledval, cycle / 2, cycle, limitPWM[i], 0);
                ledcWrite(i, brightness);
            }
        }
        break;
        case 3:
        {
            int16_t cycle = 1000;
            int16_t brightness = 100;
            for (uint16_t i = 0; i < strip.numPixels(); i++)
            {
                color = setRainbowRGB((map(millis() % cycle, 0, cycle, 1536, 0) + map(i, 0, strip.numPixels(), 0, 1536)) % 1536);
                color = setBrightnessRGB(color, brightness);
                strip.setPixelColor(i, color);
            }
            for (uint8_t i = 0; i < 3; i++)
                ledcWrite(i, limitPWM[i]);
        }
        break;
            // 充電動畫
        case 97:
        {
            const int intMax = 420, intMin = 300;
            static int roundedNumber = 0;
            static uint8_t intLevelBatter = 0;
            static uint8_t intFlashTime = 0;
            static uint8_t intONorOFF = 0;
            // float valBattery = roundedNumber / 100.0;// 将四舍五入后的整数除以100得到保留两位小数的浮点数
            // uint32_t timer = millis();
            // uint8_t maxled = map(roundedNumber, intMin, intMax, 1, strip.numPixels());

            intONorOFF++;
            // 亮完一輪停一下
            if (intFlashTime >= intLevelBatter)
            {
                if (intFlashTime == intLevelBatter)
                {
                    float number = analogRead(pinBattery) * 0.0017465437788018; // 假设这是你的浮点数值
                    roundedNumber = round(number * 100);                        // 将浮点数乘以100后四舍五入为整数
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "讀取電池電量!%d,%d0 % \n", roundedNumber, intLevelBatter);
                    intLevelBatter = map(min(max(roundedNumber, intMin), intMax), intMin, intMax, 1, 10);
                    intFlashTime++;
                }
                if (intONorOFF < 20)
                {
                    ledcWrite(0, 0);
                    ledcWrite(1, 0);
                    ledcWrite(2, 0);
                    ledcWrite(3, 0);
                    for (uint16_t i = 0; i < strip.numPixels(); i++)
                        strip.setPixelColor(i, 0);
                }
                else
                {
                    intONorOFF = 0;
                    intFlashTime = 0;
                }
            }
            // 正常閃爍電量次數
            else
            {
                if (intONorOFF < 3)
                {
                    if (intONorOFF == 1)
                    {
                        ledcWrite(0, limitPWM[0]);
                        ledcWrite(1, limitPWM[1]);
                        ledcWrite(2, limitPWM[2]);
                        ledcWrite(3, limitPWM[3]);
                        color = setRainbowRGB(map(min(max(roundedNumber, intMin), intMax), intMin, intMax, 0, 1536));
                        color = setBrightnessRGB(color, 20);
                        for (uint16_t i = 0; i < strip.numPixels(); i++)
                            strip.setPixelColor(i, color);
                    }
                }
                else if (intONorOFF >= 3 && intONorOFF < 9)
                {
                    if (intONorOFF == 3)
                    {
                        ledcWrite(0, 0);
                        ledcWrite(1, 0);
                        ledcWrite(2, 0);
                        ledcWrite(3, 0);
                        for (uint16_t i = 0; i < strip.numPixels(); i++)
                            strip.setPixelColor(i, 0);
                    }
                }
                else
                {
                    intONorOFF = 0;
                    intFlashTime++;
                }
            }
        }
        break;
        // 亮度測試
        case 98:
            ledcWrite(0, limitPWM[0]);
            ledcWrite(1, limitPWM[1]);
            ledcWrite(2, limitPWM[2]);
            ledcWrite(3, limitPWM[3]);
            for (uint16_t i = 0; i < strip.numPixels(); i++)
            {
                color = setRainbowRGB((map(millis() % 1000, 0, 1000, 0, 1536) + map(i, 0, strip.numPixels(), 0, 1536)) % 1536);
                color = setBrightnessRGB(color, 125);
                strip.setPixelColor(i, color);
            }
            break;
            // 預設開機動畫
        case 99:
        {
            switch (num)
            {
            case 0 ... 20:
                ledcWrite(0, map(num, 0, 20, 0, limitPWM[0]));
                ledcWrite(1, 0);
                ledcWrite(2, 0);
                ledcWrite(3, 0);
                break;
            case 21 ... 40:
                ledcWrite(0, limitPWM[0]);
                ledcWrite(1, map(num, 21, 40, 0, limitPWM[1]));
                ledcWrite(2, 0);
                ledcWrite(3, 0);
                break;
            case 41 ... 60:
                ledcWrite(0, limitPWM[0]);
                ledcWrite(1, limitPWM[1]);
                ledcWrite(2, map(num, 41, 60, 0, limitPWM[2]));
                ledcWrite(3, 0);
                break;
            case 61 ... 80:
                ledcWrite(0, limitPWM[0]);
                ledcWrite(1, limitPWM[1]);
                ledcWrite(2, map(num, 61, 80, limitPWM[2], 0));
                ledcWrite(3, 0);
                break;
            case 81 ... 100:
                ledcWrite(0, limitPWM[0]);
                ledcWrite(1, map(num, 81, 100, limitPWM[1], 0));
                ledcWrite(2, 0);
                ledcWrite(3, 0);
                break;
            case 101 ... 120:
                ledcWrite(0, map(num, 101, 120, limitPWM[0], 0));
                ledcWrite(1, 0);
                ledcWrite(2, 0);
                ledcWrite(3, 0);
                break;

            default:
                num = 0;
                break;
            }
            num++;

            for (uint16_t i = 0; i < strip.numPixels(); i++)
            {
                color = setRainbowRGB((map(millis() % 1000, 0, 1000, 0, 1536) + map(i, 0, strip.numPixels(), 0, 1536)) % 1536);
                color = setBrightnessRGB(color, num < 60 ? map(num, 0, 61, 0, 125) : map(num, 61, 121, 125, 0));
                strip.setPixelColor(i, color);
            }
        }
        break;
        default:
        {
            ledcWrite(0, 0);
            ledcWrite(1, 0);
            ledcWrite(2, 0);
            ledcWrite(3, 0);
            for (uint16_t i = 0; i < strip.numPixels(); i++)
                strip.setPixelColor(i, 0);
        }
        break;
        }
        if (xSemaphoreTake(rmtMutex, portMAX_DELAY))
        {
            strip.show();
            xSemaphoreGive(rmtMutex);
        }
        _DELAY_MS((*doc)["DelayTime"].as<uint16_t>());
        /**
         * @brief 每10秒送目前電池電壓值
         *
         */
        // _E2JS(_BATTERY_VAL) = roundf(analogRead(pinBattery) * 0.0017465437788018 * 100) / 100;
        if (millis() > batterTimer + 10000)
        {

            if (socketIO_Client.isConnected())
            {
                float number = analogRead(pinBattery) * 0.0017465437788018; // 假设这是你的浮点数值
                int roundedNumber = round(number * 100);                    // 将浮点数乘以100后四舍五入为整数
                _E2JS(_BATTERY_VAL) = roundedNumber / 100.0;                // 将四舍五入后的整数除以100得到保留两位小数的浮点数
                const char *str = String(_E2JS(_BATTERY_VAL).as<float>()).c_str();
                JsonDocument doc;
                JsonArray array = doc.to<JsonArray>();
                array.add("MissGame");
                JsonObject param1 = array.add<JsonObject>();
                // JsonObject param1 = array.createNestedObject();
                param1["battery"] = roundedNumber / 100.0; //?不知為何 _E2JS(_BATTERY_VAL)還原會怪怪的
                param1["id"] = _E2JS(_MODULE_ID).as<uint16_t>();

                String output;
                serializeJson(doc, output);
                // serializeJsonPretty(doc, Serial);
                socketIO_Client.sendEVENT(output);
            }

            batterTimer = millis();
        }
        if (level != 97)
        {
            if (!digitalRead(pinCHG)||digitalRead(pinCHG_Invert))
            {
                const uint8_t level = 97;
                xQueueSend(queueWeaponLight, &level, portMAX_DELAY);
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "充電模式!\n");
            }
        }
        else if (digitalRead(pinCHG)&&!digitalRead(pinCHG_Invert))
        {
            const uint8_t level = 0;
            xQueueSend(queueWeaponLight, &level, portMAX_DELAY);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "結束充電模式!關閉LED\n");
        }
    }
}
/**
 * @brief 蜘蛛
 *
 * @param pvParam
 */
void taskSpider(void *pvParam)
{
    // 初始化
    const uint8_t pinOut[]{25, 26, 27, 33};
    for (size_t i = 0; i < sizeof(pinOut); i++)
    {
        pinMode(pinOut[i], OUTPUT);
        digitalWrite(pinOut[i], 0);
    }
    String StrJson = "";
    JsonDocument doc;
    while (1)
    {
        // 如果收到隊列
        if (xQueueReceive(queueJson, &StrJson, 0) == pdPASS)
        {
            // 反序列化隊列資料
            DeserializationError error = deserializeJson(doc, StrJson);
            if (error)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s\n", error.c_str());
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, StrJson);
            }
            else
            {
                // 如果包含value
                if (doc.containsKey("value"))
                {
                    // 變更馬達輸出狀態
                    if (doc["value"].as<int16_t>() > 0)
                    {
                        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "開始動作!");
                        for (size_t i = 0; i < sizeof(pinOut); i++)
                        {
                            digitalWrite(pinOut[i], 1);
                        }
                    }
                    else
                    {
                        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "結束動作!");

                        for (size_t i = 0; i < sizeof(pinOut); i++)
                        {
                            digitalWrite(pinOut[i], 0);
                        }
                    }
                }
            }
        }
        _DELAY_MS(100);
    }
}

/**
 * @brief 訊號延長器
 *
 */
void taskSignalExtender()
{
    const uint8_t pinOut = 13, pinIn = 15, pinLED = 2;
    pinMode(pinIn, INPUT_PULLUP);
    pinMode(pinOut, OUTPUT);
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinOut, 0);
    digitalWrite(pinLED, 0);
    uint8_t numCoin = 0;
    uint32_t numTimer = 0;
    uint16_t numWriteLong = 500;
    bool isCoinEnter = false;
    bool swOnOff = false;
    while (1)
    {
        bool statuses = digitalRead(pinIn);
        // 如果有訊號就+1並等待恢復
        if (!statuses && !isCoinEnter)
        {
            isCoinEnter = true;
            numCoin++;
        }
        else if (statuses && isCoinEnter)
        {
            isCoinEnter = false;
        }
        // 如果計數大於0就持續的ON/OFF固定毫秒數
        if (numCoin > 1)
        {
            // ON週期
            if (numTimer == 0 && !swOnOff)
            {
                swOnOff = true;
                numTimer = millis();
                digitalWrite(pinOut, 1);
                digitalWrite(pinLED, 1);
            } // OFF週期
            else if (numTimer > 0 && millis() > numTimer + numWriteLong && swOnOff)
            {
                swOnOff = false;
                numTimer = millis();
                digitalWrite(pinOut, 0);
                digitalWrite(pinLED, 0);
            } // 計數-1並重置
            else if (numTimer > 0 && millis() > numTimer + numWriteLong && !swOnOff)
            {
                numCoin--;
                numTimer = 0;
            }
        }
        _DELAY_MS(1);
    }
}

void taskIRController()
{
    const uint8_t pinReceiver = 34;
    IrSender.begin();     // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
    disableLEDFeedback(); // Disable feedback LED at default feedback LED pin
    // IrSender.sendNEC(0x00, 1, 1);
    IrReceiver.begin(pinReceiver, ENABLE_LED_FEEDBACK);
    printActiveIRProtocols(&Serial);
    String StrJson = "";
    JsonDocument doc;
    while (1)
    {
        // 如果收到隊列
        if (xQueueReceive(queueJson, &StrJson, 0) == pdPASS)
        {
            // 反序列化隊列資料
            DeserializationError error = deserializeJson(doc, StrJson);
            if (error)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s\n", error.c_str());
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, StrJson);
            }
            else
            {
                if (doc.containsKey("command") && doc.containsKey("address"))
                    IrSender.sendNEC(doc["address"].as<uint16_t>(), doc["command"].as<uint16_t>(), 1);
            }
        }

        if (IrReceiver.decode())
        {

            if (IrReceiver.decodedIRData.protocol == UNKNOWN)
            {
                Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
                // We have an unknown protocol here, print extended info
                IrReceiver.printIRResultRawFormatted(&Serial, true);
                IrReceiver.resume(); // Do it here, to preserve raw data for printing with printIRResultRawFormatted()
            }
            else
            {
                IrReceiver.resume(); // Early enable receiving of the next IR frame
                IrReceiver.printIRResultShort(&Serial);
                IrReceiver.printIRSendUsage(&Serial);
                IrSender.sendNEC(IrReceiver.decodedIRData.address, IrReceiver.decodedIRData.command, 1);
            }
            Serial.println();
        }

        _DELAY_MS(50);
    }
}

/**
 * @brief
 *
 */
void taskLINE_POST()
{
    while (1)
    {
        String str = "";
        if (WiFi.status() == WL_CONNECTED)
        { // 檢查是否連接 WiFi
            if (xQueueReceive(queueLINE_POST, &str, 100) == pdPASS)
            {
                JsonDocument doc;
                DeserializationError error = deserializeJson(doc, str);
                if (error)
                {
                    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化失敗:%s\n", error.c_str());
                }
                else if (doc.containsKey("message"))
                {
                    HTTPClient http;
                    http.begin("https://notify-api.line.me/api/notify"); // LINE Notify API URL
                    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
                    http.addHeader("Authorization", "Bearer fnBwOwWj1fGtJNq8AzUxYGH0rOFI6gxCNynpjcTV2v6");

                    String postData = "message=" + doc["message"].as<String>();

                    int httpResponseCode = http.POST(postData);

                    if (httpResponseCode > 0)
                    {
                        String response = http.getString();
                        Serial.println("HTTP Response code: " + String(httpResponseCode));
                        Serial.println("Response: " + response);
                    }
                    else
                    {
                        Serial.println("Error on sending POST: " + String(httpResponseCode));
                    }

                    http.end(); // 釋放資源
                }
            }
        }
        else
        {
            Serial.println("WiFi Disconnected");
        }
        _DELAY_MS(1);
    }
}

/**
 * @brief 將數字格式輸出到74HC595
 *
 * @param newTime
 */
void set74HC595(String newTime)
{
    const uint8_t bitNumber[2][10]{{B11111100, B01100000, B11011010, B11110010, B01100110,
                                    B10110110, B10111110, B11100000, B11111110, B11110110},
                                   {B11111100, B00001100, B11011010, B10011110, B00101110,
                                    B10110110, B11110110, B00011100, B11111110, B10111110}};
    const uint8_t pinDS = 23, pinSH = 18, pinST = 4;
    String str = newTime;
    // SPI.beginTransaction(SPISettings(1000, LSBFIRST, SPI_MODE3));
    digitalWrite(pinST, LOW);
    for (int8_t i = sizeof(str) - 1, j = i; i >= 0; i--, j--)
    {
        if (str[i] == ':' || str[i] == '.' ? 1 : 0)
            continue;
        uint8_t data = bitNumber[i < 2 ? 0 : 1][str[i] - '0'];
        SPI.transfer(data + (i > 0 && (str[i + 1] == ':' || str[i + 1] == '.' || str[i - 1] == ':' || str[i - 1] == '.') ? 1 : 0));
        // SPI.transfer(1 << i);
    }
    digitalWrite(pinST, HIGH);
    //    SPI.endTransaction();
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "剩餘時間: %s\n", str.c_str());
}
/**
 * @brief MP3模組的細節
 *
 * @param type
 * @param value
 */
void printDetail(uint8_t type, int value)
{
    switch (type)
    {
    case TimeOut:
        Serial.println(F("Time Out!"));
        break;
    case WrongStack:
        Serial.println(F("Stack Wrong!"));
        break;
    case DFPlayerCardInserted:
        Serial.println(F("Card Inserted!"));
        break;
    case DFPlayerCardRemoved:
        Serial.println(F("Card Removed!"));
        break;
    case DFPlayerCardOnline:
        Serial.println(F("Card Online!"));
        break;
    case DFPlayerUSBInserted:
        Serial.println("USB Inserted!");
        break;
    case DFPlayerUSBRemoved:
        Serial.println("USB Removed!");
        break;
    case DFPlayerPlayFinished:
        Serial.print(F("Number:"));
        Serial.print(value);
        Serial.println(F(" Play Finished!"));
        break;
    case DFPlayerError:
        Serial.print(F("DFPlayerError:"));
        switch (value)
        {
        case Busy:
            Serial.println(F("Card not found"));
            break;
        case Sleeping:
            Serial.println(F("Sleeping"));
            break;
        case SerialWrongStack:
            Serial.println(F("Get Wrong Stack"));
            break;
        case CheckSumNotMatch:
            Serial.println(F("Check Sum Not Match"));
            break;
        case FileIndexOut:
            Serial.println(F("File Index Out of Bound"));
            break;
        case FileMismatch:
            Serial.println(F("Cannot Find File"));
            break;
        case Advertise:
            Serial.println(F("In Advertise"));
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

uint16_t CRC_16(byte *data, uint16_t len, CRC16_parameter_t *CRC16_parameter)
{
    uint16_t val = CRC16_parameter->Initialvalue;
    uint16_t Polynomial = 0;
    if (CRC16_parameter->Inputinversion)
    {
        for (uint16_t i = 0x8000, j = 1; i != 0; i >>= 1, j <<= 1)
            CRC16_parameter->Polynomial &j ? Polynomial += i : Polynomial += 0;
    }
    else
        Polynomial = CRC16_parameter->Polynomial;
    while (len--)
    {
        val ^= *data++;
        for (byte j = 0; j < 8; j++)
        {
            if (CRC16_parameter->Outputinversion)
                val & 0x01 ? val = (val >> 1) ^ Polynomial : val = val >> 1;

            else
                val & 0x80 ? val = (val << 1) ^ Polynomial : val = val << 1;
        }
    }

    return val ^ CRC16_parameter->XORvalue;
}