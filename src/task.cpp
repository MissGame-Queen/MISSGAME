#include "task.h"
QueueHandle_t queueBasketball = xQueueCreate(1, sizeof(String));

const uint8_t pinDS = 23, pinSH = 18, pinST = 4;
const uint8_t pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26;
/**
 * @brief 籃球機
 *
 *
 * @param pvParam
 * "status":狀態
 * "value":分數
 */
void Basketball(void *pvParam)
{

    const uint8_t pinIn[]{36, 39, 34, 35};
    pinMode(pinST, OUTPUT);
    set74HC595("00");
    for (size_t i = 0; i < sizeof(pinIn); i++)
    {
        pinMode(pinIn[i], INPUT);
    }

    Adafruit_NeoPixel strip(2, pinLED, NEO_GRB + NEO_KHZ800);

    Audio audio;
    audio.setPinout(pinBCLK, pinLRC, pinDOUT);
    audio.setVolume(11); // 0...21
    uint8_t status = 0;
    uint32_t timer = 0;
    bool first = true;
    uint8_t value[] = {0, 0};
    JsonDocument doc;
    enum status_e
    {
        Reset,
        Standby,
        Ready,
        Start,
        Finish,
        Fail
    };
    /*HACK 測試人體感應器
     while(1){
         if(!digitalRead(pinIn[2]))
          set74HC595("88");
          else
          set74HC595("--");
          _DELAY_MS(10);
     }*/
    /*HACK 測試燈泡
    while (1)
    {
        for (uint8_t i = 0; i < 5; i++)
        {
            strip.setPixelColor(0, i == 0 ? 255 : 0, i == 1 ? 255 : 0, i == 2 ? 255 : 0);
            strip.setPixelColor(1, i == 3 ? 255 : 0, i == 4 ? 255 : 0, i == 5 ? 255 : 0);
            strip.show();
            _DELAY_MS(1000);
        }
    }
      */

    while (1)
    {
        static String strCMD = "";

        if (xQueueReceive(queueBasketball, &strCMD, 0) == pdPASS)
        {
            DeserializationError error = deserializeJson(doc, strCMD);
            if (error)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化失敗:%s\n", error.c_str());
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO,strCMD);
            }
            else
            {
                if (doc.containsKey("status"))
                {
                    status = doc["status"].as<uint8_t>();
                    first=true;
                }
                if (doc.containsKey("value"))
                {
                    value[0] = doc["value"].as<uint8_t>();
                }
            }
        }
        switch (status)
        {
        case Reset:
            set74HC595("00");
            for (uint8_t i = 0; i < strip.numPixels(); i++)
                strip.setPixelColor(i, 0);
            strip.show();
            status++;
            break;
            //[ ]等待按鈕按下並放開
        case Standby:
        {
            if (first)
            {
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "進入等待狀態!");
                first = false;
            }
            static bool sw = false;
            if (!digitalRead(pinIn[0]) && !sw)
                sw = true;
            else if (digitalRead(pinIn[0]) && sw)
            {
                status++;
                sw = false;
                timer = millis();
                first = true;
            }
        }
        break;

        case Ready:
        {
            if (first)
            {
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "進入準備狀態!");
                // audio.connecttospeech("玩家按下開始按鈕,倒數3秒開始", "zh-tw");
                audio.connecttoFS(SD, "/Ready.mp3");
                audio.loop();
                first = false;
            }
            static int8_t num = 3;
            if (millis() > timer + 1000)
            {
                if (num >= 0)
                {
                    for (uint8_t i = 0; i < strip.numPixels(); i++)
                    {
                        strip.setPixelColor(i, num == 1 ? 255 : 0, num == 3 ? 255 : 0, num == 2 ? 255 : 0);
                    }
                    strip.show();
                    String str = "0" + String(num);
                    str = str.substring(str.length() - 2);
                    set74HC595(str);
                    num--;
                    timer = millis();
                }
                else if (num < 0)
                {
                    for (uint8_t i = 0; i < strip.numPixels(); i++)
                        strip.setPixelColor(i, 0);
                    strip.show();
                    num = 3;
                    status++;
                    timer = millis();
                    first = true;
                }
            }
        }
        break;
        case Start:
        {
            static uint8_t fraction = 30;

            static bool sw = false;
            static bool isLimit = false;
            static uint32_t trmerLimit = 0;
            static uint32_t trmerValue = 0;
            // 播放開始語音
            if (first)
            {
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "進入開始狀態!");
                first = false;
                audio.connecttoFS(SD, "/Start.mp3");
                audio.loop();
            }
            // 如果超線則輸出語音
            if (digitalRead(pinIn[2]) && !audio.isRunning() && trmerLimit != 0 && millis() > trmerLimit + 100)
            {
                audio.connecttoFS(SD, "/Limit.mp3");
                audio.loop();
                isLimit = true;
                set74HC595("--");
            }
            else if (digitalRead(pinIn[2]) && !audio.isRunning())
            {
                trmerLimit = millis();
            }

            else if (!digitalRead(pinIn[2]))
            {
                isLimit = false;
                trmerLimit = 0;
            }
            // 如果投進去則輸出語音
            if (!isLimit)
            {
                // 每秒更新字數顯示器
                if (millis() > timer + 1000)
                {
                    String str = "0" + String(fraction);
                    str = str.substring(str.length() - 2);
                    set74HC595(str);
                    fraction--;
                    timer = millis();
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "剩餘時間: %s,目前分數:%d\n", str.c_str(), value[0]);
                }
                if (!digitalRead(pinIn[1]) && !sw && millis() > trmerValue + 1000)
                    sw = true;
                else if (digitalRead(pinIn[1]) && sw)
                {
                    trmerValue = millis();
                    sw = false;
                    value[0]++;
                    audio.connecttoFS(SD, "/Score.mp3");
                    audio.loop();
                }
            }
            // 分數變更才刷新燈
            if (value[0] != value[1])
            {
                value[1] = value[0];
                for (uint8_t i = 0; i < strip.numPixels(); i++)
                {
                    if ((i + 1) <= value[0])
                        strip.setPixelColor(i,
                                            i * 3 + 0 < value[0] ? 255 : 0,
                                            i * 3 + 1 < value[0] ? 255 : 0,
                                            i * 3 + 2 < value[0] ? 255 : 0);
                }
                strip.show();
            }
            // 如果達到結束條件
            if (fraction > 30 || value[0] >= 5)
            {
                if (value[0] >= 5)
                    status = Finish;
                else
                    status = Fail;

                fraction = 30;
                value[0] = 0;
                value[1] = 0;
                first = true;
            }
        }
        break;
        case Finish:
        {
            // audio.connecttospeech("恭喜過關", "zh-tw");
            if (first)
            {
                audio.connecttoFS(SD, "/Finish.mp3");
                audio.loop();
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "完成!\n");
                first = false;
            }
            if (!audio.isRunning())
                status = Reset;
        }
        break;
        case Fail:
        {
            // audio.connecttospeech("是會不會,山石秒投進五顆球而已", "zh-tw");
            if (first)
            {
                audio.connecttoFS(SD, "/Fail.mp3");
                audio.loop();
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "失敗!\n");
                first = false;
            }
            if (!audio.isRunning())
                status = Reset;
        }
        break;

        default:
            break;
        }
        audio.loop();
        _DELAY_MS(2);
    }
}
void FlyingShip(void *pvParam)
{
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
    SPI.beginTransaction(SPISettings(1000, LSBFIRST, SPI_MODE3));
    digitalWrite(pinST, LOW);
    for (int8_t i = sizeof(str) - 1, j = i; i >= 0; i--, j--)
    {
        if (str[i] == ':' || str[i] == '.' ? 1 : 0)
            continue;
        uint8_t data = bitNumber[i < 2 ? 0 : 1][str[i] - '0'];
        if (str[i] == '-')
            data = B00000010;
        SPI.transfer(data + (i > 0 && (str[i + 1] == ':' || str[i + 1] == '.' || str[i - 1] == ':' || str[i - 1] == '.') ? 1 : 0));
    }
    digitalWrite(pinST, HIGH);
    SPI.endTransaction();
}