#include "task.h"

void task(void *pvParam)
{
    enum Status_e
    {
        _Reset,
        _1,
        _2,
        _3,
        _4,
        _COMPLETED,
        _PLAYSOUND,
        _FINISH,
        _DEBUG,
    };
    const uint8_t mcpAddress[] = {0x27};
    const uint8_t pinOutput[] = {12, 13, 14, 15};
    const uint8_t pinInput[] = {36, 39, 34, 35};
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0, dataOutput_8t = 0, dataOutputLast_8t = 0xFF;
    JsonDocument doc;
    const JsonDocument *ptrDoc = &doc;

    const uint32_t pinMCP_Ooutout_SmallDoor = (1 << 0);
    const uint32_t pinMCP_Ooutout_NextDoor = (1 << 1);
    const uint32_t pinMCP_Ooutout_Sound = (1 << 7);
    const uint32_t pinMCP_Ooutout_Lingth = (1 << 3);
    const uint32_t pinMCP_Input_Remote_Step = (1 << 0);
    const uint32_t pinMCP_Input_Remote_Sound = (1 << 1);
    const uint32_t pinMCP_Input_Remote_RE = (1 << 2);

    for (uint8_t i = 0; i < sizeof(pinInput); i++)
    {
        pinMode(pinInput[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < sizeof(pinOutput); i++)
    {
        pinMode(pinOutput[i], OUTPUT);
        digitalWrite(pinOutput[i], 1);
    }
    pinMode(2, OUTPUT);
    digitalWrite(2, 0);
    Adafruit_PN532 myPN532[] = {
        Adafruit_PN532(pinOutput[0], &SPI),
        Adafruit_PN532(pinOutput[1], &SPI),
        Adafruit_PN532(pinOutput[2], &SPI),
        Adafruit_PN532(pinOutput[3], &SPI),
    };
    if (xSemaphoreTake(SPIMutex, portMAX_DELAY))
    {

        // Adafruit_PN532 myPN532[] = {Adafruit_PN532(pinOutput[0],&SPI)};
        for (uint8_t i = 0; i < (sizeof(myPN532) / sizeof(myPN532[0])); i++)
        {
            digitalWrite(pinOutput[i], 0);
            myPN532[i].begin();
            uint32_t versiondata = 0;
            versiondata = myPN532[i].getFirmwareVersion();
            while (!versiondata)
            {
                myPN532[i].begin();
                versiondata = myPN532[i].getFirmwareVersion();
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%d]找不到PN5xx!\n", i);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }

            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%d]找到PN5%02X!硬體版本:%d.%d\n", i, (versiondata >> 24) & 0xFF, (versiondata >> 16) & 0xFF, (versiondata >> 8) & 0xFF);
            // 設定從卡片讀取的最大重試次數
            // 這可以防止我們永遠等待一張卡，即
            // PN532 的預設行為。
            myPN532[i].setPassiveActivationRetries(0xFF);
            myPN532[i].SAMConfig();
            digitalWrite(pinOutput[i], 1);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        xSemaphoreGive(SPIMutex);
    }
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "等待ISO14443A卡\n");
    uint8_t indexPN532 = 0, lastindexPN532 = 0xFF;

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
        case _Reset:
        {
            doc["Serial"]["value"] = 0;
            doc["Serial"]["loop"] = false;
            doc["I2S"]["name"] = "";
            xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
            digitalWrite(2, 0);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
            stepGame++;
            first = true;
            lastindexPN532 = 0xFF;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_SmallDoor;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_NextDoor;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Sound;
        }
        break;
        case _1:
        case _2:
        case _3:
        case _4:
        {
            static uint32_t timer = 0;
            static bool ONscanning = true;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                // xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                ONscanning = true;
                timer = 0;
            }
            if (!ONscanning && millis() > timer + 1000)
            {
                ONscanning = true;
            }
            if (ONscanning)
            {
                if (xSemaphoreTake(SPIMutex, portMAX_DELAY))
                {
                    bool success = false;
                    uint8_t uid[7] = {0, 0, 0, 0, 0, 0, 0}; // Buffer to store the returned UID
                    uint8_t uidLength = 0;
                    digitalWrite(pinOutput[indexPN532], 0);
                    success = myPN532[indexPN532].readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 50);

                    if (success)
                    {
                        ONscanning = false;
                        timer = millis();
                        // 避免連續讀取,關閉掃描功能直至開啟
                        if (lastindexPN532 != indexPN532)
                        {
                            lastindexPN532 = indexPN532;
                            String fmt = "%02X";
                            char teststr[fmt.length() - 2];
                            String testString = "";
                            for (uint8_t i = 0; i < uidLength; i++)
                            {
                                snprintf(teststr, fmt.length() - 1, fmt.c_str(), uid[i]);
                                testString += teststr;
                                i == uidLength - 1 ? testString += "" : testString += " ";
                            }
                            // _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%d]找到一張卡片!UID長度:%d,UID:%s\n", indexPN532, uidLength, testString.c_str());
                            if ((stepGame - _1) == indexPN532)
                            {
                                stepGame++;
                                first = true;
                            }
                            else if (indexPN532 == 0)
                            {
                                stepGame = _2;
                                first = true;
                            }
                            else
                            {
                                stepGame = _1;
                                first = true;
                            }
                        }
                    }
                    digitalWrite(pinOutput[indexPN532], 1);
                    xSemaphoreGive(SPIMutex);
                }
                indexPN532 = (indexPN532 + 1) % (sizeof(myPN532) / sizeof(myPN532[0]));
            }
        }
        break;
        case _COMPLETED:
        {
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                doc["Serial"]["value"] = 1;
                doc["Serial"]["loop"] = false;
                doc["I2S"]["name"] = "/mp3/0001通靈板成功.mp3";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_SmallDoor);
            }
        }
        break;
        case _PLAYSOUND:
        {
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_Sound);

                timer = millis();
            }
            if (timer != 0 && millis() > timer + 500)
            {
                timer = 0;
                dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Sound;
            }
        }
        case _FINISH:
        {
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_NextDoor);
            }
        }
        break;
        default:
            break;
        }
        //[ ]遙控器
        // 如果按下播放語音
        /*
        if (dataInput_32t & pinMCP_Input_Remote_Sound ||
            dataInput_8t & pinMCP_Input_Remote_Sound)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Sound;
            }
        }
        */
        // 如果按下跳關鍵
        if (dataInput_32t & pinMCP_Input_Remote_Step ||
            dataInput_8t & pinMCP_Input_Remote_Step)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                first = true;
                switch (stepGame)
                {
                case _COMPLETED:
                case _PLAYSOUND:
                    stepGame++;
                    break;
                case _FINISH:
                    stepGame = _Reset;
                    break;
                default:
                    stepGame = _COMPLETED;
                    break;
                }
            }
        }
        // 如果按下RE鍵
        if (dataInput_32t & pinMCP_Input_Remote_RE ||
            dataInput_8t & pinMCP_Input_Remote_RE)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                stepGame = _Reset;
                first = true;
            }
        }
        dataInputLast_32t = dataInput_32t;

        // strip.show();
        _DELAY_MS(50);
    }
}
