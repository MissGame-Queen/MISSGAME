#include "task.h"
QueueHandle_t queueBasketball = xQueueCreate(1, sizeof(String));
QueueHandle_t queuePCM5102 = xQueueCreate(1, sizeof(String));
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
    const uint8_t pinLock = 14;
    pinMode(pinST, OUTPUT);
    set74HC595("00");
    for (size_t i = 0; i < sizeof(pinIn); i++)
    {
        pinMode(pinIn[i], INPUT);
    }
    pinMode(pinLock, OUTPUT);
    digitalWrite(pinLock, HIGH);

    Adafruit_NeoPixel strip(2, pinLED, NEO_RGB + NEO_KHZ800);

    uint8_t status = 0;
    uint32_t timer = 0;
    bool first = true;
    uint8_t value[] = {0, 0};
    JsonDocument doc;
    String strCMD = "";
    uint8_t fraction = 30;
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

        if (xQueueReceive(queueBasketball, &strCMD, 0) == pdPASS)
        {
            DeserializationError error = deserializeJson(doc, strCMD);
            if (error)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化失敗:%s\n", error.c_str());
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, strCMD);
            }
            else
            {
                if (doc.containsKey("status"))
                {
                    status = doc["status"].as<uint8_t>();
                    first = true;
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
            digitalWrite(pinLock, HIGH);
            fraction = 30;
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
            // 待機動畫
            static uint8_t valStyle = 0;
            static uint8_t valStyle2 = 2;
            static uint32_t timerStyle2 = 0;

            // 如果開機時間大於上次紀錄的時間150ms
            if (millis() > timer + 150)
            {
                timer = millis();
                valStyle++;
                doStyle1(valStyle);
            }
            if (millis() > timerStyle2 + 1000)
            {
                timerStyle2 = millis();
                valStyle2++;
            }
            doStyle2(&strip, valStyle2, timerStyle2);
            if (valStyle > 5)
            {
                valStyle = 0;
            }
            if (valStyle2 > 6)
            {
                valStyle2 = 2;
            }
            Serial.println("i");

            // 如果按鈕按下
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
                strCMD = "{\"value\": 1}";
                xQueueSend(queuePCM5102, &strCMD, portMAX_DELAY);
                first = false;
                timer = millis() - 600;
                // 開啟廣播燈
                for (uint8_t i = 0; i < strip.numPixels(); i++)
                {
                    strip.setPixelColor(i, i == 0 ? 255 : 0, 0, 0);
                }
                strip.show();
            }
            static int8_t num = 3;
            if (millis() > timer + 1000)
            {
                String str = "0" + String(num);
                str = str.substring(str.length() - 2);
                set74HC595(str);
                if (num > 0)
                {
                    num--;
                    timer = millis();
                }
                else
                {
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
            static bool sw = false;
            static bool isLimit = false;
            static uint32_t trmerLimit = 0;
            static uint32_t trmerValue = 0;
            // 播放開始語音
            if (first)
            {
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "進入開始狀態!");
                first = false;
                // strCMD = "{\"name\": \"/Start.mp3\"}";
                // xQueueSend(queuePCM5102, &strCMD, portMAX_DELAY);
            }
            // HACK
            // 如果持續ON0.1S輸出踩線語音
            if (digitalRead(pinIn[2]) && trmerLimit != 0 && millis() > trmerLimit + 100 && !isLimit && false)
            {
                strCMD = "{\"name\": \"/Limit.mp3\"}";
                xQueueSend(queuePCM5102, &strCMD, portMAX_DELAY);
                isLimit = true;
                set74HC595("--");
            }
            else if (digitalRead(pinIn[2]))
            {
                trmerLimit = millis();
            }

            else if (!digitalRead(pinIn[2]))
            {
                isLimit = false;
                trmerLimit = 0;
            }
            // 如果投進去則輸出語音
            // 每秒更新字數顯示器
            if (millis() > timer + 1000)
            {
                if (!isLimit)
                {
                    String str = "0" + String(fraction);
                    str = str.substring(str.length() - 2);
                    set74HC595(str);
                }
                fraction--;
                timer = millis();
                //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "剩餘時間: %s,目前分數:%d\n", str.c_str(), value[0]);
            }
            // 如果感測器ON且和上次間隔超過0.5S且沒踩線
            if (!digitalRead(pinIn[1]) && !sw && millis() > trmerValue + 500 && !isLimit)
                sw = true;
            else if (digitalRead(pinIn[1]) && sw)
            {
                trmerValue = millis();
                sw = false;
                value[0]++;
                strCMD = "{\"name\": \"/Score.mp3\"}";
                xQueueSend(queuePCM5102, &strCMD, portMAX_DELAY);
            }

            // 分數變更才刷新燈
            if (value[0] != value[1])
            {
                value[1] = value[0];
                for (uint8_t i = 0; i < strip.numPixels(); i++)
                {
                    if ((i + 1) <= value[0])
                        strip.setPixelColor(i,
                                            (i * 3 - 1 < value[0]) || i == 0 ? 255 : 0,
                                            i * 3 + 0 < value[0] ? 255 : 0,
                                            i * 3 + 1 < value[0] ? 255 : 0);
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

                value[0] = 0;
                value[1] = 0;
                first = true;
                timer = millis();
            }
        }
        break;
        case Finish:
        {
            // audio.connecttospeech("恭喜過關", "zh-tw");
            if (first)
            {
                strCMD = "{\"name\": \"/Finish.mp3\",\"value\": 3}";
                xQueueSend(queuePCM5102, &strCMD, portMAX_DELAY);
                digitalWrite(pinLock, LOW);

                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "成功!json:%s\n", strCMD.c_str());
                first = false;
            }
            String str = "0" + String(fraction);
            str = str.substring(str.length() - 2);
            set74HC595(str);

            strip.setPixelColor(0, 0, 255, 255);
            strip.setPixelColor(1, 255, 255, 255);
            strip.show();
            _DELAY_MS(400);
            set74HC595("  ");
            strip.setPixelColor(0, 0, 0, 0);
            strip.setPixelColor(1, 0, 0, 0);
            strip.show();
            _DELAY_MS(400);


            if (millis() > timer + 8000)
            {
                status = Reset;
            }
        }
        break;
        case Fail:
        {
            // audio.connecttospeech("是會不會,山石秒投進五顆球而已", "zh-tw");
            if (first)
            {
                strCMD = "{\"name\": \"/Fail.mp3\",\"value\": 2}";
                xQueueSend(queuePCM5102, &strCMD, portMAX_DELAY);
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "失敗!\n");
                first = false;
                _DELAY_MS(1000);
                status = Reset;
            }
        }
        break;

        default:
            break;
        }
        _DELAY_MS(2);
    }
}
void FlyingShip(void *pvParam)
{
    Adafruit_MCP23X17 mcp;
    if (!mcp.begin_I2C())
    {
        Serial.println("Error.");
        while (1)
            ;
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        mcp.pinMode(i + 8, INPUT_PULLUP);
        mcp.pinMode(i, OUTPUT);
    }
    enum status_e
    {
        Reset,
        Play,
        End,
    };
    const uint8_t ansValue[]{5, 4, 3, 2, 1, 7, 13, 19, 20, 21, 15, 9, 8, 14, 20, 26, 27, 28};
    const uint8_t checkValue[]{7, 9, 17};
    uint8_t value[sizeof(ansValue)];
    uint8_t status = Reset;
    uint8_t index = 0;
    uint32_t timer = 0;
    String strAudio = "";
    Adafruit_NeoPixel strip(10, 23, NEO_GRB + NEO_KHZ800);
    const uint8_t ONLOCK = B00100000;
    const uint8_t OFFLOCK = B00000000;
    bool ifLock = true;
    while (1)
        ;
    {
        switch (status)
        {
        case Reset:
            for (uint8_t i = 0; i < sizeof(value); i++)
            {
                value[i] = 0;
            }
            index = 0;
            ifLock = true;
            status++;
            break;
        case Play:
            static uint8_t j = 0;
            {
                mcp.writeGPIOA((1 << j) + (ifLock ? ONLOCK : OFFLOCK));
                _DELAY_MS(10);
                uint8_t data = mcp.readGPIOB() ^ 0xFF;
                // 每個port讀取
                for (uint8_t i = 0; i < 6; i++)
                { // 如果檢測到磁簧
                    if (data & (1 << i))
                    {
                        uint8_t val = j * 6 + i;
                        static uint8_t valLast = 0xFF;
                        // 如果和上次位置不一樣
                        if (valLast != val)
                        {
                            valLast = val;
                            value[index] = val;
                            // 如果放對位置
                            if (ansValue[index] == val)
                            {
                                bool checksw = false;
                                for (uint8_t iCheck = 0; iCheck < sizeof(checkValue); iCheck++)
                                {
                                    if (checkValue[iCheck] == index)
                                    {
                                        if (iCheck == sizeof(checkValue) - 1)
                                            status++;
                                        checksw = true;
                                        break;
                                    }
                                }
                                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "正確!%d\n", val);
                                if (checksw)
                                {
                                    strAudio = "{\"name\": \"/Correct.mp3\"}";
                                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                                }
                                else
                                {
                                    strAudio = "{\"name\": \"/Touch.mp3\"}";
                                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                                }
                                index++;
                            }
                            else
                            {
                                index = 0;
                                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "錯誤!%d\n", +val);
                                strAudio = "{\"name\": \"/Mistake.mp3\"}";
                                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                            }
                        }
                    }
                }
                j++;
                if (j >= 5)
                    j = 0;
            }
            break;
        case End:
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "成功!\n");
            strAudio = "{\"name\": \"/Correct.mp3\"}";
            xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
            ifLock = false;
            mcp.writeGPIOA((ifLock ? ONLOCK : OFFLOCK));
            _DELAY_MS(5000);
            status = Reset;
            break;
        default:
            break;
        }

        _DELAY_MS(20);
    }
}

void taskPCM5102(void *pvParam)
{
    DFRobotDFPlayerMini myDFPlayer;
    uint8_t SoundPlayerLevel[2] = {0, 0};
    Serial2.begin(9600);
    bool state = false;
    state = myDFPlayer.begin(Serial2, /*isACK = */ true, /*doReset = */ true);

    if (state)
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 已連線\n");
        myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
        myDFPlayer.volume(30);
    }
    else
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 連線失敗!\n1.請重新檢查連線！\n2.請插入SD卡！\n");
    if (!SD.begin(5))
        Serial.println("SD卡發生錯誤");
    else
        Serial.println("SD卡正常!!");

    // Audio *audio = (Audio *)pvParam;
    Audio *audio = new Audio(false, I2S_DAC_CHANNEL_DISABLE, I2S_NUM_0);
    audio->setPinout(pinBCLK, pinLRC, pinDOUT);
    audio->setVolume(21); // 0...21

    String StrJson = "";
    JsonDocument doc;
    while (1)
    {
        if (xQueueReceive(queuePCM5102, &StrJson, 0) == pdPASS)
        {
            DeserializationError error = deserializeJson(doc, StrJson);
            if (error)
            {
                //_CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化失敗:%s\n", error.c_str());
                //_CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, StrJson);
            }
            else
            {
                if (doc.containsKey("name"))
                {
                    if (doc["name"] == "")
                    {
                        SoundPlayerLevel[0] = 0;
                        audio->connecttoFS(SD, "/SYSTEM/stop.mp3");
                        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "停止音樂!%d\n", SoundPlayerLevel[0]);
                    }
                    else
                    {
                        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放音樂!%s\n", doc["name"].as<String>().c_str());

                        audio->connecttoFS(SD, doc["name"].as<String>().c_str());
                        audio->loop();
                    }
                }
                if (doc.containsKey("value") && state)
                {
                    myDFPlayer.disableLoop();
                    myDFPlayer.stop();
                    uint16_t mp3Value = doc["value"].as<uint16_t>();
                    if (mp3Value != 0)
                    {
                        myDFPlayer.play(mp3Value);
                        // myDFPlayer.enableLoop();
                    }
                    if (myDFPlayer.available())
                    {
                        // printDetail(myDFPlayer.readType(), myDFPlayer.read()); // Print the detail message from DFPlayer to handle different errors and states.
                    }
                }
            }
        }
        if (SoundPlayerLevel[0] != 0)
        {
            if (!audio->isRunning())
            {
                SoundPlayerLevel[0] = 0;
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放結束!等級歸0\n");
            }
        }
        audio->loop();
        vTaskDelay(2);
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
    String str = newTime;
    SPI.beginTransaction(SPISettings(1000000, LSBFIRST, SPI_MODE3));
    digitalWrite(pinST, LOW);
    for (int8_t i = sizeof(str) - 1, j = i; i >= 0; i--, j--)
    {
        if (str[i] == ':' || str[i] == '.' ? 1 : 0)
            continue;
        uint8_t data = bitNumber[i < 2 ? 0 : 1][str[i] - '0'];
        if (str[i] == '-')
            data = B00000010;
        if (str[i] == ' ')
            data = B00000000;
        SPI.transfer(data + (i > 0 && (str[i + 1] == ':' || str[i + 1] == '.' || str[i - 1] == ':' || str[i - 1] == '.') ? 1 : 0));
    }
    digitalWrite(pinST, HIGH);
    SPI.endTransaction();
}
void doStyle1(int value)
{ //           圓點點       左下       下面槓      右下        右上       上面的      左上      中間槓槓
    byte arr[8]{B00000001, B00001000, B00010000, B00100000, B01000000, B10000000, B00000100, B00000010};
    const uint8_t pinDS = 23, pinSH = 18, pinST = 4;
    SPI.beginTransaction(SPISettings(1000, LSBFIRST, SPI_MODE3));
    digitalWrite(pinST, LOW);

    /*寫在這裡
     */

    SPI.transfer(arr[value]);
    SPI.transfer(arr[value]);
    digitalWrite(pinST, HIGH);
    SPI.endTransaction();
}
void doStyle2(Adafruit_NeoPixel *strip, int value, uint16_t val)
{
    val = millis() - val;
    for (size_t j = 0; j < 2; j++)
    {
        uint8_t newval = 0;
        // 漸亮階段
        if (val < 500)
        {
            newval = map(val, 0, 500, 0, 255);
        }
        // 漸暗階段
        else
        {
            newval = map(val, 501, 1000, 255, 0);
        }
        strip->setPixelColor(0, value == 1 ? newval : 0, value == 2 ? newval : 0, value == 3 ? newval : 0);
        strip->setPixelColor(1, value == 4 ? newval : 0, value == 5 ? newval : 0, value == 6 ? newval : 0);
        strip->show();
    }
}
