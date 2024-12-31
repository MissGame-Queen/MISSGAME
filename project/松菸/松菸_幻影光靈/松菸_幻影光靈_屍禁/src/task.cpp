#include "task.h"
/**
 * @brief 通靈板程式
 *
 * @param pvParam
 */
void taskOuijaBoard(void *pvParam)
{

    enum Status_e
    {
        _RESET,
        _1,
        _2,
        _3,
        _4,
        _COMPLETED,
        _FINISH,
        _AUTO_RESET,
        _SAVERFID,
        _DEBUG,
    };
    const uint8_t pinOutput[] = {12, 13, 14, 15};
    const uint8_t pinInput[] = {36, 39, 34, 35};
    const uint8_t pinLED = 2;
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0xFF, dataOutputLast_32t = 0;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0, dataOutput_8t = 0, dataOutputLast_8t = 0xFF;
    JsonDocument doc;
    const JsonDocument *ptrDoc = &doc;

    const uint32_t pinMCP_Ooutout_SmallDoor = (1 << 0);
    const uint32_t pinMCP_Ooutout_NextDoor = (1 << 4);
    const uint32_t pinMCP_Ooutout_Lingth = (1 << 2);
    const uint32_t pinMCP_Ooutout_MirrorLingth = (1 << 3);
    const uint32_t pinMCP_Ooutout_Sound = (1 << 7);
    /*
    const uint32_t pinMCP_Input_Completed = (1 << 0);
    const uint32_t pinMCP_Input_Step = (1 << 1);
    const uint32_t pinMCP_Input_RE = (1 << 2);
    const uint32_t pinMCP_Input_SaveRFID = (1 << 3);
    */
    const uint32_t pinMCP_Input_Remot_Completed = (1 << 0);
    const uint32_t pinMCP_Input_Remote_Step = (1 << 1);
    const uint32_t pinMCP_Input_Remote_RE = (1 << 2);
    const uint32_t pinMCP_Input_Remote_SaveRFID = (1 << 3);
    // 32_t pinMCP_Input_Remote_Sound = (1 << 1);

    bool isFlash = false;
    for (uint8_t i = 0; i < sizeof(pinInput); i++)
    {
        pinMode(pinInput[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < sizeof(pinOutput); i++)
    {
        pinMode(pinOutput[i], OUTPUT);
        digitalWrite(pinOutput[i], 1);
    }
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, 1);
    //[ ]RFID初始化檢查
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
    //[ ]限定特定Tag功能
    SPIFFS.begin(true);
    File file;
    doc["Tag"].to<JsonArray>();
    file = SPIFFS.open("/Tag.json", "r");
    if (file)
    {
        DeserializationError error = deserializeJson(doc["Tag"], file);
        if (error)
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s\n", error.c_str());
            file.close();
            file = SPIFFS.open("/Tag.json", "w");
            if (!file)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "無法覆寫檔案!\n");
                _DEBUG_WHILE;
            }
            file.print("[]");
            file.close();
        }
    }
    else
    {
        file.close();
        file = SPIFFS.open("/Tag.json", "w");
        file.print("[]");
    }
    file.close();
    digitalWrite(pinLED, 0);

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
        case _RESET:
        {
            doc["Serial"]["value"] = 3;
            doc["Serial"]["loop"] = true;
            doc["I2S"]["name"] = "";
            xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
            digitalWrite(pinLED, 0);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
            isFlash = false;
            stepGame++;
            first = true;
            lastindexPN532 = 0xFF;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_SmallDoor;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_NextDoor;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Sound;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Lingth;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_MirrorLingth;
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
                    //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%d,%d\n", success, millis());
                    if (success)
                    {
                        digitalWrite(pinLED, 1);
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
                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%d]找到一張卡片!UID長度:%d,UID:%s\n", indexPN532, uidLength, testString.c_str());
                            bool found = false;
                            for (JsonVariant v : doc["Tag"].as<JsonArray>())
                            {
                                if (v == testString)
                                {
                                    found = true;
                                    break;
                                }
                            }

                            if (found)
                            {
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
                            else
                            {
                                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "未登記的卡片 %s ...\n", testString.c_str());
                            }
                        }
                    }
                    else
                    {
                        digitalWrite(pinLED, 0);
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
            static uint32_t timerDigitlSound = 0;
            static uint32_t timerDigitlSound_Long = 0;
            if (first)
            {
                digitalWrite(pinLED, 0);
                doc["Serial"]["value"] = 1;
                doc["Serial"]["loop"] = true;
                doc["I2S"]["name"] = "";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                isFlash = true;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_SmallDoor);
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_Sound);
                timerDigitlSound = millis();
                timerDigitlSound_Long = millis();
            }
            if (timerDigitlSound != 0 && millis() > timerDigitlSound + 500)
            {
                timerDigitlSound = 0;
                dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Sound;
            }
            if (timerDigitlSound_Long != 0 && millis() > timerDigitlSound_Long + 10000)
            {
                timerDigitlSound_Long = 0;
                doc["Serial"]["value"] = 3;
                doc["Serial"]["loop"] = true;
                doc["I2S"]["name"] = "";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
            }
        }
        break;
        case _FINISH:
        {
            static uint32_t timer = 0;
            if (first)
            {
                doc["Serial"]["value"] = 2;
                doc["Serial"]["loop"] = false;
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                isFlash = false;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_Lingth);
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_MirrorLingth);
                timer = millis();
            }
            // 開門並進入下一階段
            if (timer != 0 && millis() > timer + 20000)
            {
                timer = 0;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_NextDoor);
                stepGame++;
                first = true;
            }
        }
        break;
        case _AUTO_RESET:
        {
            static uint32_t timer = 0;
            if (first)
            {
                timer = millis();
            }
            if (timer != 0 && millis() > timer + 120000)
            {
                timer = 0;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_NextDoor);
                stepGame = _RESET;
                first = true;
            }
        }
        break;
        case _SAVERFID:
        {
            static uint32_t timer = 0;
            static bool ONscanning = true;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                // doc["I2S"]["name"] = "/mp3/燒錄模式.mp3";
                doc["Serial"]["value"] = 4;
                doc["Serial"]["loop"] = false;
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
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
                    //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%d,%d\n", success, millis());
                    if (success)
                    {
                        digitalWrite(pinLED, 1);
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
                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%d]找到一張卡片!UID長度:%d,UID:%s\n", indexPN532, uidLength, testString.c_str());
                            bool found = false;
                            for (JsonVariant v : doc["Tag"].as<JsonArray>())
                            {
                                if (v == testString)
                                {
                                    found = true;
                                    break;
                                }
                            }

                            if (found)
                            {
                                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已紀錄 %s \n", testString.c_str());
                            }
                            else
                            {
                                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "正在紀錄 %s ...\n", testString.c_str());
                                doc["Tag"].add(testString);
                                file.close();
                                file = SPIFFS.open("/Tag.json", "w");
                                if (!file)
                                {
                                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "無法打開檔案進行寫入\n");
                                    break;
                                }
                                serializeJsonPretty(doc["Tag"], file);
                                file.close();
                            }
                            // doc["I2S"]["name"] = "/mp3/燒錄成功.mp3";
                            doc["Serial"]["value"] = 5;
                            doc["Serial"]["loop"] = false;
                            xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                            _DELAY_MS(3000);
                            stepGame = _RESET;
                            first = true;
                        }
                    }
                    else
                    {
                        digitalWrite(pinLED, 0);
                    }
                    digitalWrite(pinOutput[indexPN532], 1);
                    xSemaphoreGive(SPIMutex);
                }
                indexPN532 = (indexPN532 + 1) % (sizeof(myPN532) / sizeof(myPN532[0]));
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
        if ((~dataInput_32t) & pinMCP_Input_Remote_Step ||
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
                    stepGame++;
                    break;
                case _FINISH:
                    //    stepGame = _RESET;
                    break;
                default:
                    stepGame = _COMPLETED;
                    break;
                }
            }
        }
        // 如果按下RE鍵
        if ((~dataInput_32t) & pinMCP_Input_Remote_RE ||
            dataInput_8t & pinMCP_Input_Remote_RE)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                stepGame = _RESET;
                first = true;
            }
        }
        // 如果按下完成鍵
        if ((~dataInput_32t) & pinMCP_Input_Remot_Completed ||
            dataInput_8t & pinMCP_Input_Remot_Completed)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                stepGame = _COMPLETED;
                first = true;
            }
        }
        dataInputLast_32t = dataInput_32t;
        static uint32_t timer_SaveRFID = 0;
        // 如果按下SaveRFID鍵
        if (0 == timer_SaveRFID && (~dataInput_32t) & pinMCP_Input_Remote_SaveRFID ||
            dataInput_8t & pinMCP_Input_Remote_SaveRFID)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                timer_SaveRFID = millis();
            }
        }
        // 如果按超過3秒就跳到燒錄模式
        else if (0 != timer_SaveRFID && millis() > timer_SaveRFID + 3000)
        {
            timer_SaveRFID = 0;
            if (((~dataInput_32t) & pinMCP_Input_Remote_SaveRFID || dataInput_8t & pinMCP_Input_Remote_SaveRFID))
            {
                stepGame = _SAVERFID;
                first = true;
            }
        }
        dataInputLast_32t = dataInput_32t;
        //[ ]燈光閃爍
        if (isFlash)
        {
            static bool ledONOFF = true;
            static uint32_t timer = 0, timerCheck = 0;
            if (millis() > timer + timerCheck)
            {
                timer = millis();
                ledONOFF = !ledONOFF;
                if (ledONOFF)
                {
                    dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Lingth;
                    timerCheck = random(300, 1500);
                }
                else
                {
                    dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_Lingth);
                    timerCheck = random(150, 750);
                }
            }
        }

        // strip.show();
        _DELAY_MS(50);
    }
}
/**
 * @brief 4順序按鈕開電磁鐵
 *
 * @param pvParam
 */
void task4SequenceButtons(void *pvParam)
{
    bool isOld = false;
    enum Status_e
    {
        _RESET,
        _1,
        _2,
        _3,
        _4,
        _FINISH,
        _DEBUG,
    };
    const uint8_t pinOutput[] = {14, 15};
    const uint8_t pinInput[] = {
        36,
        39,
        34,
        35, 12, 13};
    const uint8_t pinLED = 2;
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0xFF, dataOutputLast_32t = 0;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0, dataOutput_8t = 0, dataOutputLast_8t = 0xFF;
    JsonDocument doc;
    const JsonDocument *ptrDoc = &doc;

    const uint32_t pinMCP_Ooutout_Box = (1 << 0);
    const uint32_t pinMCP_Ooutout_light = (1 << 1);
    const uint32_t pinMCP_Input_Button[] = {
        (1 << 0), (1 << 1), (1 << 2), (1 << 3)};
    uint8_t ansMask = 0;
    for (uint8_t i = 0; i < sizeof(pinMCP_Input_Button) / sizeof(pinMCP_Input_Button[0]); i++)
        ansMask = ansMask | (pinMCP_Input_Button[i] & 0xFF);
    const uint32_t pinMCP_Input_Remote_RE = (1 << 4);
    const uint32_t pinMCP_Input_Remote_Step = (1 << 5);

    for (uint8_t i = 0; i < sizeof(pinInput); i++)
    {
        pinMode(pinInput[i], INPUT_PULLUP);
    }
    for (uint8_t i = 0; i < sizeof(pinOutput); i++)
    {
        pinMode(pinOutput[i], OUTPUT);
        digitalWrite(pinOutput[i], 1);
    }
    pinMode(pinLED, OUTPUT);
    digitalWrite(pinLED, 0);

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
        for (uint8_t i = 0; i < sizeof(pinInput); i++)
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
        case _RESET:
        {
            doc["Serial"]["value"] = 0;
            doc["Serial"]["loop"] = false;
            doc["I2S"]["name"] = "";
            xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
            digitalWrite(pinLED, 0);
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
            stepGame++;
            first = true;
            dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_light;
            dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_Box);
            dataOutput_8t = dataOutput_8t | pinMCP_Ooutout_light;
            dataOutput_8t = dataOutput_8t & (~pinMCP_Ooutout_Box);
        }
        break;
        case _1:
        case _2:
        case _3:
        case _4:
        {
            static bool swState = false;
            static uint8_t numTure = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                swState = false;
                numTure = 0;
            }
            // 套用遮罩避免其他輸入誤判
            uint32_t ansValue_8t = (dataInput_8t & ansMask);
            uint32_t ansValue_32t = (dataInput_32t & ansMask);
            // 若判斷按下按鈕
            if ((ansValue_8t > 0 || ansValue_32t > 0) && !swState)
            {
                swState = true;
                // 確保玩家一次只按一個按鈕才判斷正確
                if (ansValue_8t == pinMCP_Input_Button[stepGame - _1] ||
                    ansValue_32t == pinMCP_Input_Button[stepGame - _1])
                    numTure = 1;
                // 若在其他步驟按下按鈕1則維持在步驟2
                else if ((ansValue_8t == pinMCP_Input_Button[0] ||
                          ansValue_32t == pinMCP_Input_Button[0]))
                    numTure = 2;
                else
                    numTure = 0;
                if (isOld)
                    doc["I2S"]["name"] = "/mp3/0006按鈕按壓.mp3";
                // 若按下按鈕則播放音樂
                else if (ansValue_8t == pinMCP_Input_Button[1] ||
                         ansValue_32t == pinMCP_Input_Button[1])
                    doc["I2S"]["name"] = "/mp3/0001喜按鈕.mp3";
                else if (ansValue_8t == pinMCP_Input_Button[0] ||
                         ansValue_32t == pinMCP_Input_Button[0])
                    doc["I2S"]["name"] = "/mp3/0002怒按鈕.mp3";
                else if (ansValue_8t == pinMCP_Input_Button[2] ||
                         ansValue_32t == pinMCP_Input_Button[2])
                    doc["I2S"]["name"] = "/mp3/0003哀按鈕.mp3";
                else if (ansValue_8t == pinMCP_Input_Button[3] ||
                         ansValue_32t == pinMCP_Input_Button[3])
                    doc["I2S"]["name"] = "/mp3/0004樂按鈕.mp3";

                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
            }
            // 判斷放開按鈕
            else if (ansValue_8t == 0 && ansValue_32t == 0 && swState)
            {
                swState = false;
                // 若剛剛按下正確按鈕則進到下一流程
                if (numTure == 1)
                {
                    stepGame++;
                    first = true;
                }
                else if (stepGame == _2)
                {
                    stepGame = _2;
                    first = true;
                }
                // 若按錯回到步驟1
                else
                {
                    stepGame = _1;
                    first = true;
                }
            }
        }

        break;

        case _FINISH:
        {
            static uint32_t timer = 0;
            if (first)
            {
                _DELAY_MS(1000);
                if (isOld)
                    doc["I2S"]["name"] = "/mp3/0007成功.mp3";
                else
                    doc["I2S"]["name"] = "/mp3/0005喜怒哀樂成功.mp3";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_light);
                dataOutput_32t = dataOutput_32t | pinMCP_Ooutout_Box;
                dataOutput_8t = dataOutput_8t & (~pinMCP_Ooutout_light);
                dataOutput_8t = dataOutput_8t | pinMCP_Ooutout_Box;
                timer = millis();
            }
            if (isOld)
            {
                if (millis() > timer + 60000)
                {
                    stepGame = _RESET;
                }
            }
        }
        break;
        default:
            break;
        }
        //[ ]遙控器
        // 如果按下跳關鍵
        if (dataInput_32t & pinMCP_Input_Remote_Step ||
            (~dataInput_8t) & pinMCP_Input_Remote_Step)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                first = true;
                switch (stepGame)
                {
                case _FINISH:
                    stepGame = _RESET;
                    break;
                default:
                    stepGame = _FINISH;
                    break;
                }
            }
        }
        // 如果按下RE鍵
        if (dataInput_32t & pinMCP_Input_Remote_RE ||
            (~dataInput_8t) & pinMCP_Input_Remote_RE)
        {
            static uint32_t timer = 0;
            if (millis() > timer + 3000)
            {
                timer = millis();
                stepGame = _RESET;
                first = true;
            }
        }
        dataInputLast_32t = dataInput_32t;
        _DELAY_MS(50);
    }
}
