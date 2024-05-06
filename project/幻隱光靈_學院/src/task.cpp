#include "task.h"
Servo myServo; //   Create Servo object to control the servo
               // QueueHandle_t queueTimer = xQueueCreate(1, sizeof(uint16_t));
String Timer_newSecond = "00:00";
uint16_t Timer_status = 0;
JsonDocument docWeaponLight;
tm tmTimer;
DFRobotDFPlayerMini myDFPlayer[2];
int16_t BallTime = 0;
uint16_t SoundPlayerLevel[2] = {0, 0};

Stepper myStepper(2000, 13, 12, 14, 27);

/**
 * @brief 出幣機程式
 *
 */
void CoinDispenser(uint16_t time)
{
    const uint8_t pinServo = 13, pinLED = 4, pinTrigger = 15;
    const uint16_t waittim = 600;
    const int16_t startdeg = _E2JS(_SEVER_DEG_START).as<uint8_t>();
    const int16_t enddeg = _E2JS(_SEVER_DEG_END).as<uint8_t>();
    /*
    if (time > 0)
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被函數觸發了~");
        for (uint16_t i = 0; i < time; i++)
        {
            digitalWrite(pinLED, 0);
            myServo.write(enddeg);
            _DELAY_MS(waittim);
            digitalWrite(pinLED, 1);
            myServo.write(startdeg);
            _DELAY_MS(waittim);
        }
        return;
    }
    */
    myServo.attach(pinServo); //   Servo is connected to digital pin 9
    myServo.write(startdeg);
    pinMode(pinLED, OUTPUT);
    pinMode(pinTrigger, INPUT_PULLUP);
    while (1)
    {

        bool isTrigger = digitalRead(pinTrigger);
        if (!isTrigger)
        {
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被實體按鈕觸發了~");
            BallTime++;
            Serial.println(BallTime);
        }
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
    Serial1.begin(115200, SERIAL_8N1, 22, 21);

    pinMode(13, INPUT_PULLUP);
    while (1)
    {
        if (!digitalRead(13))
            BallTime++;
        if (BallTime > 0)
        {
            bool dir = 0;
            uint16_t speed = 20;
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
            BallTime = 0;
            _DELAY_MS(dalayTime);
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
 * @brief 音效播放程式
 *0=初始化，1~9999=player1，10001~19999=player2
 */
void SoundPlayer(uint16_t playerNumber)
{
    static bool state[2] = {false, false};

    if (playerNumber == 0)
    {
        Serial1.begin(9600, SERIAL_8N1, 18, 5);
        Serial2.begin(9600);
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "正在初始化 DFPlayer ...（可能需要 3~5 秒）");
        for (uint8_t i = 0; i < sizeof(myDFPlayer) / sizeof(myDFPlayer[0]); i++)
        {
            if (i == 0)
                state[i] = myDFPlayer[i].begin(Serial2, /*isACK = */ true, /*doReset = */ true);
            else if (i == 1)
                state[i] = myDFPlayer[i].begin(Serial1, /*isACK = */ true, /*doReset = */ true);
            if (state[i])
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini[%d] 已連線.\n", i);
                myDFPlayer[i].setTimeOut(500); // Set serial communictaion time out 500ms
                                               /*
                                                  //----Set volume----
                                                  myDFPlayer[i].volume(10);   // Set volume value (0~30).
                                                  myDFPlayer[i].volumeUp();   // Volume Up
                                                  myDFPlayer[i].volumeDown(); // Volume Down
                               
                                                  //----Set different EQ----
                                                  myDFPlayer[i].EQ(DFPLAYER_EQ_NORMAL);
                               
                                                  //  myDFPlayer[i].EQ(DFPLAYER_EQ_POP);
                                                  //  myDFPlayer[i].EQ(DFPLAYER_EQ_ROCK);
                                                  //  myDFPlayer[i].EQ(DFPLAYER_EQ_JAZZ);
                                                  //  myDFPlayer[i].EQ(DFPLAYER_EQ_CLASSIC);
                                                  //  myDFPlayer[i].EQ(DFPLAYER_EQ_BASS);
                //----Set device we use SD as default----
                                              */
                myDFPlayer[i].outputDevice(DFPLAYER_DEVICE_SD);
                /*
                //  myDFPlayer[i].outputDevice(DFPLAYER_DEVICE_U_DISK);
                //  myDFPlayer[i].outputDevice(DFPLAYER_DEVICE_AUX);
                //  myDFPlayer[i].outputDevice(DFPLAYER_DEVICE_SLEEP);
                //  myDFPlayer[i].outputDevice(DFPLAYER_DEVICE_FLASH);

                //----Mp3 control----
                //  myDFPlayer[i].sleep();     //sleep
                //  myDFPlayer[i].reset();     //Reset the module
                //  myDFPlayer[i].enableDAC();  //Enable On-chip DAC
                //  myDFPlayer[i].disableDAC();  //Disable On-chip DAC
                //  myDFPlayer[i].outputSetting(true, 15); //output setting, enable the output and set the gain to 15

                //----Mp3 play----

                // myDFPlayer[i].playMp3Folder(4); // play specific mp3 in SD:/MP3/0004.mp3; File Name(0~65535)
                //----Read imformation----

                CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "State=%d\nVolume=%d\nEQ=%d\n=%d\nCurrentFileNumber=%d\nFileCountsInFolder=%d\n",
                                myDFPlayer[i].readState(), myDFPlayer[i].readVolume(), myDFPlayer[i].readEQ(), myDFPlayer[i].readFileCounts(),
                                myDFPlayer[i].readCurrentFileNumber(), myDFPlayer[i].readFileCountsInFolder(3));
                                */

                myDFPlayer[i].disableLoop();
                // myDFPlayer[i].enableLoop();
                myDFPlayer[i].volume(30);
            }
            else
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini[%d] 連線失敗!\n1.請重新檢查連線！\n2.請插入SD卡！\n", i);
            }
        }
    }
    else
    {
        /*
        if (args[1].containsKey("level"))
        {
            if (args[1]["level"].as<uint16_t>() > SoundPlayerLevel[value >= 10000 ? 1 : 0])
            {
                SoundPlayerLevel[value >= 10000 ? 1 : 0] = args[1]["level"].as<uint16_t>();
                SoundPlayer(value);
            }
        }
        else if (SoundPlayerLevel[value >= 10000 ? 1 : 0] == 0)
        {
            SoundPlayer(value);
        }
*/
        if (playerNumber > 10000 && state[1])
        {
            playerNumber -= 10000;
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "模組1播放:%d\n", playerNumber);
            myDFPlayer[1].play(playerNumber);
        }
        else
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "模組0播放:%d\n", playerNumber);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "playerNumber=%d,state[1]=%d\n", playerNumber, state[1]);
            myDFPlayer[0].play(playerNumber);
        }
    }
}
/**
 * @brief 計時器程式
 *
 */
void taskTimer(void *pvParam)
{
    // uint16_t second = 0;
    String nowsecond = "00:00";
    uint8_t status[2] = {0, 0};
    const uint8_t bitNumber[2][10]{{B11111100, B01100000, B11011010, B11110010, B01100110,
                                    B10110110, B10111110, B11100000, B11111110, B11110110},
                                   {B11111100, B00001100, B11011010, B10011110, B00101110,
                                    B10110110, B11110110, B00011100, B11111110, B10111110}};
    const uint8_t pinDS = 13, pinSH = 12, pinST = 14;
    const uint8_t pinOut[2]{27, 26};
    const uint8_t pinin[3]{18, 19, 21};
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
    pinMode(pinin[0], INPUT_PULLUP);
    pinMode(pinin[1], INPUT_PULLUP);
    pinMode(pinin[2], INPUT_PULLUP);

    Timer_status = TEST;
    time_t time = 0;
    tmTimer = *localtime(&time);
    /*
    while (xQueueReceive(queueTimer, &second, 100) != pdPASS)
        _DELAY_MS(1000);
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已收到秒數!%d\n", second);
    nowsecond = second;
    status[i] = RUN;
    */
    while (1)
    {
        status[0] = Timer_status;
        // 如果狀態改變或處於倒數狀態
        if (status[0] != status[1] || status[0] == RUN)
        {
            status[1] = status[0];
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "狀態為:%d\n", status[0]);
            switch (status[0])
            {
            case STOP:
                digitalWrite(pinOut[0], 1);
                digitalWrite(pinOut[1], 0);
                nowsecond = Timer_newSecond;
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
                if (millis() % 2000 < 1000)
                    sprintf(buffer, "%02d:%02d", tmTimer.tm_min, tmTimer.tm_sec);
                else
                    sprintf(buffer, "%02d%02d", tmTimer.tm_min, tmTimer.tm_sec);
                nowsecond = buffer;
                set74HC595(nowsecond);
                // 將 tm 轉換為 time_t 型別
                time_t time = mktime(&tmTimer);
                if (time != 0)
                {
                    // 扣 1 秒
                    time--;
                    // 將 time 轉換為 tm 型別
                    tmTimer = *localtime(&time);
                }
                else
                    status[0] = END;
            }
            break;
            case TEST:
                for (byte i = 0; i < 10; i++)
                {
                    set74HC595(String(i) + String(i) + (i % 2 == 0 ? ":" : "") + String(i) + String(i));
                    delay(500);
                }
                break;
            }
        }
        _DELAY_MS(200);
    }
}
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

    JsonDocument *otrdoc = (JsonDocument *)pvParam;
    docWeaponLight = *otrdoc;
    // delete otrdoc;
    //  JsonObject *prtObj = (JsonObject *)pvParam;
    const uint8_t pinOut[]{25, 26, 27, 33};

    Adafruit_NeoPixel strip(docWeaponLight.containsKey("Length") ? docWeaponLight["Length"] : 11,
                            docWeaponLight.containsKey("Pin") ? docWeaponLight["Pin"] : 15,
                            NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.show();
    /*
    for (size_t i = 0; i < sizeof(pinOut); i++)
    {
        ledcSetup(i, 100 , 12);
        ledcAttachPin(pinOut[i], i);
        ledcWrite(i, 0);
    }
    */
    uint16_t id = _E2JS(_MODULE_ID).as<uint16_t>();
    uint32_t color = 0;
    uint8_t num = 0;
    uint32_t batterTimer = 0;
    const uint16_t setCycle = 10000;
    while (1)
    {
        switch (id)
        {
            /*
     30~39 杖
     40~49 矛
     50~59 錘
     60~69 鞭
     70~79 劍
     80~89 斧
     */
        case 30 ... 89:
        {
            uint16_t limitPWM[] = {600, 600, 600};

            switch (id)
            {
            case 40 ... 49:
                limitPWM[0] = 600;
                limitPWM[1] = 600;
                limitPWM[2] = 600;
            case 50 ... 59:
                limitPWM[0] = 500;
                limitPWM[1] = 500; // 1500
                break;
            case 60 ... 69:
                limitPWM[0] = 2500;
                limitPWM[1] = 2500;
                limitPWM[2] = 2500;
                break;
            case 70 ... 79:
                limitPWM[0] = 2500;
                limitPWM[1] = 2200;
                limitPWM[2] = 2500;
                break;
            case 80 ... 89:
                limitPWM[0] = 800;
                limitPWM[1] = 800;
                limitPWM[2] = 700;
                break;
            }

            //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "等級為%d\n", docWeaponLight["Level"].as<uint8_t>());
            switch (docWeaponLight["Level"].as<uint8_t>())
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
            case 99:
            {
                switch (num)
                { // 20=1.8 2000=2.4
                case 0 ... 20:
                    ledcWrite(0, limitPWM[0]);
                    ledcWrite(1, 0);
                    ledcWrite(2, 0);
                    ledcWrite(3, 0);
                    break;
                case 21 ... 40:
                    ledcWrite(0, limitPWM[0]);
                    ledcWrite(1, limitPWM[1]);
                    ledcWrite(2, 0);
                    ledcWrite(3, 0);
                    break;
                case 41 ... 60:
                    ledcWrite(0, limitPWM[0]);
                    ledcWrite(1, limitPWM[1]);
                    ledcWrite(2, limitPWM[2]);
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
                    color = setBrightnessRGB(color, 10);
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
            strip.show();
        }
        }
        _DELAY_MS(docWeaponLight["DelayTime"].as<uint16_t>());
        /**
         * @brief 每10秒送目前電池電壓值
         *
         */
        // _E2JS(_BATTERY_VAL) = roundf(analogRead(32) * 0.0017465437788018 * 100) / 100;
        if (millis() > batterTimer + setCycle)
        {
            float number = analogRead(32) * 0.0017465437788018; // 假设这是你的浮点数值
            int roundedNumber = round(number * 100);            // 将浮点数乘以100后四舍五入为整数
            _E2JS(_BATTERY_VAL) = roundedNumber / 100.0;        // 将四舍五入后的整数除以100得到保留两位小数的浮点数
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
            socketIO.sendEVENT(output);
            batterTimer = millis();
        }
    }
}
/**
 * @brief 訊號延長器
 *
 */
void taskSignalExtender()
{
    const uint8_t pinOut = 13, pinIn = 15;
    pinMode(pinIn, INPUT_PULLUP);
    pinMode(pinOut, OUTPUT);
    digitalWrite(pinOut, 0);
    uint8_t numCoin = 0;
    uint16_t numTimer = 0;
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
            } // OFF週期
            else if (numTimer > 0 && millis() > numTimer + numWriteLong && swOnOff)
            {
                swOnOff = false;
                numTimer = millis();
                digitalWrite(pinOut, 0);
            } // 計數-1並重置
            else if (numTimer > 0 && millis() > numTimer + numWriteLong && !swOnOff)
            {
                numCoin--;
                numTimer = 0;
            }
        }
        _DELAY_MS(10);
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
    const uint8_t pinDS = 13, pinSH = 12, pinST = 14;
    String str = newTime;
    // SPI.beginTransaction(SPISettings(1000, LSBFIRST, SPI_MODE3));
    digitalWrite(pinST, LOW);
    for (int8_t i = sizeof(str) - 1, j = i; i >= 0; i--, j--)
    {
        if (str[i] == ':' || str[i] == '.' ? 1 : 0)
            continue;
        uint8_t data = bitNumber[i < 2 ? 0 : 1][str[i] - '0'];
        myshiftOut(pinDS, pinSH, LSBFIRST, data + (i > 0 && (str[i + 1] == ':' || str[i + 1] == '.' || str[i - 1] == ':' || str[i - 1] == '.') ? 1 : 0));
        // SPI.transfer(data + (i > 0 && (str[i + 1] == ':' || str[i + 1] == '.' || str[i - 1] == ':' || str[i - 1] == '.') ? 1 : 0));
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

void myshiftOut(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (bitOrder == LSBFIRST)
            digitalWrite(dataPin, !!(val & (1 << i)));
        else
            digitalWrite(dataPin, !!(val & (1 << (7 - i))));

        digitalWrite(clockPin, HIGH);
        _DELAY_MS(1);
        digitalWrite(clockPin, LOW);
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