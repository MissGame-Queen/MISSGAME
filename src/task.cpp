#include "task.h"
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
void setRemember(uint8_t state, uint8_t *password)
{
}
void cat(void *pvParam)
{
    enum emcpOutput
    {
        MUSIC_1,
        MUSIC_2,
        MUSIC_3,
        SOUND_1,
        SOUND_2,
        SOUND_3,
        SOUND_4,
        SOUND_5,
        SOUND_6,
        ROOM_LIGHT,
        MIDDLE_DOOR,
        FINAL_DOOR,
        REMOTE_CONTROL_1,
        REMOTE_CONTROL_2,
        REMOTE_CONTROL_3,
        REMOTE_CONTROL_4,
    };
    enum ePin_ESP32
    {
        ESP_LIGHT_R_PWM_CHANNEL = 0,
        ESP_LIGHT_G_PWM_CHANNEL = 0,
        ESP_LIGHT_B_PWM_CHANNEL = 0,
        ESP_LIGHT_R = 32,
        ESP_LIGHT_G = 33,
        ESP_LIGHT_B = 25,
        ESP_CAT_PAW_1 = 26,
        ESP_CAT_PAW_2 = 27,
        ESP_CAT_PAW_3 = 14,
        ESP_CAT_PAW_4 = 12,
        ESP32_STRIP = 13
    };
    Adafruit_MCP23X08 mcpInput, mcpOutput;
    Adafruit_NeoPixel strip(24 * 15, ESP32_STRIP, NEO_GRB + NEO_KHZ800);
    mcpInput.begin_I2C(0x20);
    mcpOutput.begin_I2C(0x21);
    pinMode(ESP_CAT_PAW_1, INPUT_PULLUP);
    pinMode(ESP_CAT_PAW_2, INPUT_PULLUP);
    pinMode(ESP_CAT_PAW_3, INPUT_PULLUP);
    pinMode(ESP_CAT_PAW_4, INPUT_PULLUP);
    ledcSetup(ESP_LIGHT_R_PWM_CHANNEL, 5000, 8);
    ledcSetup(ESP_LIGHT_G_PWM_CHANNEL, 5000, 8);
    ledcSetup(ESP_LIGHT_B_PWM_CHANNEL, 5000, 8);
    ledcAttachPin(ESP_LIGHT_R, ESP_LIGHT_R_PWM_CHANNEL);
    ledcAttachPin(ESP_LIGHT_G, ESP_LIGHT_G_PWM_CHANNEL);
    ledcAttachPin(ESP_LIGHT_B, ESP_LIGHT_B_PWM_CHANNEL);
    for (int8_t i = 0; i < 16; i++)
    {
        mcpInput.pinMode(i, INPUT_PULLUP);
        if (i < 12)
        {
            mcpOutput.pinMode(i, OUTPUT);
            mcpOutput.digitalWrite(i, 0);
        }
        else
            mcpOutput.pinMode(i, INPUT_PULLUP);
    }
    mcpOutput.digitalWrite(FINAL_DOOR, 1);
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

            step++;
            break;
        case 1:
            // 第一階段的記憶之花
            //「第一首背景音樂」會一直循環撥放
            mcpOutput.digitalWrite(MUSIC_1, 1);
            // 玩家放下碎片的時候
            // 碎片後對應位置的燈片會亮黃燈，並且有「放碎片」音效(06)
            inFlower[0] = mcpInput.readGPIO(0) | (mcpInput.readGPIO(1) << 8);
            if (inFlower[0] != inFlower[1])
            {
                inFlower[1] = inFlower[0];
                mcpOutput.digitalWrite(SOUND_6, 1);
            }
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
        case 4:
            mcpOutput.digitalWrite(SOUND_5, 1);
            mcpOutput.digitalWrite(FINAL_DOOR, 0);
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
