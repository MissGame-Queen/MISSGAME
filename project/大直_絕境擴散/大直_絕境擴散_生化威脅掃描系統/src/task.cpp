#include "task.h"
QueueHandle_t queueJson = xQueueCreate(1, sizeof(String));
QueueHandle_t queuePCM5102 = xQueueCreate(1, sizeof(String));
QueueHandle_t queueMCP230x7_Input = xQueueCreate(1, sizeof(uint32_t));
QueueHandle_t queueMCP230x7_Output = xQueueCreate(1, sizeof(uint32_t));
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
String strAudio = "";
void task(void *pvParam)
{

    const uint8_t mcpAddress[] = {0x27};
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0;
    const uint32_t pinMCP_Output_FinalDoor = (1 << 0);             // 最終大門
    const uint32_t pinMCP_Output_Projector = (1 << 1);             // 投影機
    const uint32_t pinMCP_Output_VentilationDuct = (1 << 5);       // 通風管電磁鐵
    const uint32_t pinMCP_Output_Door = (1 << 6);                  // 氣動門電磁閥
    const uint32_t pinMCP_Output_AirDisinfection = (1 << 7);       // 空氣消毒完成訊號
    const uint32_t pinMCP_Input_ScanButton = (1 << 0);             // 掃描按鈕
    const uint32_t pinMCP_Input_AirDisinfection = (1 << 1);        // 空氣消毒完成訊號
    const uint32_t pinMCP_Input_Remote_AntidoteMachine = (1 << 4); // 解藥機完成遙控器
    const uint32_t pinMCP_Input_Remote_Door = (1 << 5);            // 氣動門遙控器
    const uint32_t pinMCP_Input_Remote_AirDisinfection = (1 << 6); // 空氣消毒完成訊號
    const uint32_t pinMCP_Input_Remote_Projector = (1 << 7);       // 投影機遙控器

    bool havelock_Projector = false;
    while (1)
    {
        // 讀取MCP23017的Input資料
        if (xQueueReceive(queueMCP230x7_Input, &dataInput_32t, 0) == pdPASS)
        {
            ;
        }
        // 如果Output焊上次不一樣則輸出到MCP23017
        if (dataOutput_32t != dataOutputLast_32t)
        {
            xQueueSend(queueMCP230x7_Output, &dataOutput_32t, portMAX_DELAY);
            dataOutputLast_32t = dataOutput_32t;
            //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "dataOutput_32t=0x%08X\n", dataOutput_32t);
        }
        dataInput_8t = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            dataInput_8t += ((!digitalRead(pinInput[i])) ? 1 << i : 0);
        }
        if (dataInputLast_8t != dataInput_8t)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "輸入:%02x!\n", dataInput_8t);
            dataInputLast_8t = dataInput_8t;
        }

        //[ ]遊戲流程

        switch (stepGame)
        {
        case _Reset:
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);

            dataOutput_32t = dataOutput_32t | pinMCP_Output_FinalDoor;
            dataOutput_32t = dataOutput_32t | pinMCP_Output_VentilationDuct;
            havelock_Projector = false;
            //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "dataOutput_32t=0x%08X\n", dataOutput_32t);
            first = true;

            stepGame++;
        }
        break;
        case _PressButton:
        {
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
            }
            if (dataInput_32t & pinMCP_Input_ScanButton)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _PlaySound:
        {
            static uint8_t if3timeplay = 0;
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                if3timeplay = 0;
                timer = 0;
                havelock_Projector = true;
            }
            if (if3timeplay < 3 && millis() > 4000 && (millis() - 4000) > timer)
            {
                strAudio = "{\"name\": \"/mp3/1-1.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                if3timeplay++;
            }
            else if (if3timeplay == 3 && millis() > 4000 && (millis() - 4000) > timer)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _PlaySound_2:
        {
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                timer = 0;
                havelock_Projector = false;
                strAudio = "{\"name\": \"/mp3/1-2.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
            }
            if ((millis() > 7000) && (millis() - 7000) > timer)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _AirDisinfection:
        {
            static uint8_t swRemote;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                swRemote = dataInput_32t & pinMCP_Input_AirDisinfection;
            }
            // 如果收到訊號或者按壓遙控器
            if (swRemote != (dataInput_32t & pinMCP_Input_AirDisinfection))
            {
                stepGame++;
                first = true;
            }
            // 如果又按按鈕則回到上一個按鈕階段
            if (dataInput_32t & pinMCP_Input_ScanButton)
            {
                stepGame = _PlaySound;
                first = true;
            }
        }
        break;
        case _PressButton_2:
        {
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
            }
            if (dataInput_32t & pinMCP_Input_ScanButton)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _PlaySound_3:
        {
            static uint8_t playType = 0;
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                playType = 0;
                timer = 0;
                havelock_Projector = true;
            }
            if (playType < 2 && millis() > 4000 && (millis() - 4000) > timer)
            {
                strAudio = "{\"name\": \"/mp3/1-1.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 2 && millis() > 4000 && (millis() - 4000) > timer)
            {
                strAudio = "{\"name\": \"/mp3/1-2.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 3 && millis() > 8000 && (millis() - 8000) > timer)
            {
                strAudio = "{\"name\": \"/mp3/1-3.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 4 && millis() > 5000 && (millis() - 5000) > timer)
            {
                strAudio = "{\"name\": \"/mp3/1-3.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 5 && millis() > 5000 && (millis() - 5000) > timer)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _AntidoteMachine:
        {
            static uint8_t swRemote = 0;
            if (first)
            {
                dataOutput_32t = dataOutput_32t & (~pinMCP_Output_VentilationDuct);

                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                swRemote = dataInput_32t & pinMCP_Input_Remote_AntidoteMachine;
                havelock_Projector = false;
            }
            // 如果狀態被改變判定按壓遙控器
            if (swRemote != (dataInput_32t & pinMCP_Input_Remote_AntidoteMachine))
            {
                stepGame++;
                first = true;
            }
            // 如果又按按鈕則回到上一個按鈕階段
            if (dataInput_32t & pinMCP_Input_ScanButton)
            {
                stepGame = _PlaySound_3;
                first = true;
            }
        }
        break;
        case _PressButton_3:
        {
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
            }
            if (dataInput_32t & pinMCP_Input_ScanButton)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _PlaySound_4:
        {
            static uint8_t playType = 0;
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                playType = 0;
                strAudio = "{\"name\": \"/mp3/1-1.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                havelock_Projector = true;
            }
            if (playType < 2 && millis() > 5000 && (millis() - 5000) > timer)
            {
                strAudio = "{\"name\": \"/mp3/1-4.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 2 && millis() > 5000 && (millis() - 5000) > timer)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _Finish:
        {
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                dataOutput_32t = dataOutput_32t & (!pinMCP_Output_FinalDoor);
                timer = millis();
                havelock_Projector = false;
            }
            if ((millis() - 5000) > timer)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _DEBUG:
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DEBUG END!\n");
            stepGame = _Reset;
        }
        break;

        default:
            break;
        }
        //[ ]遙控器
        // 如果按下投影機按鈕強制開啟否則照流程
        if (dataInput_32t & pinMCP_Input_Remote_Projector)
        {
            dataOutput_32t = dataOutput_32t | pinMCP_Output_Projector;
        }
        else
        {
            if (havelock_Projector)
                dataOutput_32t = dataOutput_32t | (pinMCP_Output_Projector);
            else
                dataOutput_32t = dataOutput_32t & (~pinMCP_Output_Projector);
        }
        // 氣動門
        if (dataInput_32t & pinMCP_Input_Remote_Door)
            dataOutput_32t = dataOutput_32t | pinMCP_Output_Door;
        else
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_Door);
        // 空氣消毒謎題跳過
        if (dataInput_32t & pinMCP_Input_Remote_AirDisinfection)
            dataOutput_32t = dataOutput_32t | pinMCP_Output_AirDisinfection;
        else
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_AirDisinfection);

        dataInputLast_32t = dataInput_32t;

        // strip.show();
        _DELAY_MS(100);
    }
}
void taskMCP230x7(void *pvParam)
{
    const uint8_t mcpAddress[] = {0x20};
    Adafruit_MCP23X17 mcp[sizeof(mcpAddress)];
    // 初始化
    bool isInit = true;
    for (uint8_t i = 0; i < sizeof(mcpAddress); i++)
    {
        if (!mcp[i].begin_I2C(mcpAddress[i]))
            isInit = false;
        for (uint8_t j = 0; j < 8; j++)
        {
            _DELAY_MS(1);
            mcp[i].pinMode(j, OUTPUT);
        }
        _DELAY_MS(1);
    }
    // 如果初始化失敗
    if (!isInit)
    {
        while (1)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "MCP連線失敗!\n");
            _DELAY_MS(1000);
        }
    }
    else
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "MCP連線成功!\n");
    uint32_t dataInput_32t = 0, dataOutput_32t = 0;
    while (1)
    {
        // 如果收到寫入封包
        if (xQueueReceive(queueMCP230x7_Output, &dataOutput_32t, 0) == pdPASS)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[task]MCP Output=%08X\n", dataOutput_32t);
            for (uint8_t i = 0; i < sizeof(mcpAddress); i++)
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
            for (uint8_t i = 0; i < sizeof(mcpAddress); i++)
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

void taskPCM5102(void *pvParam)
{
    DFRobotDFPlayerMini myDFPlayer;
    uint8_t SoundPlayerLevel[2] = {0, 0};
    Serial2.begin(9600);
    bool state[2] = {
        false,
        false,
    };
    state[1] = myDFPlayer.begin(Serial2, /*isACK = */ true, /*doReset = */ true);

    if (state[1])
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 已連線\n");
        myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms
        myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
        myDFPlayer.volume(30);
    }
    else
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 連線失敗!\n1.請重新檢查連線！\n2.請插入SD卡！\n");
    pinMode(5, OUTPUT);
    state[0] = SD.begin(5);
    if (!state[0])
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "SD卡發生錯誤\n");
    }
    else
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "SD卡正常!!\n");
    }

    // Audio *audio = (Audio *)pvParam;
    Audio *audio = new Audio(false, I2S_DAC_CHANNEL_DISABLE, I2S_NUM_0);
    if (audio != nullptr && state[0])
    {
        audio->setPinout(pinBCLK, pinLRC, pinDOUT);
        audio->setVolume(21); // 0...21
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "啟用I2S!\n");
    }
    else
    {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "指標為空\n");
        state[0] = false;
    }
    String StrJson = "";

    JsonDocument doc;
    //?因為PSRAM關係需要用這個
    /*
   SpiRamAllocator allocator;
   JsonDocument doc(&allocator);
   */
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
                //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[task]PCM5102=%s\n", StrJson.c_str());
                if (doc.containsKey("name") && state[0])
                {
                    if (doc["name"] == "")
                    {
                        SoundPlayerLevel[0] = 0;
                        audio->connecttoFS(SD, "/SYSTEM/stop.mp3");
                        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "停止音樂!%d\n", SoundPlayerLevel[0]);
                    }
                    else
                    {
                        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[task]播放音樂!%s\n", doc["name"].as<String>().c_str());
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
        if (SoundPlayerLevel[0] != 0 && state[0])
        {
            if (!audio->isRunning())
            {
                SoundPlayerLevel[0] = 0;
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放結束!等級歸0\n");
            }
        }
        if (xSemaphoreTake(xMutex, portMAX_DELAY))
        {
            if (state[0])
                audio->loop();

            _DELAY_MS(1);
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(3);
    }
}
