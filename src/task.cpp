#include "task.h"
QueueHandle_t queueJson = xQueueCreate(1, sizeof(String));
QueueHandle_t queuePCM5102 = xQueueCreate(1, sizeof(String));
QueueHandle_t queueMCP230x7_Input = xQueueCreate(1, sizeof(uint32_t));
QueueHandle_t queueMCP230x7_Output = xQueueCreate(1, sizeof(uint32_t));
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
String strAudio = "";
void task(void *pvParam)
{
    const uint8_t pinOutput[] = {15, 14, 12, 13};
    const uint8_t pinInput[] = {36, 39, 34, 35};
    const uint8_t mcpAddress[] = {0x27, 0x26, 0x25, 0x24};
    const uint32_t pinInputMPC[12] = {0x10, 0x80, 0x2000, 0x1000, 0x4000, 0x8, 0x4, 0x2, 0x200, 0x800, 0x400, 0x40};
    const uint32_t pinOutputMPC[4]{0x08, 0x04, 0x01, 0x02};
    const uint32_t pinOutputMPC_MODE = 0x10;
    const uint32_t pinOutputMPC_DoorS = 0x20;
    const uint32_t pinOutputMPC_DoorS_Remote = 0x10000;
    bool havelock_DoorS = true;
    const uint32_t pinOutputMPC_Door = 0x40;
    const uint32_t ansGame1[4][4] = {{0x2000, 0, 0, 0}, {0x1000, 0x8, 0, 0}, {0x4, 0x1000, 0x400, 0}, {0x200, 0x40, 0x4000, 0x80}};
    const uint32_t ansGame2[2][12] = {{0x8, 0x2, 0x200, 0x4, 0x4000, 0x40, 0x10, 0x80, 0x400, 0x2000, 0x800, 0x1000}, {0x8, 0x2, 0x200, 0x4000, 0x4, 0x40, 0x10, 0x80, 0x400, 0x2000, 0x800, 0x1000}};
    uint8_t ansGame2Index = 0;
    uint32_t ansGame2Value = 0;
    uint8_t stepGame1 = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0, dataOutput_8t = 0, dataOutputLast_8t = 0xFF;
    for (uint8_t i = 0; i < 4; i++)
    {
        if (i != 0)
        {
            pinMode(pinOutput[i], OUTPUT);
            digitalWrite(pinOutput[i], 1);
        }
        pinMode(pinInput[i], INPUT);
    }
    Adafruit_NeoPixel strip(12, pinOutput[0], NEO_GRB + NEO_KHZ800);

    /*
    RST
    關門
    12個燈滅
    中間背景燈恆亮
    中間狀態燈滅
    play
    根據踩的狀態點亮12燈
    踩到正確的中間狀態燈要亮,正確音效
    end
    開門音效
    開門
    時間到RST
    */
    while (1)
    {
        if (xQueueReceive(queueMCP230x7_Input, &dataInput_32t, 0) == pdPASS)
            ;
        if (dataOutput_32t != dataOutputLast_32t)
        {
            xQueueSend(queueMCP230x7_Output, &dataOutput_32t, portMAX_DELAY);
            dataOutputLast_32t = dataOutput_32t;
        }
        dataInput_8t = 0;
        for (uint8_t i = 0; i < 4; i++)
        {
            dataInput_8t += ((!digitalRead(pinInput[i])) ? 1 << i : 0);
        }
        if (dataInputLast_8t != dataInput_8t)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "輸入:%02x!\n", dataInput_8t);
            if ((dataInput_8t & 1) && !(dataInputLast_8t & 1))
            {
                dataOutput_32t = dataOutput_32t | pinOutputMPC_MODE;
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "流亡模式!%02X\n", dataOutput_32t);
                stepGame1 = 0;
            }
            else if (!(dataInput_8t & 1) && (dataInputLast_8t & 1))
            {
                dataOutput_32t = dataOutput_32t & (~(pinOutputMPC_MODE));
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "奇術模式!%02X\n", dataOutput_32t);
                stepGame1 = 0;
            }

            dataInputLast_8t = dataInput_8t;
        }
        if (dataOutputLast_8t != dataOutput_8t)
        {
            for (uint8_t i = 1; i < 4; i++)
                digitalWrite(pinOutput[i], dataOutput_8t & (1 << i) ? 1 : 0);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "輸出:%02x!\n", dataOutput_8t);
            dataOutputLast_8t = dataOutput_8t;
        }

        bool isCorrect = false;
        //[v]流亡
        if (dataInput_8t & 1)
        {
            for (uint8_t i = 0; i < sizeof(pinInputMPC) / sizeof(pinInputMPC[0]); i++)
            {
                if (pinInputMPC[i] & dataInput_32t)
                    strip.setPixelColor(i, 125, 125, 125);
                else
                    strip.setPixelColor(i, 0);
            }
            switch (stepGame1)
            {
            case 0:
            {
                first = true;
                stepGame1++;
                dataOutput_32t = dataOutput_32t & (~pinOutputMPC_Door);
                havelock_DoorS = true;
                dataOutput_32t = dataOutput_32t & (~(pinOutputMPC[0] + pinOutputMPC[1] +
                                                     pinOutputMPC[2] + pinOutputMPC[3]));
            }
            break;
            case 1:
                if (dataInput_32t == ansGame1[stepGame1 - 1][0])
                {
                    stepGame1++;
                    isCorrect = true;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[0];
                }
                break;
            case 2:
                if ((dataInput_32t & ansGame1[stepGame1 - 1][0]) &&
                    (dataInput_32t & ansGame1[stepGame1 - 1][1]))
                {
                    stepGame1++;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[1];
                    isCorrect = true;
                }
                break;
            case 3:
                if ((dataInput_32t & ansGame1[stepGame1 - 1][0]) &&
                    (dataInput_32t & ansGame1[stepGame1 - 1][1]) &&
                    (dataInput_32t & ansGame1[stepGame1 - 1][2]))
                {
                    stepGame1++;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[2];
                    isCorrect = true;
                }
                break;
            case 4:
                if ((dataInput_32t & ansGame1[stepGame1 - 1][0]) &&
                    (dataInput_32t & ansGame1[stepGame1 - 1][1]) &&
                    (dataInput_32t & ansGame1[stepGame1 - 1][2]) &&
                    (dataInput_32t & ansGame1[stepGame1 - 1][3]))
                {
                    stepGame1++;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[3];
                }
                break;
            case 5:
            {
                static uint32_t timer = millis();
                if (first)
                {
                    first = !first;
                    timer = millis();
                    strAudio = "{\"name\": \"/流亡_女神門開.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    dataOutput_32t = dataOutput_32t | (pinOutputMPC_Door);

                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "女神門開!%d\n", timer);
                }
                if (millis() > timer + 21000)
                {
                    stepGame1 = 0;
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "音樂結束RST!\n");
                }
            }
            break;
            }

            if (stepGame1 != 5)
            {
                if (isCorrect)
                {
                    strAudio = "{\"name\": \"/流亡_踩對符號.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    isCorrect = false;
                }
                else if ((dataInputLast_32t& 0xFFFF) < (dataInput_32t& 0xFFFF))
                {
                    strAudio = "{\"name\": \"/流亡_踩地板.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%08x!\n", dataInput_32t);
                }
            }
        }
        //[ ]奇術
        else if (!(dataInput_8t & 1))
        {
            switch (stepGame1)
            {
            case 0:
            {
                for (uint8_t i = 0; i < sizeof(pinInputMPC) / sizeof(pinInputMPC[0]); i++)
                {
                    strip.setPixelColor(i, 0);
                }
                first = true;
                stepGame1++;
                ansGame2Value = 0;
                dataOutput_32t = dataOutput_32t & (~pinOutputMPC_Door);
                havelock_DoorS = true;
                dataOutput_32t = dataOutput_32t & (~(pinOutputMPC[0] + pinOutputMPC[1] +
                                                     pinOutputMPC[2] + pinOutputMPC[3]));
                ansGame2Index = 0;
            }
            break;
            case 1:
                if ((dataInput_32t == ansGame2[0][ansGame2Index] && !(ansGame2Value & ansGame2[0][ansGame2Index])) ||
                    (dataInput_32t == ansGame2[1][ansGame2Index] && !(ansGame2Value & ansGame2[1][ansGame2Index])))
                {
                    ansGame2Value = ansGame2Value | dataInput_32t;
                    for (uint8_t i = 0; i < sizeof(pinInputMPC) / sizeof(pinInputMPC[0]); i++)
                    {
                        if (pinInputMPC[i] & dataInput_32t)
                            strip.setPixelColor(i, 125, 125, 125);
                    }
                    ansGame2Index++;
                    isCorrect = true;
                }
                if (ansGame2Index == sizeof(ansGame2[0]) / sizeof(ansGame2[0][0]))
                {
                    stepGame1++;
                }
                break;
            case 2:
            {
                static uint32_t timer = millis();
                if (first)
                {
                    first = !first;
                    timer = millis();
                    strAudio = "{\"name\": \"/奇術_踩對行星順序.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "正確答案!%d\n", timer);
                }
                if (millis() > timer + 5000)
                {
                    stepGame1++;
                    first = true;
                }
            }
            case 3:
            {
                static uint32_t timer = millis();
                if (first)
                {
                    first = !first;
                    timer = millis();
                    strAudio = "{\"name\": \"/奇術_蜘蛛門開.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    havelock_DoorS = false;
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "蜘蛛門開!%d\n", timer);
                }
                if (millis() > timer + 5000)
                {
                    stepGame1 = 0;
                }
            }
            break;
            }

            if (stepGame1 == 1)
            {
                if (isCorrect)
                {
                    strAudio = "{\"name\": \"/流亡_踩地板.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    isCorrect = false;
                }
                else if (!(dataInput_32t & ansGame2Value) && (dataInputLast_32t & 0xFFFF) < (dataInput_32t & 0xFFFF))
                {
                    strAudio = "{\"name\": \"/奇術_踩錯行星順序.mp3\"}";
                    xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%08x!\n", dataInput_32t);
                    stepGame1 = 0;
                }
            }
        }
        //[ ]遙控器
        // 如果按下蜘蛛門
        if (dataInput_32t & pinOutputMPC_DoorS_Remote)
        {
            dataOutput_32t = dataOutput_32t & (~pinOutputMPC_DoorS);
        }
        else
        {
            if (havelock_DoorS)
                dataOutput_32t = dataOutput_32t | (pinOutputMPC_DoorS);
            else
                dataOutput_32t = dataOutput_32t & (~pinOutputMPC_DoorS);
        }
        dataInputLast_32t = dataInput_32t;

        strip.show();
        _DELAY_MS(100);
    }
}
void taskMCP230x7(void *pvParam)
{
    const uint8_t mcpAddress[] = {0x27, 0x26, 0x25, 0x24};
    Adafruit_MCP23X17 mcp[4];
    // 初始化
    bool isInit = true;
    for (uint8_t i = 0; i < sizeof(mcp) / sizeof(mcp[0]); i++)
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
            for (uint8_t i = 0; i < sizeof(mcp) / sizeof(mcp[0]); i++)
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
            for (uint8_t i = 0; i < sizeof(mcp) / sizeof(mcp[0]); i++)
            {
                uint8_t dataBit = mcp[i].readGPIOB() ^ 0xFF;
                dataInput_32t += dataBit << (i * 8);
                //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%d=%d\n", i, dataRead[i]);
                _DELAY_MS(1);
            }
            if (dataInputLast_32t != dataInput_32t)
            {
                xQueueSend(queueMCP230x7_Input, &dataInput_32t, portMAX_DELAY);
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
    Audio *audio = new Audio(false, I2S_DAC_CHANNEL_DISABLE, I2S_NUM_1);
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
                        //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放音樂!%s\n", doc["name"].as<String>().c_str());
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
        if (xSemaphoreTake(xMutex, portMAX_DELAY))
        {
            audio->loop();

            _DELAY_MS(1);
            xSemaphoreGive(xMutex);
        }
        vTaskDelay(3);
    }
}
