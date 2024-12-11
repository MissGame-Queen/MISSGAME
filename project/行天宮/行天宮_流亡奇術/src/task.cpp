#include "task.h"
void FloorMechanism(void *pvParam)
{
    enum stripType_Game_e
    {
        Standby = 1, // 待機狀態
        Start = 2,   // 遊戲開始狀態
        Correct = 4, // 步數正確
        Mistake = 8, // 步數錯誤
        Save = 16,   // 進度保存
        Finish = 32, // 完成
    };
    JsonDocument doc;
    JsonDocument *ptrDoc = &doc;
    const uint8_t pinOutput[] = {15, 14, 12, 13};
    const uint8_t pinInput[] = {36, 39, 34, 35};
    const uint8_t pinLED = 15, pinBCLK = 27, pinLRC = 25, pinDOUT = 26;
    const uint32_t pinInputMPC[12] = {mpcI_0_4, mpcI_0_7, mpcI_1_5, mpcI_1_4, mpcI_1_6, mpcI_0_3,
                                      mpcI_0_2, mpcI_0_1, mpcI_1_1, mpcI_1_3, mpcI_1_2, mpcI_0_6};
    const uint32_t pinOutputMPC[4]{mpcO_0_3, mpcO_0_2, mpcO_0_0, mpcO_0_1};

    const uint32_t pinMPC_Output_MODE = mpcO_0_4;           // 模式燈
    const uint32_t pinMPC_Output_DoorS = mpcO_0_5;          // 蜘蛛門
    const uint32_t pinMPC_Output_Door = mpcO_0_6;           // 女神門
    const uint32_t pinMPC_Output_StoneR = mpcO_1_0;         // R石像電源
    const uint32_t pinMPC_Output_StoneL = mpcO_1_1;         // L石像電源
    const uint32_t pinMPC_Output_Stone_Water = mpcO_1_2;    // 石像噴水訊號
    const uint32_t pinMPC_Input_StoneR = mpcI_2_4;          // R石像輸入
    const uint32_t pinMPC_Input_StoneL = mpcI_2_5;          // L石像輸入
    const uint32_t pinMPC_Input_Remote_Mode = mpcI_3_0;     // 模式切換_遙控器
    const uint32_t pinMPC_Input_Remote_Door = mpcI_3_1;     // 女神門_遙控器
    const uint32_t pinMPC_Input_Remote_DoorS = mpcI_3_2;    // 蜘蛛門_遙控器
    const uint32_t pinMPC_Input_Remote_StoneR = mpcI_3_4;   // R石像啟動_遙控器
    const uint32_t pinMPC_Input_Remote_StoneL = mpcI_3_5;   // L石像啟動_遙控器
    const uint32_t pinMPC_Input_Remote_StoneRST = mpcI_3_6; // 石像重置_遙控器
                                                            /*
                                                                const uint32_t pinMPC_Input_Remote_DoorS = mpcI_3_0;    // 蜘蛛門_遙控器
                                                                const uint32_t pinMPC_Input_Remote_StoneR = mpcI_2_1;   // R石像啟動_遙控器
                                                                const uint32_t pinMPC_Input_Remote_StoneL = mpcI_2_2;   // L石像啟動_遙控器
                                                                const uint32_t pinMPC_Input_Remote_StoneRST = mpcI_2_3; // 石像重置_遙控器
                                                            */

    bool havelock_Door = false;
    bool havelock_DoorS = true;
    bool isONStoneR = false;
    bool isONStoneL = false;
    bool isONStoneWater = false;
    const uint32_t ansGame1[4][4] = {
        {mpcI_1_5, 0, 0, 0},
        {mpcI_1_4, mpcI_0_3, 0, 0},
        {mpcI_0_2, mpcI_1_4, mpcI_1_2, 0},
        {mpcI_1_1, mpcI_0_6, mpcI_1_6, mpcI_0_7}};
    // 僅讀取12格輸入，避免遙控器輸入導致判讀錯誤
    uint32_t ansGame1_allbit = 0;
    for (uint8_t i = 0; i < 4; i++)
    {
        for (uint8_t j = 0; j < 4; j++)
        {
            ansGame1_allbit = ansGame1_allbit | ansGame1[i][j];
        }
    }
    const uint32_t ansGame2[2][12] = {{mpcI_0_3, mpcI_0_1, mpcI_1_1, mpcI_0_2, mpcI_1_6, mpcI_0_6,
                                       mpcI_0_4, mpcI_0_7, mpcI_1_2, mpcI_1_5, mpcI_1_3, mpcI_1_4},
                                      {mpcI_0_3, mpcI_0_1, mpcI_1_1, mpcI_1_6, mpcI_0_2, mpcI_0_6,
                                       mpcI_0_4, mpcI_0_7, mpcI_1_2, mpcI_1_5, mpcI_1_3, mpcI_1_4}};
    uint32_t ansGame2_allbit = 0;
    for (uint8_t i = 0; i < 2; i++)
    {
        for (uint8_t j = 0; j < 12; j++)
        {
            ansGame2_allbit = ansGame2_allbit | ansGame2[i][j];
        }
    }
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0;
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

            /*
                        if ((dataInput_8t & 1) && !(dataInputLast_8t & 1))
                        {
                            dataOutput_32t = dataOutput_32t | pinMPC_Output_MODE;
                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "流亡模式!%02X\n", dataOutput_32t);
                            stepGame = 0;
                        }
                        else if (!(dataInput_8t & 1) && (dataInputLast_8t & 1))
                        {
                            dataOutput_32t = dataOutput_32t & (~(pinMPC_Output_MODE));
                            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "奇術模式!%02X\n", dataOutput_32t);
                            stepGame = 0;
                        }
            */
            dataInputLast_8t = dataInput_8t;
        }

        bool isCorrect = false;
        //[v]流亡
        if (dataInput_8t & 1)
        {
            for (uint8_t i = 0; i < sizeof(pinInputMPC) / sizeof(pinInputMPC[0]); i++)
            {
                if (dataInput_32t & pinInputMPC[i])
                    strip.setPixelColor(i, 125, 125, 125);
                else
                    strip.setPixelColor(i, 0);
            }
            // 避免作弊一次採全部，只能採正確答案，一旦多採判定錯誤
            uint32_t valAns = (dataInput_32t & ansGame1_allbit);
            //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%08X,%08X,\n",ansGame1_allbit, valAns);
            switch (stepGame)
            {
            case 0:
            {
                first = true;
                stepGame++;
                havelock_Door = false;
                havelock_DoorS = true;
                isONStoneR = false;
                isONStoneL = false;
                isONStoneWater = false;
                dataOutput_32t = dataOutput_32t & (~(pinOutputMPC[0] + pinOutputMPC[1] +
                                                     pinOutputMPC[2] + pinOutputMPC[3]));
            }
            break;
            case 1:
            {

                if (valAns == ansGame1[stepGame - 1][0])
                {
                    stepGame++;
                    isCorrect = true;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[0];
                }
            }
            break;
            case 2:
            {
                if (valAns ==

                    (ansGame1[stepGame - 1][0] |
                     ansGame1[stepGame - 1][1]))
                {
                    stepGame++;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[1];
                    isCorrect = true;
                }
            }
            break;
            case 3:
            {
                if (valAns == (ansGame1[stepGame - 1][0] |
                               ansGame1[stepGame - 1][1] |
                               ansGame1[stepGame - 1][2]))
                {
                    stepGame++;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[2];
                    isCorrect = true;
                }
            }
            break;
            case 4:
            {
                if (valAns == (ansGame1[stepGame - 1][0] |
                               ansGame1[stepGame - 1][1] |
                               ansGame1[stepGame - 1][2] |
                               ansGame1[stepGame - 1][3]))
                {
                    stepGame++;
                    dataOutput_32t = dataOutput_32t | pinOutputMPC[3];
                }
            }
            break;
            case 5:
            {
                static uint32_t timer = millis();
                if (first)
                {
                    first = !first;
                    timer = millis();

                    doc["I2S"]["name"] = "/流亡_女神門開.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                    havelock_Door = true;

                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "女神門開!%d\n", timer);
                }
                if (millis() > timer + 21000)
                {
                    stepGame = 0;
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "音樂結束RST!\n");
                }
            }
            break;
            }

            if (stepGame != 5)
            {
                if (isCorrect)
                {
                    doc["I2S"]["name"] = "/流亡_踩對符號.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                    isCorrect = false;
                }
                else if ((dataInputLast_32t & 0xFFFF) < (dataInput_32t & 0xFFFF))
                {
                    doc["I2S"]["name"] = "/流亡_踩地板.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);

                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%08x!\n", dataInput_32t);
                }
            }
        }
        //[v]奇術
        else
        {
            static uint8_t ansGame2Index = 0;
            static uint32_t ansGame2Value = 0;
            static uint16_t valStone = 0;

            switch (stepGame)
            {
            case 0:
            {
                ansGame2Index = 0;
                ansGame2Value = 0;
                valStone = 0;
                for (uint8_t i = 0; i < sizeof(pinInputMPC) / sizeof(pinInputMPC[0]); i++)
                {
                    strip.setPixelColor(i, 0);
                }
                first = true;
                stepGame++;
                havelock_Door = false;
                havelock_DoorS = true;
                isONStoneR = false;
                isONStoneL = false;
                isONStoneWater = false;
                dataOutput_32t = dataOutput_32t & (~(pinOutputMPC[0] + pinOutputMPC[1] +
                                                     pinOutputMPC[2] + pinOutputMPC[3]));
            }
            break;
            case 1:
            {
                // 避免作弊一次採全部，只能採正確答案，一旦多採判定錯誤
                uint32_t valAns = (dataInput_32t & ansGame2_allbit);

                if ((valAns == ansGame2[0][ansGame2Index] && !(ansGame2Value & ansGame2[0][ansGame2Index])) ||
                    (valAns == ansGame2[1][ansGame2Index] && !(ansGame2Value & ansGame2[1][ansGame2Index])))
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
                    stepGame++;
                    first = true;
                }
            }
            break;
            case 2:
            {
                static uint32_t timer = 0;
                if (first)
                {
                    first = !first;
                    timer = millis();
                    doc["I2S"]["name"] = "/奇術_踩對行星順序.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);

                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "正確答案!%d\n", timer);
                }
                if (millis() > timer + 5000)
                {
                    stepGame++;
                    first = true;
                    //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "stepGame++!%d,%d\n", stepGame,first);
                }
            }
            break;
            case 3:
            {
                static uint32_t timer = 0;
                if (first)
                {
                    first = !first;
                    timer = millis();
                    doc["I2S"]["name"] = "/奇術_蜘蛛門開.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                    havelock_DoorS = false;
                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "蜘蛛門開!%d\n", timer);
                }
                if (millis() > timer + 5000)
                {
                    stepGame = 0;
                    first = true;
                }
            }
            break;
            }

            if (stepGame == 1)
            {
                if (isCorrect)
                {
                    doc["I2S"]["name"] = "/流亡_踩地板.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                    isCorrect = false;
                }
                else if (!(dataInput_32t & ansGame2Value) && (dataInputLast_32t & 0xFFFF) < (dataInput_32t & 0xFFFF))
                {
                    doc["I2S"]["name"] = "/奇術_踩錯行星順序.mp3";
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);

                    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%08x!\n", dataInput_32t);
                    stepGame = 0;
                }
            }
            // 如果左右石像同時有東西

            if (dataInput_32t & pinMPC_Input_StoneR && dataInput_32t & pinMPC_Input_StoneL && valStone < 100)
            {
                isONStoneWater = true;
                valStone++;
            }
            // 如果有人中途拿出來
            else if ((!(dataInput_32t & pinMPC_Input_StoneR) || !(dataInput_32t & pinMPC_Input_StoneL)) && valStone < 100)
            {
                isONStoneWater = false;
            }
            // 如果累計噴水達10S
            else if (valStone >= 100 && valStone < 300)
            {
                isONStoneWater = false;
                havelock_Door = true;
                valStone++;
            }
            // 開門20S後關門
            else if (valStone >= 300)
            {
                valStone = 0;
                havelock_Door = false;
                isONStoneR = false;
                isONStoneL = false;
            }
        }
        //[ ]遙控器

        // 如果模式訊號被觸發
        if ((~dataInput_32t) & pinMPC_Input_Remote_Mode && !((~dataInputLast_32t) & pinMPC_Input_Remote_Mode))
        {
            dataOutput_32t = dataOutput_32t | pinMPC_Output_MODE;
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "流亡模式!%02X\n", dataOutput_32t);
            stepGame = 0;
            first = true;
        }
        else if (!((~dataInput_32t) & pinMPC_Input_Remote_Mode) && (~dataInputLast_32t) & pinMPC_Input_Remote_Mode)
        {
            dataOutput_32t = dataOutput_32t & (~(pinMPC_Output_MODE));
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "奇術模式!%02X\n", dataOutput_32t);
            stepGame = 0;
            first = true;
        }

        if ((~dataInput_32t) & pinMPC_Input_Remote_DoorS)
        {
            dataOutput_32t = dataOutput_32t & (~pinMPC_Output_DoorS);
        }
        else
        {
            if (havelock_DoorS)
                dataOutput_32t = dataOutput_32t | (pinMPC_Output_DoorS);
            else
                dataOutput_32t = dataOutput_32t & (~pinMPC_Output_DoorS);
        }
        // 如果按下蜘蛛門
        if ((~dataInput_32t) & pinMPC_Input_Remote_DoorS)
        {
            dataOutput_32t = dataOutput_32t & (~pinMPC_Output_DoorS);
        }
        else
        {
            if (havelock_DoorS)
                dataOutput_32t = dataOutput_32t | (pinMPC_Output_DoorS);
            else
                dataOutput_32t = dataOutput_32t & (~pinMPC_Output_DoorS);
        }

        // 如果按下女神門
        if ((~dataInput_32t) & pinMPC_Input_Remote_Door)
        {
            dataOutput_32t = dataOutput_32t & (~pinMPC_Output_Door);
        }
        else
        {
            if (havelock_Door)
                dataOutput_32t = dataOutput_32t | (pinMPC_Output_Door);
            else
                dataOutput_32t = dataOutput_32t & (~pinMPC_Output_Door);
        }

        // 如果RST石像
        if ((~dataInput_32t) & pinMPC_Input_Remote_StoneRST)
        {
            isONStoneR = false;
            isONStoneL = false;
        }
        else
        {
            // 如果開啟右石像
            if ((~dataInput_32t) & pinMPC_Input_Remote_StoneR)
                isONStoneR = true;
            // 如果開啟左石像
            if ((~dataInput_32t) & pinMPC_Input_Remote_StoneL)
                isONStoneL = true;
        }

        // 右石像
        if (isONStoneR)
            dataOutput_32t = dataOutput_32t | (pinMPC_Output_StoneR);
        else
            dataOutput_32t = dataOutput_32t & (~pinMPC_Output_StoneR);
        // 左石像
        if (isONStoneL)
            dataOutput_32t = dataOutput_32t | (pinMPC_Output_StoneL);
        else
            dataOutput_32t = dataOutput_32t & (~pinMPC_Output_StoneL);
        // 石像噴水
        if (isONStoneWater)
            dataOutput_32t = dataOutput_32t | (pinMPC_Output_Stone_Water);
        else
            dataOutput_32t = dataOutput_32t & (~pinMPC_Output_Stone_Water);

        dataInputLast_32t = dataInput_32t;

        if (xSemaphoreTake(rmtMutex, portMAX_DELAY))
        {
            strip.show();
            _DELAY_MS(1);
            xSemaphoreGive(rmtMutex);
        }

        _DELAY_MS(100);
    }
}

void Dialla(void *pvParam)
{
    enum Status_e
    {
        _Reset,
        _Standby,
        _Sound1,
        _Finish,
        _DEBUG,
    };
    JsonDocument doc;
    JsonDocument *ptrDoc = &doc;
    const uint8_t pinOutput[] = {12, 13, 14, 15};
    const uint8_t pinInput[] = {36, 39, 34, 35};
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0;
    // const uint32_t pinMCP_Output_FinalDoor = (1 << 0);             // 最終大門
    // const uint32_t pinMCP_Input_ScanButton = (1 << 0);             // 掃描按鈕
    // const uint32_t pinMCP_Input_Remote_Projector = (1 << 7);       // 投影機遙控器
    const uint32_t pin_Input_Remote_A = (1 << 0); // 遙控器A鍵
    const uint32_t pin_Input_Remote_B = (1 << 1); // 遙控器B鍵
    for (uint8_t i = 0; i < 4; i++)
        pinMode(pinInput[i], INPUT);
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
            doc["I2S"]["name"] = "";
            doc["Serial"]["value"] = 0;
            doc["Serial"]["loop"] = false;
            xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
            first = true;
            stepGame++;
        }
        break;
        case _Standby:
            break;
        case _Sound1:
        {
            static uint32_t timer = 0;
            static uint8_t intsound = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                doc["I2S"]["name"] = "/mp3/001換紫光世界.mp3";
                intsound = 1;
                doc["Serial"]["value"] = intsound;
                doc["Serial"]["loop"] = false;
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
            }
            switch (intsound)
            {
            case 1:
                if (millis() > timer + 7000)
                {
                    doc["I2S"]["name"] = "/mp3/002你們是誰.mp3";
                    timer = millis();
                    intsound++;
                    doc["Serial"]["value"] = intsound;
                    doc["Serial"]["loop"] = false;
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                }
                break;
            case 2:
                if (millis() > timer + 11000)
                {
                    doc["I2S"]["name"] = "/mp3/003獅子.mp3";
                    timer = millis();
                    intsound++;
                    doc["Serial"]["value"] = intsound;
                    doc["Serial"]["loop"] = false;
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                }
                break;
            case 3:
                if (millis() > timer + 22000)
                {
                    doc["I2S"]["name"] = "/mp3/004達拉夫人.mp3";
                    timer = millis();
                    intsound++;
                    doc["Serial"]["value"] = intsound;
                    doc["Serial"]["loop"] = false;
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                }
                break;
            case 4:
                if (millis() > timer + 20000)
                {
                    first = true;
                    stepGame = _Reset;
                }
                break;
            }
        }
        break;
        case _Finish:
        {
            static uint32_t timer = 0;
            static uint8_t intsound = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                doc["I2S"]["name"] = "/mp3/005成功破除結界.mp3";
                intsound = 5;
                doc["Serial"]["value"] = intsound;
                doc["Serial"]["loop"] = false;
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
            }
            switch (intsound)
            {
            case 5:
                if (millis() > timer + 4000)
                {
                    doc["I2S"]["name"] = "/mp3/006恭喜你們.mp3";
                    timer = millis();
                    intsound++;
                    doc["Serial"]["value"] = intsound;
                    doc["Serial"]["loop"] = false;
                    xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                }
                break;
            case 6:
                if (millis() > timer + 33000)
                {
                    first = true;
                    stepGame = _Reset;
                }
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
        if ((~dataInput_8t) & pin_Input_Remote_A)
        {
            stepGame = _Sound1;
            first = true;
        }
        else if ((~dataInput_8t) & pin_Input_Remote_B)
        {
            stepGame = _Finish;
            first = true;
        }

        dataInputLast_32t = dataInput_32t;

        _DELAY_MS(100);
    }
}
