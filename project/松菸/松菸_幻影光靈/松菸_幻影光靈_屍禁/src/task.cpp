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
        _Reset,
        _1,
        _2,
        _3,
        _4,
        _COMPLETED,
        _FINISH,
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
        case _Reset:
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
            if (timer != 0 && millis() > timer + 20000)
            {
                timer = 0;
                dataOutput_32t = dataOutput_32t & (~pinMCP_Ooutout_NextDoor);
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
                            stepGame = _Reset;
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
                    //    stepGame = _Reset;
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
                stepGame = _Reset;
                first = true;
            }
        }
        // 如果按下完成鍵
        if ((~dataInput_32t) & pinMCP_Input_Remot_Completed||
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
        if (isFlash )
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
        _Reset,
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
        case _Reset:
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
                    stepGame = _Reset;
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
                    stepGame = _Reset;
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
                stepGame = _Reset;
                first = true;
            }
        }
        dataInputLast_32t = dataInput_32t;
        _DELAY_MS(50);
    }
}
/**
 * @brief 提燈程式
 *
 */
void task()
{
    const uint8_t pinColor[] = {11, 13, 12};
    const uint8_t pinInput[2] = {5, 4};
    const bool onSide = 1;
    Serial.begin(115200);
    for (uint8_t i = 0; i < 3; i++)
    {
        pinMode(pinColor[i], OUTPUT);
        digitalWrite(pinColor[i], !onSide);
    }
    delay(3000); // 用來等待上傳用

    for (uint8_t i = 0; i < 2; i++)
    {
        pinMode(pinInput[i], INPUT);
    }

    uint8_t stateLED = 0;
    bool swState = false;
    uint16_t cycleONOFF = 0;
    bool isChange = false;
    bool isSetLingth = false;
    const uint8_t timeON = 10, timeOFF = 10;
    uint32_t timer = 0;
    while (true)
    {
        bool onSwitch = digitalRead(pinInput[0]);
        bool offSwitch = digitalRead(pinInput[1]);
        if (offSwitch)
        {
            stateLED = 0;
            swState = false;
            isSetLingth = true;
        }
        else if (isSetLingth && !onSwitch)
        {
            isSetLingth = false;
        }
        else if (onSwitch && !swState && !isSetLingth)
        {
            swState = true;
            timer = millis();
        } // 若按了3秒以上關閉LED
        else if (swState && stateLED != 0 && millis() > timer + 1500)
        {
            stateLED = 0;
            swState = false;
            isSetLingth = true;
        }
        // 若原本是開啟狀態則變為閃爍
        else if (!onSwitch && swState)
        {
            swState = false;
            isSetLingth = true;
            if (stateLED == 1)
            {
                stateLED = 2;
                // 默認開啟LED
            }
            else
            {
                stateLED = 1;
            }
            cycleONOFF = 0;
            isChange = false;
            timer = millis();
        }

        if (stateLED == 1 && !isChange)
        {
            digitalWrite(pinColor[0], onSide);
            digitalWrite(pinColor[1], onSide);
            digitalWrite(pinColor[2], onSide);

            // Serial.println("ON");
            isChange = true;
        }
        else if (stateLED == 0 && !isChange)
        {
            digitalWrite(pinColor[0], !onSide);
            digitalWrite(pinColor[1], !onSide);
            digitalWrite(pinColor[2], !onSide);

            // Serial.println("OFF");
            isChange = true;
        }
        else
        {
            if (cycleONOFF < timeON && !isChange)
            {
                // Serial.println("AOUT ON");
                digitalWrite(pinColor[0], onSide);
                digitalWrite(pinColor[1], onSide);
                digitalWrite(pinColor[2], onSide);
                isChange = true;
            }
            else if (cycleONOFF > timeON && cycleONOFF < (timeON + timeOFF) && !isChange)
            {
                // Serial.println("AOUT OFF");
                digitalWrite(pinColor[0], !onSide);
                digitalWrite(pinColor[1], !onSide);
                digitalWrite(pinColor[2], !onSide);

                isChange = true;
            }
            else if (timeON == cycleONOFF || 0 == cycleONOFF)
            {
                isChange = false;
            }
        }
        cycleONOFF++;
        if (cycleONOFF > (timeON + timeOFF))
            cycleONOFF = 0;
        delay(10);
    }
}


#define _TYPE 1

#if _TYPE == 0
#include <W600WiFi.h>
#endif
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
//0:主控,1:從機

//"192.168.0.8:1833"
#define _MQTT_IPPORT "192.168.1.124:1833"
//"MissGAME_SOG"
#define _WIFI_SSID "MissGame_B2"
#define _WIFI_PASSWORD "missgame"
#define _MODULE_ID "100"

String clientId = "Lantern_" + String(_MODULE_ID);
const uint8_t pinColor[] = { 11, 13, 12 };
const bool onSide = 1;
const uint8_t pinOutput[] = { 5, 6 };
const uint8_t pinInput[] = { 4, 7, 8, 9 };
const uint8_t pinSoftwareSerial_RX = 2;
const uint8_t pinSoftwareSerial_TX = 3;
SoftwareSerial Player_Serial(pinSoftwareSerial_RX, pinSoftwareSerial_TX);
DFRobotDFPlayerMini myDFPlayer;
uint8_t intLingth = 0;
//uint8_t intShock = 0;
//uint8_t intSound = 0;
void myCMD(JsonObject *ptrObj) {
  JsonObject obj = (*ptrObj);
  serializeJsonPretty(obj, Serial);
  //燈光控制
  if (!obj["lingth"].isNull()) {
    uint16_t value = obj["lingth"].as<uint16_t>();
    Serial.print("燈光變更為:");
    Serial.println(value);
    intLingth = value;
  }
  //震動控制
  if (!obj["shock"].isNull()) {
    bool value = obj["shock"].as<bool>();
    Serial.print("震動變更為:");
    Serial.println(value);
    value ? digitalWrite(pinOutput[1], 1) : digitalWrite(pinOutput[1], 0);
    //intShock = value;
  }
  //音效控制
  if (!obj["sound"].isNull()) {
    if (!obj["sound"]["value"].isNull()) {
      uint16_t value = obj["sound"]["value"].as<uint16_t>();
      Serial.print("音效播放:");
      Serial.println(value);
      if (obj["sound"]["loop"].isNull() || !obj["sound"]["loop"].as<bool>()) {
        myDFPlayer.play(value);
        delay(20);
      } else {
        myDFPlayer.loop(value);
      }
      delay(20);
    }
    if (!obj["sound"]["volume"].isNull()) {
      myDFPlayer.volume(obj["sound"]["volume"].as<uint8_t>());
      delay(20);
    }

    //intSound = value;
    //myDFPlayer.loop(mp3Value);
  }
}
void Lingth(uint8_t value) {
  static uint8_t lastvalue = 0;
  static uint32_t timer = 0;
  static bool onoff = true;
  if (lastvalue != value) {
    lastvalue = value;
    if (0 == value)
      digitalWrite(pinOutput[0], 0);
    else if (1 == value)
      digitalWrite(pinOutput[0], 1);
  }
  if (value == 2) {
    if (millis() > timer + 200) {
      timer = millis();
      onoff = !onoff;
      digitalWrite(pinOutput[0], onoff);
    }
  }
}
#if _TYPE == 0
void MQTT_Callback(char *topic, uint8_t *payload, unsigned int length) {
  JsonDocument doc;
  JsonArray args = doc.to<JsonArray>();
  args.add(topic);
  String eventName = args[0].as<const char *>();
  DeserializationError error = deserializeJson(args[1], payload, length);
  // String str(payload, length);
  String str = "";
  for (uint16_t i = 0; i < length; i++) {
    str += char(payload[i]);
  }
  if (error) {
    Serial.print("反序列化失敗:");
    Serial.print(error.c_str());
    Serial.print("，以字串模式運行!\n");
  } else {
    //serializeJsonPretty((*doc), Serial);

    if (eventName == "MissGame" || eventName == _MODULE_ID) {

      // 如果封包不包含ids自動補進id
      if (!args[1]["ids"].isNull() && args[1]["id"].isNull()) {
        args[1]["ids"].add(args[1]["id"].as<uint16_t>());
      }
      // 或者event本身就是id則補進id
      else if (eventName == _MODULE_ID) {
        args[1]["ids"].add(String(_MODULE_ID).toInt());
      }

      JsonArray args = doc.as<JsonArray>();
      uint16_t id = 0;

      for (JsonVariant item : args[1]["ids"].as<JsonArray>()) {
        id = item.as<uint16_t>();
        // FIXME 補上自製模組的運作方法

        if (id == String(_MODULE_ID).toInt()) {
          switch (id) {
            case 100 ... 110:
              {
                JsonObject obj = args[1].as<JsonObject>();
                //myCMD(&obj);
                char tt[100];
                serializeJsonPretty(obj, tt);
                String trst = tt;
                //透過Serial傳輸Json控制從機
                Serial.println(trst);
              }
              break;
            default:
              Serial.print("無定義此ID:");
              Serial.println(id);
              break;
          }
        }
      }

    } /*
  else if (eventName == "Alive")
    ;
  else {
    Serial.print("無定義此事件:");
    Serial.println(eventName.c_str());
  }
  */
  }
  doc.clear();
}

void WiFi_Connect() {

  static uint32_t timer = 0;
  static bool first = true;
  uint8_t status = WiFi.status();
  if (timer == 0 && status != WL_CONNECTED) {
    timer = millis();
    WiFi.begin(_WIFI_SSID, _WIFI_PASSWORD);
    Serial.print(millis());
    Serial.print(",嘗試連線到:");
    Serial.println(_WIFI_SSID);
  } else if (timer != 0 && millis() > timer + 1000 && status != WL_CONNECTED) {
    digitalWrite(pinColor[0], onSide);
    digitalWrite(pinColor[1], !onSide);
    digitalWrite(pinColor[2], !onSide);
    Serial.print(".");
    timer = millis();

  } else if (timer != 0 && status == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi已接回:");
    Serial.println(WiFi.localIP());
    digitalWrite(pinColor[0], !onSide);
    timer = 0;
  } else if (first && status == WL_CONNECTED) {
    digitalWrite(pinColor[0], !onSide);
    first = false;
    Serial.println();
    Serial.println("初次WiFi連線:");
    Serial.println(WiFi.localIP());
  }
}
void MQTT_Connect() {
  String ip_port = _MQTT_IPPORT;
  static int colonIndex = ip_port.lastIndexOf(':');  // 找到最後一個冒號的位置
  if (colonIndex == -1) {
    Serial.print("MQTT網址錯誤!");
    Serial.print(ip_port.c_str());
    Serial.println();
    while (1) {
      delay(100);
    }
  }
  // 變數不能放條件內 不知為何會連不到
  static String ipStr = ip_port.substring(0, colonIndex);
  static String portStr = ip_port.substring(colonIndex + 1);  // 提取冒號後的子字符串
  static uint16_t port = portStr.toInt();                     // 將字符串轉換為uint16_t類型
  static WiFiClient WiFiClient;
  static PubSubClient MQTTClient(WiFiClient);
  static bool first = true;
  if (first) {
    first = false;
    MQTTClient.setServer(ipStr.c_str(), port);
    // 設定回呼函式
    MQTTClient.setCallback(MQTT_Callback);
  };
  if (WiFi.status() == WL_CONNECTED) {
    if (!MQTTClient.connected()) {
      static uint32_t timer = 0;
      Serial.print("正在嘗試MQTT連線...{");
      Serial.print(ipStr.c_str());
      Serial.print(",");
      Serial.print(port);
      Serial.print("}\n");
      digitalWrite(pinColor[1], !onSide);
      digitalWrite(pinColor[2], onSide);
      int8_t status = MQTTClient.connect((clientId != "" ? clientId.c_str() : String("W600-" + String(WiFi.macAddress())).c_str()));
      if (status > 0) {
        Serial.println("已連結到MQTT代理!重新訂閱主題");
        digitalWrite(pinColor[1], onSide);
        digitalWrite(pinColor[2], !onSide);
        MQTTClient.subscribe("MissGame");
        MQTTClient.subscribe(_MODULE_ID);
        timer = 0;
        // MQTTClient.publish("test/topic", String("{\"data\":\"Hello!I am " + clientId + "\"}").c_str());
      } else if (status <= 0 && timer == 0) {
        timer = millis();
      } else if (status <= 0 && timer != 0 && millis() > timer + 5000) {
        timer = millis();
        Serial.print("錯誤代碼:");
        Serial.print(MQTTClient.state());
        Serial.print("，5秒後重連....\n");
      }
    }
    MQTTClient.loop();
  }
}
#endif
void task() {
  Serial.begin(115200);
  delay(3000);  // 用來等待上傳用
  Serial.println("開始執行!");
//初始化
#if _TYPE == 0
  for (uint8_t i = 0; i < 3; i++) {
    pinMode(pinColor[i], OUTPUT);
    digitalWrite(pinColor[i], !onSide);
  }
  digitalWrite(pinColor[0], onSide);
#elif _TYPE == 1

  for (uint8_t i = 0; i < sizeof(pinInput); i++) {
    pinMode(pinInput[i], INPUT);
  }
  for (uint8_t i = 0; i < sizeof(pinOutput); i++) {
    pinMode(pinOutput[i], OUTPUT);
    digitalWrite(pinOutput[i], 0);
  }
  uint16_t delayTime = 20;
  Player_Serial.begin(9600);

  if (myDFPlayer.begin(Player_Serial, /*isACK = */ true, /*doReset = */ true)) {
    Serial.println("DFPlayer Mini 已連線\n");
    myDFPlayer.setTimeOut(500);  // Set serial communictaion time out 500ms
    delay(delayTime);
    myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    delay(delayTime);
    myDFPlayer.volume(30);
    delay(delayTime);
    myDFPlayer.disableLoop();
    delay(delayTime);
    myDFPlayer.stop();
  }

#endif

  uint16_t dalayTime = 10;
  while (true) {

#if _TYPE == 0
    // 保持WiFi連線
    WiFi_Connect();
    // 保持MQTT連線
    MQTT_Connect();
#elif _TYPE == 1
    if (Serial.available()) {
      delay(10);
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, Serial);
      JsonObject obj = doc.as<JsonObject>();
      myCMD(&obj);
    }
    bool onSwitch = digitalRead(pinInput[0]);
    bool offSwitch = digitalRead(pinInput[1]);
    static uint32_t timer = 0;
    //如果按下關閉
    if (offSwitch && intLingth != 0) {
      intLingth = 0;
    }
    //如果按下打開則紀錄時間
    else if (onSwitch && 0 == timer) {
      timer = millis();
    }
    //如果短按則切換模式或打開燈
    else if (!onSwitch && 0 != timer) {
      timer = 0;
      if (1 == intLingth) intLingth = 2;
      else if (2 == intLingth || 0 == intLingth) {
        intLingth = 1;
      }
    } else if (onSwitch && 0 != timer && millis() > timer + 3000) {
      timer = 0;
      intLingth = 0;
    }
    Lingth(intLingth);
#endif
    delay(dalayTime);
  }
}



void setup() {
  task();
}


void loop() {}

