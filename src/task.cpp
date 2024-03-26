#include "task.h"
Servo myServo; //   Create Servo object to control the servo
               // QueueHandle_t queueTimer = xQueueCreate(1, sizeof(uint16_t));
String Timer_newSecond = "00:00";
uint16_t Timer_status = 0;
JsonDocument docWeaponLight;
tm tmTimer;
DFRobotDFPlayerMini myDFPlayer;
/**
 * @brief 出幣機程式
 *
 */
void CoinDispenser(uint16_t time)
{
    const uint8_t pinServo = 13, pinLED = 4, pinTrigger = 15;
    const uint16_t waittim = 600;
    const int16_t startdeg = 105;
    const int16_t enddeg = 96;
    if (time > 0)
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被函數觸發了~");
        for (uint16_t i = 0; i < time; i++)
        {
            digitalWrite(pinLED, 0);
            myServo.write(startdeg);
            _DELAY_MS(waittim);
            digitalWrite(pinLED, 1);
            myServo.write(enddeg);
            _DELAY_MS(waittim);
        }
        return;
    }

    myServo.attach(pinServo); //   Servo is connected to digital pin 9
    myServo.write(startdeg);
    pinMode(pinLED, OUTPUT);
    pinMode(pinTrigger, INPUT_PULLUP);
    while (1)
    {
        while (!digitalRead(pinTrigger))
        {
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被實體按鈕觸發了~");
            digitalWrite(pinLED, 0);
            myServo.write(startdeg);
            _DELAY_MS(waittim);
            digitalWrite(pinLED, 1);
            myServo.write(enddeg);
            _DELAY_MS(waittim);
        }
        _DELAY_MS(50);
    }
}

/**
 * @brief 出球機程式
 *
 */
void BallDispenser(uint16_t time)
{
    /*
    const uint8_t pinStepMotor[]{19, 18, 5, 17};
    const uint8_t pinLED = 4;
    const uint8_t pinTrigger = 15;
    const uint16_t waittim = 500;
    const uint8_t stepValue[]{B1110, B1010, B1011, B1001, B1101, B0101, B0111, B0110};
    if (time > 0)
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被觸發了~");
        for (uint16_t i = 0; i < time; i++)
        {
            int16_t d = 103;
            digitalWrite(pinLED, 0);
            myServo.write(d); //   Rotate servo  clockwise
            // Serial.println(d);
            _DELAY_MS(waittim);
            d -= 96;
            digitalWrite(pinLED, 1);
            myServo.write(d); //   Rotate servo  clockwise
            // Serial.println(d);
            _DELAY_MS(waittim);
        }
        return;
    }
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出球機模式~");

    pinMode(pinLED, OUTPUT);
    pinMode(pinTrigger, INPUT_PULLUP);
    for (size_t i = 0; i < sizeof(pinStepMotor); i++)
    {
        pinMode(pinStepMotor[i], OUTPUT);
        digitalWrite(pinStepMotor[i], 1);
    }
    while (1)
    {

        for (size_t j = 0; j < 400; j++)
        {
            for (size_t i = 0; i < sizeof(pinStepMotor); i++)
                digitalWrite(pinStepMotor[i], stepValue[j % 8] & (1 << i));
            //Serial.println(j);
            _DELAY_MS(5);
        }
        _DELAY_MS(5000);
    }
    */
    /*
        // Include the Arduino Stepper Library

        // Number of steps per output rotation
        const int stepsPerRevolution = 200;

        // Create Instance of Stepper library
        Stepper myStepper(200, 19, 18, 5, 17);

        // set the speed at 60 rpm:
        myStepper.setSpeed(60);
        // initialize the serial port:
        Serial.begin(115200);

        while (1)
        {
            // step one revolution in one direction:
            // Serial.println("clockwise");
            myStepper.step(200);
            delay(5000);
        }
        */

    const uint8_t pinServo = 13, pinLED = 4, pinTrigger = 15;
    const uint16_t waittim = 600;
    const int16_t startdeg = 90;
    const int16_t enddeg = 150;
    if (time > 0)
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出球機被函數觸發了~");
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

    myServo.attach(pinServo); //   Servo is connected to digital pin 9
    myServo.write(startdeg);
    pinMode(pinLED, OUTPUT);
    pinMode(pinTrigger, INPUT_PULLUP);
    while (1)
    {
        while (!digitalRead(pinTrigger))
        {
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出球機被實體按鈕觸發了~");

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
 * @brief 音效播放程式
 *
 */
void SoundPlayer()
{

    Serial2.begin(9600);

    Serial.begin(115200);

    Serial.println();
    Serial.println(F("正在初始化 DFPlayer ...（可能需要 3~5 秒）"));

    if (!myDFPlayer.begin(Serial2, /*isACK = */ true, /*doReset = */ true))
    { // Use serial to communicate with mp3.
        Serial.println(F("無法開始："));
        Serial.println(F("1.請重新檢查連線！"));
        Serial.println(F("2.請插入SD卡！"));
        while (true)
            ;
    }
    Serial.println(F("DFPlayer Mini online."));

    myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms

    //----Set volume----
    myDFPlayer.volume(10);   // Set volume value (0~30).
    myDFPlayer.volumeUp();   // Volume Up
    myDFPlayer.volumeDown(); // Volume Down

    //----Set different EQ----
    myDFPlayer.EQ(DFPLAYER_EQ_NORMAL);
    /*
    //  myDFPlayer.EQ(DFPLAYER_EQ_POP);
    //  myDFPlayer.EQ(DFPLAYER_EQ_ROCK);
    //  myDFPlayer.EQ(DFPLAYER_EQ_JAZZ);
    //  myDFPlayer.EQ(DFPLAYER_EQ_CLASSIC);
    //  myDFPlayer.EQ(DFPLAYER_EQ_BASS);
*/
    //----Set device we use SD as default----
    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    /*
    //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_U_DISK);
    //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_AUX);
    //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_SLEEP);
    //  myDFPlayer.outputDevice(DFPLAYER_DEVICE_FLASH);
*/
    //----Mp3 control----
    //  myDFPlayer.sleep();     //sleep
    //  myDFPlayer.reset();     //Reset the module
    //  myDFPlayer.enableDAC();  //Enable On-chip DAC
    //  myDFPlayer.disableDAC();  //Disable On-chip DAC
    //  myDFPlayer.outputSetting(true, 15); //output setting, enable the output and set the gain to 15

    //----Mp3 play----

    // myDFPlayer.playMp3Folder(4); // play specific mp3 in SD:/MP3/0004.mp3; File Name(0~65535)
    //----Read imformation----
    Serial.println(myDFPlayer.readState());               // read mp3 state
    Serial.println(myDFPlayer.readVolume());              // read current volume
    Serial.println(myDFPlayer.readEQ());                  // read EQ setting
    Serial.println(myDFPlayer.readFileCounts());          // read all file counts in SD card
    Serial.println(myDFPlayer.readCurrentFileNumber());   // read current play file number
    Serial.println(myDFPlayer.readFileCountsInFolder(3)); // read file counts in folder SD:/03

    while (1)
    {
        static unsigned long timer = millis();

        if (millis() - timer > 3000)
        {
            timer = millis();
            myDFPlayer.next(); // Play next mp3 every 3 second.
        }

        if (myDFPlayer.available())
        {
            printDetail(myDFPlayer.readType(), myDFPlayer.read()); // Print the detail message from DFPlayer to handle different errors and states.
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

    const uint8_t pinMO = 13, pinCLK = 12, pinCS = 14;
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
    pinMode(pinMO, OUTPUT);
    pinMode(pinCLK, OUTPUT);
    pinMode(pinCS, OUTPUT);
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
    status[0] = RUN;
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
void set74HC595(String newTime)
{
    const uint8_t bitNumber[2][10]{{B11111100, B01100000, B11011010, B11110010, B01100110,
                                    B10110110, B10111110, B11100000, B11111110, B11110110},
                                   {B11111100, B00001100, B11011010, B10011110, B00101110,
                                    B10110110, B11110110, B00011100, B11111110, B10111110}};
    const uint8_t pinMO = 13, pinCLK = 12, pinCS = 14;
    String str = newTime;
    digitalWrite(pinCS, LOW);
    for (int8_t i = sizeof(str) - 1, j = i; i >= 0; i--, j--)
    {
        if (str[i] == ':' || str[i] == '.' ? 1 : 0)
            continue;
        uint8_t data = bitNumber[i < 2 ? 0 : 1][str[i] - '0'];
        shiftOut(pinMO, pinCLK, LSBFIRST, data + ((str[i + 1] == ':' || str[i + 1] == '.' || str[i - 1] == ':' || str[i - 1] == '.') ? 1 : 0));
    }
    digitalWrite(pinCS, HIGH);
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
/**
 * @brief 武器燈
 *
 */
// FIXME不知為何會中斷while() 須提升優先級
/**
 * @brief
 *
 * @param pvParam
 */
void taskWeaponLight(void *pvParam)
{
    JsonDocument *otrdoc = (JsonDocument *)pvParam;
    docWeaponLight = *otrdoc;
    delete otrdoc;
    // JsonObject *prtObj = (JsonObject *)pvParam;
    const uint8_t pinOut[]{25, 26, 27, 33};

    Adafruit_NeoPixel strip(docWeaponLight.containsKey("Length") ? docWeaponLight["Length"] : 11,
                            docWeaponLight.containsKey("Pin") ? docWeaponLight["Pin"] : 15,
                            NEO_GRB + NEO_KHZ800);
    strip.begin();
    strip.show();
    for (size_t i = 0; i < sizeof(pinOut); i++)
    {
        ledcSetup(i, 100, 12);
        ledcAttachPin(pinOut[i], i);
        ledcWrite(i, 0);
    }
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
            uint16_t limitPWM[] = {1200, 1200, 1200};
            switch (id)
            {
            case 50 ... 59:
                limitPWM[0] = 500;
                limitPWM[1] = 2500;
                break;
            case 60 ... 69:
                limitPWM[0] = 2500;
                limitPWM[1] = 2500;
                limitPWM[2] = 2500;
                break;
            case 70 ... 79:
                limitPWM[0] = 1600;
                limitPWM[1] = 1800;
                limitPWM[2] = 2500;
                break;
            case 80 ... 89:
                limitPWM[0] = 800;
                limitPWM[1] = 10;
                limitPWM[2] = 700;
                break;
            }
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
                int16_t brightness = 255;
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
                int16_t brightness = 155;
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
                case 0 ... 10:
                    ledcWrite(0, limitPWM[0]);
                    ledcWrite(1, 0);
                    ledcWrite(2, 0);
                    ledcWrite(3, 0);
                    break;
                case 11 ... 20:
                    ledcWrite(0, limitPWM[0]);
                    ledcWrite(1, limitPWM[1]);
                    ledcWrite(2, 0);
                    ledcWrite(3, 0);
                    break;
                case 21 ... 30:
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
            JsonObject param1 = array.createNestedObject();
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
