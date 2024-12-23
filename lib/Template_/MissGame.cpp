#include <MissGame.h>
QueueHandle_t queuePCM5102 = xQueueCreate(1, sizeof(JsonDocument *));
QueueHandle_t queueMCP230x7_Input = xQueueCreate(1, sizeof(uint32_t));
QueueHandle_t queueMCP230x7_Output = xQueueCreate(1, sizeof(uint32_t));

void taskPCM5102(void *pvParam)
{
    uint8_t pinSD = 5, pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26, pinTX2 = 32, pinRX2 = 33;
    DFRobotDFPlayerMini myDFPlayer;
    uint8_t SoundPlayerLevel[2] = {0, 0};
    JsonDocument *ptrJson;
    JsonDocument doc;
    Audio *audio;
    uint16_t delayTime = 20;
    bool state[2] = {false, false};
    Serial2.begin(9600, SERIAL_8N1, pinRX2, pinTX2);
    state[1] = myDFPlayer.begin(Serial2, /*isACK = */ false, /*doReset = */ true);
    _DELAY_MS(delayTime);

    if (state[1])
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 已連線\n");
        myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms
        _DELAY_MS(delayTime);

        myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
        _DELAY_MS(delayTime);

        myDFPlayer.volume(30);
        _DELAY_MS(delayTime);
    }
    else
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 連線失敗!\n1.請重新檢查連線!\n2.請插入SD卡!\n");
    if (xSemaphoreTake(SPIMutex, portMAX_DELAY))
    {
        state[0] = SD.begin(pinSD);
        xSemaphoreGive(SPIMutex);
    }

    if (!state[0])
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "SD卡發生錯誤\n");
    }
    //?不知為何開啟PSRAM需在外面宣告Audio實體
    if (state[0])
    {
        if (psramFound())
        {
            static Audio ptrAudio = Audio(false, I2S_DAC_CHANNEL_DISABLE, I2S_NUM_0);
            audio = &ptrAudio;
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "使用main實體I2S\n");
        }
        else
        {
            audio = new Audio(false, I2S_DAC_CHANNEL_DISABLE, I2S_NUM_0);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "使用task實體I2S\n");
        }
        audio->setPinout(pinBCLK, pinLRC, pinDOUT);
        audio->setVolume(21); // 0...21
    }

    while (1)
    {
        if (xQueueReceive(queuePCM5102, &ptrJson, 0) == pdPASS)
        {
            doc = (*ptrJson);
            if (state[0])
            {
                if (!doc["I2S"].isNull())
                {
                    JsonObject docI2S = doc["I2S"];
                    if (!docI2S["volume"].isNull())
                    {
                        audio->setVolume(docI2S["volume"].as<uint8_t>()); // 0...21
                    }
                    if (!docI2S["name"].isNull())
                    {

                        bool boolAllowPlay = true;
                        String mp3Value = docI2S["name"].as<String>().c_str();

                        // 如果清空播放清單
                        if (docI2S["name"] == "")
                        {
                            boolAllowPlay = false;
                            SoundPlayerLevel[0] = 0;
                            audio->stopSong();
                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "停止音樂!%d\n", SoundPlayerLevel[0]);
                        }
                        else
                        {
                            // 如果有附帶等級
                            if (!docI2S["level"].isNull())
                            {
                                // 如果大於等於目前等級則播放
                                if (docI2S["level"].as<uint8_t>() >= SoundPlayerLevel[0])
                                {
                                    SoundPlayerLevel[0] = docI2S["level"].as<uint8_t>();
                                }
                                else
                                {
                                    boolAllowPlay = false;
                                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放失敗!目前level:%d,接收level:%d\n", SoundPlayerLevel[0], docI2S["level"].as<uint8_t>());
                                }
                            }
                        }
                        if (boolAllowPlay)
                        {
                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放I2S音樂!%s\n", mp3Value.c_str());
                            if (xSemaphoreTake(SPIMutex, portMAX_DELAY))
                            {
                                audio->connecttoFS(SD, mp3Value.c_str());
                                audio->loop();
                                xSemaphoreGive(SPIMutex);
                            }
                        }
                    }
                }
            }
            if (state[1])
            {
                if (!doc["Serial"].isNull())
                {
                    JsonObject docSerial = doc["Serial"];

                    if (!docSerial["value"].isNull())
                    {
                        bool boolAllowPlay = true;
                        uint16_t mp3Value = docSerial["value"].as<uint16_t>();
                        // 如果清空播放清單
                        if (mp3Value == 0)
                        {
                            boolAllowPlay = false;
                            SoundPlayerLevel[1] = 0;
                            myDFPlayer.disableLoop();
                            _DELAY_MS(delayTime);
                            myDFPlayer.stop();
                            _DELAY_MS(delayTime);

                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "停止音樂!%d\n", SoundPlayerLevel[1]);
                        }
                        else
                        {
                            // 如果有附帶等級
                            if (!docSerial["level"].isNull())
                            {
                                // 如果大於等於目前等級則播放
                                if (docSerial["level"].as<uint8_t>() >= SoundPlayerLevel[1])
                                {
                                    SoundPlayerLevel[1] = docSerial["level"].as<uint8_t>();
                                }
                                else
                                {
                                    boolAllowPlay = false;
                                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放失敗!目前level:%d,接收level:%d\n", SoundPlayerLevel[1], docSerial["level"].as<uint8_t>());
                                }
                            }
                        }
                        if (boolAllowPlay)
                        {
                            // 如果沒特別設定loop則默認loop
                            if (!docSerial["loop"].isNull())
                            {
                                if (docSerial["loop"].as<bool>())
                                {
                                    _DELAY_MS(delayTime);
                                    myDFPlayer.loop(mp3Value);
                                    _DELAY_MS(delayTime);
                                }
                                else
                                {
                                    _DELAY_MS(delayTime);
                                    myDFPlayer.play(mp3Value);
                                    _DELAY_MS(delayTime);
                                }
                            }
                            else
                            {
                                _DELAY_MS(delayTime);
                                myDFPlayer.loop(mp3Value);
                                _DELAY_MS(delayTime);
                            }

                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放Serial音樂!%d\n", mp3Value);

                            // Print the detail message from DFPlayer to handle different errors and states.
                            // if (myDFPlayer.available())
                            //  printDetail(myDFPlayer.readType(), myDFPlayer.read());
                        }
                    }
                    if (!docSerial["volume"].isNull())
                    {
                        myDFPlayer.volume(docSerial["volume"].as<uint8_t>());
                        _DELAY_MS(delayTime);
                    }
                }
            }
        }
        if (state[0])
        {
            if (SoundPlayerLevel[0] != 0)
            {
                if (!audio->isRunning())
                {
                    SoundPlayerLevel[0] = 0;
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放結束!等級歸0\n");
                }
            }
            if (xSemaphoreTake(rmtMutex, portMAX_DELAY))
            {
                if (xSemaphoreTake(SPIMutex, portMAX_DELAY))
                {
                    audio->loop();
                    _DELAY_MS(1);
                    xSemaphoreGive(SPIMutex);
                }
                xSemaphoreGive(rmtMutex);
            }
        }
        vTaskDelay(3);
    }
}
void taskMCP230x7(void *pvParam)
{
    JsonDocument *doc = (JsonDocument *)pvParam;

    JsonObject obj;
    JsonArray arr;
    uint8_t mcpAddress[] = {0x20};
    uint32_t dataInput_32t = 0, dataOutput_32t = 0;
    bool isInit = true;
    if ((*doc).containsKey("MCP23017"))
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "啟用MCP23017!\n");
        obj = (*doc)["MCP23017"];
        // 若沒指定地址則使用默認
        if (!obj.containsKey("Address"))
        {
            arr = obj["Address"].to<JsonArray>();
            for (uint8_t i = 0; i < sizeof(mcpAddress); i++)
                arr.add(mcpAddress[i]);
        }
        else
            arr = obj["Address"].as<JsonArray>();
    }
    else
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "不啟用MCP23017!\n");
        isInit = false;
        obj = (*doc)["MCP23017"];
        arr = obj["Address"].to<JsonArray>();
        for (uint8_t i = 0; i < sizeof(mcpAddress); i++)
            arr.add(mcpAddress[i]);
    }
    // 若啟用則嘗試初始化
    Adafruit_MCP23X17 mcp[arr.size()];
    if (isInit)
    {
        for (uint8_t i = 0; i < arr.size(); i++)
        {
            if (!mcp[i].begin_I2C(arr[i].as<uint8_t>()))
            {
                isInit = false;
                break;
            }
            else
            {
                for (uint8_t j = 0; j < 8; j++)
                {
                    _DELAY_MS(1);
                    mcp[i].pinMode(j, OUTPUT);
                }
                _DELAY_MS(1);
            }
        }
    }
    // 如果初始化失敗
    if (!isInit)
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "MCP連線失敗!\n");
        while (true)
        {
            if (xQueueReceive(queueMCP230x7_Output, &dataOutput_32t, 0) == pdPASS)
                ;
            _DELAY_MS(10);
        }
    }
    else
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "MCP連線成功!\n");

    while (1)
    {
        // 如果收到寫入封包
        if (xQueueReceive(queueMCP230x7_Output, &dataOutput_32t, 0) == pdPASS)
        {
                        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[task]MCP Output=%08X\n", dataOutput_32t);
            for (uint8_t i = 0; i < arr.size(); i++)
            {
                uint8_t dataBit = (dataOutput_32t >> (i * 8)) & 0xFF;
                for (uint8_t j = 0; j < 8; j++)
                {
                    mcp[i].digitalWrite(j, (dataBit & (1 << j)) > 0 ? 1 : 0);
                    // mcp[i].writeGPIO(/*(data >> (i * 8)) &*/ 0xFF, 0);
                    _DELAY_MS(1);
                }
            }
        }
        // 定期輪詢所有Input，若與上次不同則發送結果
        static uint32_t timer = millis();
        static uint32_t dataInputLast_32t = 0;
        if (millis() > timer + 100)
        {
            dataInput_32t = 0;
            for (uint8_t i = 0; i < arr.size(); i++)
            {
                uint8_t dataBit = mcp[i].readGPIOB() ^ 0xFF;
                dataInput_32t += dataBit << (i * 8);
                //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%d=%d\n", i, dataRead[i]);
                _DELAY_MS(1);
            }
            if (dataInputLast_32t != dataInput_32t)
            {
                xQueueSend(queueMCP230x7_Input, &dataInput_32t, portMAX_DELAY);
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[task]MCP Input=%08X\n", dataInput_32t);
                dataInputLast_32t = dataInput_32t;
            }
            timer = millis();
        }
        _DELAY_MS(10);
    }
}
void taskExamples(void *pvParam)
{
    const uint8_t mcpAddress[] = {0x27};
    const uint8_t pinOutput[] = {12, 13, 14, 15};
    const uint8_t pinInput[] = {36, 39, 34, 35};
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0, dataOutput_8t = 0, dataOutputLast_8t = 0xFF;
    /*
    const uint32_t pinMCP_Input_ReedSwitch = (1 << 0);       // 斷手磁簧
    const uint32_t pinMCP_Input_PuzzleCompleted = (1 << 1);  // 謎題完成訊號
    const uint32_t pinMCP_Input_VentilationDuct = (1 << 5);  // 通風管訊號
    const uint32_t pinMCP_Input_Door = (1 << 6);             // 氣動門訊號
    const uint32_t pinMCP_Input_Finsh = (1 << 7);            // 跳關訊號
    const uint32_t pinMCP_Output_LiftMotorPow = (1 << 0);    // 升降馬達電源
    const uint32_t pinMCP_Output_LiftMotorDir = (1 << 1);    // 升降馬達方向
    const uint32_t pinMCP_Output_RunLight = (1 << 2);        // 空氣消毒燈光
    const uint32_t pinMCP_Output_VentilationDuct = (1 << 3); // 通風管電磁鐵
    const uint32_t pinMCP_Output_Door = (1 << 4);            // 氣動門
    const uint32_t pinMCP_Output_GameFinsh = (1 << 5);       // 遊戲結束訊號
*/
    for (uint8_t i = 0; i < sizeof(pinInput); i++)
    {
        pinMode(pinInput[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < sizeof(pinOutput); i++)
    {
        pinMode(pinOutput[i], OUTPUT);
        digitalWrite(pinOutput[i], 0);
    }
    while (1)
    {
        // 讀取MCP23017的Input資料
        if (xQueueReceive(queueMCP230x7_Input, &dataInput_32t, 0) == pdPASS)
        {
            ;
        }
        // 如果Output和上次不一樣則輸出到MCP23017
        if (dataOutput_32t != dataOutputLast_32t)
        {
            xQueueSend(queueMCP230x7_Output, &dataOutput_32t, portMAX_DELAY);
            dataOutputLast_32t = dataOutput_32t;
        }
        dataInput_8t = 0;
        for (uint8_t i = 0; i < sizeof(pinOutput); i++)
        {
            dataInput_8t += ((!digitalRead(pinInput[i])) ? 1 << i : 0);
        }
        if (dataInputLast_8t != dataInput_8t)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "主控輸入:%02x!\n", dataInput_8t);
            dataInputLast_8t = dataInput_8t;
        }
        if (dataOutput_8t != dataOutputLast_8t)
        {
            dataOutputLast_8t = dataOutput_8t;
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "主控輸出:%02x!\n", dataOutput_8t);
            for (uint8_t i = 0; i < sizeof(pinOutput); i++)
            {
                digitalWrite(pinOutput[i], dataOutput_8t & (1 << i));
            }
        }

        //[ ]遊戲流程

        switch (stepGame)
        {
        default:
            break;
        }
        //[ ]遙控器

        dataInputLast_32t = dataInput_32t;

        // strip.show();
        _DELAY_MS(100);
    }
}

#ifdef DEBUG_PCM5102
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
#endif