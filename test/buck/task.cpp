#include "task.h"
/*
void task_Audio(void *pvParam)
{
    waitConnect();
    audio.setPinout(pinAudio_BCK, pinAudio_WS, pinAudio_DO);
    audio.setVolume(21); // 0...21
    // audio.connecttospeech("開機完成", "zh");
    while (true)
    {
        audio.loop();
        _DELAY_MS(1);
    }
}
*/
void task_LED(void *pvParam)
{

    strip.begin();
    for (uint8_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, 0);
    }
    strip.show();

    while (true)
    {
        switch (_E2JS(_LED_TYPE).as<uint16_t>())
        {
        default:
            for (uint8_t i = 0; i < strip.numPixels(); i++)
            {
                uint32_t cycle;
                switch (ledType[i])
                {
                case 0:
                case 0x80:
                    strip.setPixelColor(i, strip.Color(
                                               ledR[i],
                                               ledG[i],
                                               ledB[i]));
                    break;
                case 1:
                case 0x80 + 1:
                    cycle = ((millis() - ledTime[i]) % (ledON[i] + ledOFF[i]));
                    if (cycle > ledON[i])
                        strip.setPixelColor(i, 0);
                    else
                        strip.setPixelColor(i, strip.Color(
                                                   ledR[i],
                                                   ledG[i],
                                                   ledB[i]));
                    break;
                case 2:
                case 0x80 + 2:
                    float brightness = 0.0;
                    cycle = ((millis() - ledTime[i]) % (ledON[i] + ledOFF[i]));
                    if (cycle > ledON[i])
                        brightness = map(cycle,
                                         ledON[i], (ledON[i] + ledOFF[i]),
                                         255, 0);

                    else
                        brightness = map(cycle,
                                         0, (ledON[i]),
                                         0, 255);
                    brightness /= 255.0;
                    strip.setPixelColor(i, strip.Color(
                                               ledR[i] * brightness,
                                               ledG[i] * brightness,
                                               ledB[i] * brightness));
                    break;
                }
            }

            strip.show();
        }

        _DELAY_MS(10);
    }
}
void task_MQTT(void *pvParam)
{
    waitConnect(0);
    // 設定 MQTT 伺服器
    clientMQTT.setServer(_E2JS(MQTT_SERVER).as<const char *>(), _E2JS(MQTT_PORT).as<uint16_t>());
    // 設定回呼函式
    clientMQTT.setCallback(callback_MQTT);
    while (true)
    {
        // 保持連線
        reconnect_MQTT();

        // 監聽 MQTT 訊息
        clientMQTT.loop();
        _DELAY_MS(1);
        // 做其他的事情...
    }
}
void task_WebSocket(void *pvParam)
{
    using namespace websockets;
    WebsocketsServer server;
    waitConnect(0, 1);
    server.listen(81);
    WebsocketsClient client;
    while (true)
    {
        while (!client.available())
        {
            client = server.accept();
            _DELAY_MS(1000);
        }
        while (true)
        {
            /*
            WebsocketsMessage msg = client.readBlocking();
                        // log
                        Serial.print("Got Message: ");
                        Serial.println(msg.data());
                        // return echo
                        client.send("Echo: " + msg.data());
                        // close the connection
                        client.close();
                        */
            client.send(String(millis()));

            _DELAY_MS(1000);
        }
    }
}

void cat()
{
    enum emcpOutput
    {
        CATPAW_1,
        CATPAW_2,
        CATPAW_3,
        CATPAW_4,
        REMOTE_CONTROL_1,
        REMOTE_CONTROL_2,
        REMOTE_CONTROL_3,
        REMOTE_CONTROL_4,
        ROOM_LIGHT,
        FINAL_DOOR,
    };
    enum ePin_ESP32
    {
        ESP_LIGHT_R_PWM_CHANNEL = 0,
        ESP_LIGHT_G_PWM_CHANNEL = 1,
        ESP_LIGHT_B_PWM_CHANNEL = 2,
        ESP_LIGHT_R = 37,
        ESP_LIGHT_G = 14,
        ESP_LIGHT_B = 12,
        ESP_STRIP = 13,
        ESP_MIDDLE_DOOR_ON = 26,
        ESP_MIDDLE_DOOR_OFF = 27,
        ESP_SD_CS = 5,
        ESP_MCP_IN_CS = 33,
        ESP_MCP_OUT_CS = 32,
    };
    Adafruit_MCP23X08 mcpInput, mcpOutput;
    Adafruit_NeoPixel strip(24 * 15, ESP_STRIP, NEO_GRB + NEO_KHZ800);

    if (!mcpInput.begin_SPI(ESP_MCP_IN_CS) || !mcpOutput.begin_SPI(ESP_MCP_OUT_CS))
    {
        Serial.println("ERROR");
        while (1)
            ;
    }

    ledcSetup(ESP_LIGHT_R_PWM_CHANNEL, 5000, 8);
    ledcSetup(ESP_LIGHT_G_PWM_CHANNEL, 5000, 8);
    ledcSetup(ESP_LIGHT_B_PWM_CHANNEL, 5000, 8);
    ledcAttachPin(ESP_LIGHT_R, ESP_LIGHT_R_PWM_CHANNEL);
    ledcAttachPin(ESP_LIGHT_G, ESP_LIGHT_G_PWM_CHANNEL);
    ledcAttachPin(ESP_LIGHT_B, ESP_LIGHT_B_PWM_CHANNEL);
    for (int8_t i = 0; i < 16; i++)
    {
        mcpInput.pinMode(i, INPUT_PULLUP);

        if (i < 8)
        {
            mcpOutput.pinMode(i, INPUT_PULLUP);
        }
        else
        {
            mcpOutput.pinMode(i, OUTPUT);
            mcpOutput.digitalWrite(i, 0);
        }
    }
    // mcpOutput.digitalWrite(FINAL_DOOR, 1);
    strip.begin();                 // Initialize NeoPixel strip object (REQUIRED)
    strip.show();                  // Initialize all pixels to 'off'
    uint8_t step = 0;              // 步驟
    uint16_t inFlower[2] = {0, 0}; // 比對放入碎片用
    uint8_t nowState = 0;          // 目前回憶影片狀態
    uint8_t passwordState[8];      // 比對回憶順序用
    uint16_t numberTime = 0;
    while (1)
    {
        switch (step)
        {
        case 0:
            // 參數初始化
            /*
            for (int8_t i = 0; i < 12; i++)
            {
                mcpOutput.digitalWrite(i, 0);
                if (i < 8)
                    passwordState[i] = 0;
                if (i < 2)
                    inFlower[i] = 0;
            }
            nowState = 0;
            numberTime = 0;
            mcpOutput.digitalWrite(FINAL_DOOR, 1);
            ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 0);
            ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 0);
            ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 0);
            */
            // HACK
            while (1)
            {
                for (byte i = 0; i < 16; i++)
                {
                    if (!mcpInput.digitalRead(i))
                        Serial.println(i);
                }
                delay(1000);
            }
            step++;
            break;
        case 1:
            // 第一階段的記憶之花
            // 「第一首背景音樂」會一直循環撥放
            /*
            mcpOutput.digitalWrite(MUSIC_1, 1);
            // 玩家放下碎片的時候
            // 碎片後對應位置的燈片會亮黃燈，並且有「放碎片」音效(06)
            inFlower[0] = mcpInput.readGPIO(0) | (mcpInput.readGPIO(1) << 8);
            if (inFlower[0] != inFlower[1])
            {
                inFlower[1] = inFlower[0];
                mcpOutput.digitalWrite(SOUND_6, 1);
            }
*/
            /*
碎片如果拿走燈暗掉
碎片拿走後重覆再放可以無限次重複觸發上述效果
*/
            for (uint8_t i = 0; i < 15; i++)
            {
                if (!(inFlower[0] & (1 << i)))
                {
                    for (uint8_t j = 0; j < 24; j++)
                    {
                        strip.setPixelColor(i * 24 + j, strip.Color(255, 255, 0));
                    }
                }
                else
                {
                    for (uint8_t j = 0; j < 24; j++)
                    {
                        strip.setPixelColor(i * 24 + j, strip.Color(0, 0, 0));
                    }
                }
            }
            // 當15個碎片都放上以後約3秒進入第二階段
            if (inFlower[0] & 0x7FFF == 0)
            {
                _DELAY_MS(3000);
                step++;
            }
            break;
        case 2:
            /*
                        // 進入第二階段時，房間的燈會暗掉
                        mcpOutput.digitalWrite(ROOM_LIGHT, 0);
                        // 「第一首背景音樂」停止，開始撥放「第三首背景音樂」
                        mcpOutput.digitalWrite(MUSIC_1, 0);
                        mcpOutput.digitalWrite(MUSIC_3, 1);
                        // 此時記憶之花的五瓣門會打開花中間的LED會亮燈
                        mcpOutput.digitalWrite(MIDDLE_DOOR, 1);
                        // 從觸發到完全開啟時間剛好會是「第三首背景音樂」的時間
                        mcpOutput.digitalWrite(MUSIC_3, 0);
                        // FIXME //!待確認秒數
                        _DELAY_MS(30000);
                        nowState = 255;
                        step++;
                        */
            break;
            // 打開完畢後會開始撥放「第二首背景音樂」，牆壁雲朵的LED燈帶是寶藍色的
        case 3:
            /*
第二階段的記憶之花
        此時玩家會拿到花裡面的貓掌
        其實貓掌在第一階段其實如果按的到是可以直接觸發的
        只是用物理的方式讓玩家無法在第一階段按它
        貓掌上的四個按鈕是下述的4種狀態

        在狀態中按下其他三顆是可以被中斷直接撥按下去的狀態
        例如：在紅色狀態下，按下黃色狀態的按鈕會立刻切換黃色狀態
        但是在紅色狀態下，按下紅色狀態的按鈕會沒反應
        */
            // 左數第一顆按紐按下，「第二首背景音樂」會被中斷，撥放「藍色狀態」音效(01)
            // 牆壁雲朵的LED燈帶變藍色，26秒後雲朵LED燈帶變回寶藍色
            /*
            if (nowState != 1 && !digitalRead(ESP_CAT_PAW_1))
            {
                nowState = 1;
                numberTime = 0;
                mcpOutput.digitalWrite(MUSIC_2, 0);
                mcpOutput.digitalWrite(SOUND_1, 1);
                ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 0);
                ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 0);
                ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 255);

                for (uint8_t i = 7; i >= 1; i--)
                    passwordState[i] = passwordState[i - 1];
                passwordState[0] = nowState;
            }
            // 左數第二顆按紐按下，「第二首背景音樂」會被中斷，撥放「黃色狀態」音效(02)，
            //  牆壁雲朵的LED燈帶變黃色，26秒後雲朵LED燈帶變回寶藍色
            else if (nowState != 2 && !digitalRead(ESP_CAT_PAW_2))
            {
                nowState = 2;
                numberTime = 0;
                mcpOutput.digitalWrite(MUSIC_2, 0);
                mcpOutput.digitalWrite(SOUND_2, 1);
                ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 255);
                ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 255);
                ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 0);

                for (uint8_t i = 7; i >= 1; i--)
                    passwordState[i] = passwordState[i - 1];
                passwordState[0] = nowState;
            }
            // 左數第三顆按紐按下，「第二首背景音樂」會被中斷，撥放「紅色狀態」音效(03)，
            // 牆壁雲朵的LED燈帶變紅色，26秒後雲朵LED燈帶變回寶藍色
            else if (nowState != 3 && !digitalRead(ESP_CAT_PAW_3))
            {
                nowState = 3;
                numberTime = 0;
                mcpOutput.digitalWrite(MUSIC_2, 0);
                mcpOutput.digitalWrite(SOUND_3, 1);
                ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 255);
                ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 0);
                ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 0);

                for (uint8_t i = 7; i >= 1; i--)
                    passwordState[i] = passwordState[i - 1];
                passwordState[0] = nowState;
            }
            // 左數第四顆按紐按下，「第二首背景音樂」會被中斷，，撥放「綠色狀態」音效(04)，
            // 牆壁雲朵的LED燈帶變綠色，26秒後雲朵LED燈帶變回寶藍色
            else if (nowState != 4 && !digitalRead(ESP_CAT_PAW_4))
            {
                nowState = 4;
                numberTime = 0;
                mcpOutput.digitalWrite(MUSIC_2, 0);
                mcpOutput.digitalWrite(SOUND_4, 1);
                ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 0);
                ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 255);
                ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 0);

                for (uint8_t i = 7; i >= 1; i--)
                    passwordState[i] = passwordState[i - 1];
                passwordState[0] = nowState;
            }
            // 初始化
            else if (nowState == 255)
            {
                nowState = 0;
                mcpOutput.digitalWrite(MUSIC_2, 1);
                ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 0);
                ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 255);
                ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 255);
            }
            else
            {
                if (numberTime >= 260)
                {
                    nowState = 255;
                    numberTime = 0;
                }
                // 按下 紅藍黃綠紅藍黃綠(31243124) 後，地道門電磁鐵斷電，撥放「貓掌成功」音效(05)
                if (
                    passwordState[0] == 4 && passwordState[1] == 2 &&
                    passwordState[2] == 1 && passwordState[3] == 3 &&
                    passwordState[4] == 4 && passwordState[5] == 2 &&
                    passwordState[6] == 1 && passwordState[7] == 3 &&
                    true)
                {
                    step++;
                }
            }
            numberTime++;
            break;
            */
        case 4:
            /*
                mcpOutput.digitalWrite(SOUND_5, 1);
                mcpOutput.digitalWrite(FINAL_DOOR, 0);
                */
            break;
        }
        _DELAY_MS(100);
        /*
        最後是遙控器的四個按鍵
        A(地道RE)：牆壁雲朵燈暗掉，地道門按完才吸得上
        B(碎片RE)：第二階段結束，回到第一階段
        C(開地道)：觸發按下 紅藍黃綠紅藍黃綠 後發生的事情→地道門電磁鐵斷電，撥放「貓掌成功」音效(05)
        D(碎片啟動)：觸發放滿15個記憶碎片讓記憶花進入第二階段包含語音、五瓣花門開、房間燈關
            */
        if (!mcpOutput.digitalRead(REMOTE_CONTROL_1))
        {
            ledcWrite(ESP_LIGHT_R_PWM_CHANNEL, 0);
            ledcWrite(ESP_LIGHT_G_PWM_CHANNEL, 0);
            ledcWrite(ESP_LIGHT_B_PWM_CHANNEL, 0);
            mcpOutput.digitalWrite(FINAL_DOOR, 1);
        }
        if (!mcpOutput.digitalRead(REMOTE_CONTROL_2))
        {
            step = 0;
        }
        if (!mcpOutput.digitalRead(REMOTE_CONTROL_3))
        {
            step = 4;
        }
        if (!mcpOutput.digitalRead(REMOTE_CONTROL_3))
        {
            nowState = 255;
            step = 3;
        }
    }
}
void cat2()
{
    enum eESP32_Pin
    {
        IN_1 = 21,
        IN_2 = 19,
        IN_3 = 18,
        IN_4 = 5,
        IN_5 = 17,
        IN_6 = 16,
        IN_7 = 4,
        IN_8 = 32,
        IN_9 = 33,
        IN_10 = 25,
        IN_11 = 26,
        IN_12 = 27,
        IN_13 = 14,
        IN_14 = 12,
        IN_15 = 35,
        IN_REMOTE_STEP = 34,
        IN_REMOTE_RST = 39,
        OUT_SOUND = 13,
        OUT_DOOR = 23,
    };
    Adafruit_NeoPixel strip(24 * 15, 22, NEO_GRB + NEO_KHZ800);
    strip.begin(); // Initialize NeoPixel strip object (REQUIRED)
    strip.show();  // Initialize all pixels to 'off'

    const byte pinInput[15]{
        IN_1, IN_2, IN_3, IN_4,
        IN_5, IN_6, IN_7, IN_8,
        IN_9, IN_10, IN_11, IN_12,
        IN_13, IN_14, IN_15};
    uint16_t inFlower[2] = {0, 0};
    uint16_t timer = 0;
    uint8_t step = 0;
    size_t timeSnapshot = 0;
    for (byte i = 0; i < 15; i++)
        pinMode(pinInput[i], INPUT_PULLUP);
    pinMode(OUT_SOUND, OUTPUT);
    digitalWrite(OUT_SOUND, 1);
    pinMode(OUT_DOOR, OUTPUT);
    while (1)
    {
        inFlower[0] = 0;
        for (byte i = 0; i < 15; i++)
        {
            if (i == 14)
            {
                if (analogRead(pinInput[i]) < 512)
                    inFlower[0] += (1 << i);
            }
            else
            {
                if (!digitalRead(pinInput[i]))
                {
                    inFlower[0] += (1 << i);
                }
            }
        }
        if (inFlower[0] != inFlower[1])
        {
            // Serial.println(inFlower[0], HEX);
            if (inFlower[0] > inFlower[1])
            {
                digitalWrite(OUT_SOUND, 0);
                vTaskDelay(50);
                digitalWrite(OUT_SOUND, 1);
            }
            inFlower[1] = inFlower[0];
        }
        if (analogRead(IN_REMOTE_RST) < 512)
        {
            for (uint16_t i = 0; i < 24 * 15; i++)
                strip.setPixelColor(i, 0, 0, 0);
            strip.show();
            step = 0;
        }
        if (analogRead(IN_REMOTE_STEP) < 512)
        {
            step = 2;
        }
        /*
                else if (timeSnapshot != 0 && analogRead(IN_REMOTE) > 512)
                {

                    if (millis() - timeSnapshot > 2800)
                    {
                        Serial.printf("RST!,%d\n", millis() - timeSnapshot);
                        for (uint16_t i = 0; i < 24 * 15; i++)
                            strip.setPixelColor(i, 0, 0, 0);
                        strip.show();
                        step = 0;
                        timeSnapshot = 0;
                    }
                    else if (millis() - timeSnapshot > 1000)
                    {
                        Serial.printf("STEP!,%d\n", millis() - timeSnapshot);
                        step = 2;
                    }
                }
        */
        if (step == 0)
        {
            /*
碎片如果拿走燈暗掉
碎片拿走後重覆再放可以無限次重複觸發上述效果
*/
            for (uint8_t i = 0; i < 15; i++)
            {

                if ((inFlower[0] & (1 << i)))
                {
                    for (uint8_t j = 0; j < 24; j++)
                    {
                        strip.setPixelColor(i * 24 + j, strip.Color(255, 255, 0));
                        // uint32_t colocr = setRGB(millis() % 1536);
                        // strip.setPixelColor(i * 24 + j, colocr&0xFF,(colocr>>8)&0xFF,(colocr>>16)&0xFF);
                    }
                }
                else
                {
                    for (uint8_t j = 0; j < 24; j++)
                    {
                        strip.setPixelColor(i * 24 + j, strip.Color(0, 0, 0));
                    }
                }
            }
            strip.show();
            // 當15個碎片都放上以後約3秒進入第二階段
            // Serial.printf("%x\n", inFlower[0]);
            if (inFlower[0] == 0x7FFF)
            {
                step++;
                digitalWrite(OUT_DOOR, 1);
            }
        }
        else if (step == 1)
        {
            timer++;
            if (timer > 30)
            {
                timer = 0;
                digitalWrite(OUT_DOOR, 0);
                step++;
            }
        }
        else if (step == 2)
        {
            timer++;
            for (uint16_t i = 0; i < 24 * 15; i++)
            {
                int32_t colocr = setRGB((map(i, 0, 24 * 15, 1, 1535) + map(millis() % 1500, 0, 1500, 1, 1535)) % 1535 + 1);
                strip.setPixelColor(i, colocr & 0xFF, (colocr >> 8) & 0xFF, (colocr >> 16) & 0xFF);
            }
            strip.show(); // Initialize all pixels to 'off'
            if (timer > 100)
            {
                step++;
                timer = 0;
            }
        }
        else if (step == 3)
        {
            for (uint16_t i = 0; i < 24 * 15; i++)
                strip.setPixelColor(i, 255, 255, 0);

            strip.show();
            step++;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
uint32_t setRGB(uint16_t value)

{
    uint32_t rtValue = 0;

    if (value < 256) // 紅到黃
        rtValue = 255 | (value << 8);
    else if (value >= 256 && value < 512) // 黃到綠
        rtValue = (255 - (value - 256)) | (255 << 8);
    else if (value >= 512 && value < 768) // 綠到碧
        rtValue = (255 << 8) | ((value - 512) << 16);
    else if (value >= 768 && value < 1024) // 碧到藍
        rtValue = ((255 - (value - 768)) << 8) | (255 << 16);
    else if (value >= 1024 && value < 1280) // 藍到紫
        rtValue = (value - 1024) | (255 << 16);
    else if (value >= 1280 && value < 1536) // 紫到紅
        rtValue = 255 | (255 - (value - 1280) << 16);
    else
        ;
    return rtValue;
}

void task_Weapon(void *pvParam)
{
    while (1)
    {
        Serial.println(millis());
        _DELAY_MS(1000);
    }
}

void taskAudio()
{
// Digital I/O used
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 14
#define I2S_BCLK 12
#define I2S_LRC 13

    Audio audio;
    WiFiMulti wifiMulti;
    String ssid = "xxxxx";
    String password = "xxxxx";

    pinMode(SD_CS, OUTPUT);
    digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    Serial.begin(115200);
    SD.begin(SD_CS);
    /*
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(ssid.c_str(), password.c_str());
    wifiMulti.run();
    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.disconnect(true);
        wifiMulti.run();
    }
    */
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(12); // 0...21

    audio.connecttoFS(SD, "0001.mp3");
    //    audio.connecttohost("http://www.wdr.de/wdrlive/media/einslive.m3u");
    //    audio.connecttohost("http://somafm.com/wma128/missioncontrol.asx"); //  asx
    //    audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.aac"); //  128k aac
    // audio.connecttohost("http://mp3.ffh.de/radioffh/hqlivestream.mp3"); //  128k mp3

    while (1)
    {
        audio.loop();
    }
}

// optional
void audio_info(const char *info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}
void audio_showstation(const char *info)
{
    Serial.print("station     ");
    Serial.println(info);
}
void audio_showstreamtitle(const char *info)
{
    Serial.print("streamtitle ");
    Serial.println(info);
}
void audio_bitrate(const char *info)
{
    Serial.print("bitrate     ");
    Serial.println(info);
}
void audio_commercial(const char *info)
{ // duration in sec
    Serial.print("commercial  ");
    Serial.println(info);
}
void audio_icyurl(const char *info)
{ // homepage
    Serial.print("icyurl      ");
    Serial.println(info);
}
void audio_lasthost(const char *info)
{ // stream URL played
    Serial.print("lasthost    ");
    Serial.println(info);
}
