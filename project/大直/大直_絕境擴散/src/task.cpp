#include <MissGame.h>

JsonDocument doc;
JsonDocument* ptrDoc=&doc;
/**
 * @brief 生化威脅掃描系統
 *
 * @param pvParam
 */
void task(void *pvParam)
{
    enum Status_e
    {
        _Reset,
        _PressButton,
        _PlaySound,
        _PlaySound_2,
        _AirDisinfection,
        _PressButton_2,
        _PlaySound_3,
        _AntidoteMachine,
        _PressButton_3,
        _PlaySound_4,
        _Finish,
        _DEBUG,
    };
    const uint8_t pinOutput[] = {12, 13, 14, 15};
const uint8_t pinInput[] = {36, 39, 34, 35};
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
                doc["name"]="/mp3/1-1.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
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
                                doc["name"]="/mp3/1-2.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
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
                                doc["name"]="/mp3/1-1.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 2 && millis() > 4000 && (millis() - 4000) > timer)
            {
                doc["name"]="/mp3/1-2.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 3 && millis() > 8000 && (millis() - 8000) > timer)
            {
                doc["name"]="/mp3/1-3.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
                playType++;
            }
            else if (playType == 4 && millis() > 5000 && (millis() - 5000) > timer)
            {
                doc["name"]="/mp3/1-3.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
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
                doc["name"]="/mp3/1-1.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
                havelock_Projector = true;
            }
            if (playType < 2 && millis() > 5000 && (millis() - 5000) > timer)
            {
                doc["name"]="/mp3/1-4.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
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
/**
 * @brief 空氣消毒系統
 *
 * @param pvParam
 */
void task2(void *pvParam)
{
    enum Status_e
    {
        _Reset,           // RE
        _DismissalHand,   // 放斷手
        _ButtonRaised,    // 按鈕升起
        _PuzzleWait,      // 等待謎題完成
        _PuzzleCompleted, // 謎題完成
        _Finish,
        _DEBUG,
    };
    const uint8_t pinOutput[] = {12, 13, 14, 15};
const uint8_t pinInput[] = {36, 39, 34, 35};
    uint8_t stepGame = 0;
    bool first = true;
    uint32_t dataInput_32t = 0, dataInputLast_32t = 0, dataOutput_32t = 0, dataOutputLast_32t = 0xFF;
    uint8_t dataInput_8t = 0, dataInputLast_8t = 0;
    uint8_t dataOutput_8t = 0, dataOutputLast_8t = 0xFF;
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
        case _Reset:
        {
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);

            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_LiftMotorPow);
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_LiftMotorDir);
            dataOutput_8t = dataOutput_8t & (~pinMCP_Output_LiftMotorPow);
            dataOutput_8t = dataOutput_8t & (~pinMCP_Output_LiftMotorDir);
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_RunLight);
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_GameFinsh);
            first = true;
            stepGame++;
        }
        case _DismissalHand:
        {
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
            }
            if (dataInput_32t & pinMCP_Input_ReedSwitch)
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _ButtonRaised:
        {
            static uint32_t timer = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                dataOutput_32t = dataOutput_32t | pinMCP_Output_LiftMotorDir;
                dataOutput_8t = dataOutput_8t | pinMCP_Output_LiftMotorDir;

                doc["name"]="/mp3/2-1.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
            }
            if (millis() > 16000 + timer)
            {
                stepGame++;
                first = true;
            }
            // 到下一階段前先關閉馬達電源
            else if (millis() > 15000 + timer)
            {
                dataOutput_32t = dataOutput_32t & (~pinMCP_Output_LiftMotorPow);
                dataOutput_8t = dataOutput_8t & (~pinMCP_Output_LiftMotorPow);
            }
            // 方向設定完後0.1S才啟動馬達電源
            else if (millis() > 100 + timer)
            {
                dataOutput_32t = dataOutput_32t | pinMCP_Output_LiftMotorPow;
                dataOutput_8t = dataOutput_8t | pinMCP_Output_LiftMotorPow;
            }
        }
        break;
        case _PuzzleWait:
        {
            static uint32_t sw = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                sw = dataInput_32t & pinMCP_Input_Finsh;
            }
            // 收到正常過關訊號或者主控的跳關訊號或者主版上的測試按鈕
            if ((dataInput_32t & pinMCP_Input_PuzzleCompleted) ||
                sw != (dataInput_32t & pinMCP_Input_Finsh) ||
                (dataInput_8t & pinMCP_Input_PuzzleCompleted))
            {
                stepGame++;
                first = true;
            }
        }
        break;
        case _PuzzleCompleted:
        {
            static uint32_t timer = 0;
            static uint8_t intPlay = 0;
            if (first)
            {
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "遊戲流程%d\n", stepGame);
                first = false;
                doc["name"]="/mp3/2-2.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
                intPlay = 0;
            }
            if (millis() > 4000 + timer && intPlay == 0)
            {
                dataOutput_32t = dataOutput_32t | pinMCP_Output_RunLight;
                intPlay++;
                doc["name"]="/mp3/2-3.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
            }
            else if (millis() > 16000 + timer && intPlay == 1)
            {
                intPlay++;
                doc["name"]="/mp3/2-4.wav";
                xQueueSend(queuePCM5102, &ptrDoc, portMAX_DELAY);
                timer = millis();
            }
            else if (millis() > 4000 + timer && intPlay == 2)
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
                dataOutput_32t = dataOutput_32t & (~pinMCP_Output_LiftMotorDir);
                dataOutput_8t = dataOutput_8t & (~pinMCP_Output_LiftMotorDir);
                dataOutput_32t = dataOutput_32t | pinMCP_Output_GameFinsh;

                timer = millis();
            }
            if (millis() > 20000 + timer)
            {
                dataOutput_32t = dataOutput_32t & (~pinMCP_Output_LiftMotorPow);
                dataOutput_8t = dataOutput_8t & (~pinMCP_Output_LiftMotorPow);
                stepGame = _DEBUG; // HACK 取消自動RE
                first = true;
            }
            else if (millis() > 100 + timer)
            {
                dataOutput_32t = dataOutput_32t | pinMCP_Output_LiftMotorPow;
                dataOutput_8t = dataOutput_8t | pinMCP_Output_LiftMotorPow;
            }
        }
        case _DEBUG:
            break;
            break;
        default:
            break;
        }
        //[ ]遙控器

        // 主控傳來的通風管訊號
        if (dataInput_32t & pinMCP_Input_VentilationDuct)
            dataOutput_32t = dataOutput_32t | pinMCP_Output_VentilationDuct;
        else
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_VentilationDuct);
        // 主控傳來的氣動門訊號
        if (dataInput_32t & pinMCP_Input_Door)
            dataOutput_32t = dataOutput_32t | pinMCP_Output_Door;
        else
            dataOutput_32t = dataOutput_32t & (~pinMCP_Output_Door);

        dataInputLast_32t = dataInput_32t;

        // strip.show();
        _DELAY_MS(100);
    }
}

// ################消毒系統上方16*16遊戲程式變數################
#include <Arduino.h>
// const uint8_t pinSwitch[2][4] = {{A0, A1, A2, A3}, {A8, A9, A10, A11}};
// const uint8_t pinLingth[2][4] = {{A4, A5, A6, A7}, {A12, A13, A14, A15}};
const uint8_t pinSwitch[2][4] = {};
const uint8_t pinLingth[2][4] = {};
const uint8_t pinOut = 13;
// uint8_t arrLingth[4]{0, 0, 0, 0};
uint16_t bitLingth = 0X6B9E;               // 16*16點亮位數
uint16_t bitSwitch = 0, bitLastSwitch = 0; // 16*16點按壓值
uint32_t timer = 0;
bool isFinsh = false;
// ################消毒系統上方16*16遊戲程式變數################
void setLingth()
{
    static uint8_t valX = 0, valY = 0;
    digitalWrite(pinLingth[0][valY], 1);
    if (((uint16_t)(1 << (valX + valY * 4))) & bitLingth)
    {
        digitalWrite(pinLingth[1][valX], 0);
        delay(1);
        digitalWrite(pinLingth[1][valX], 1);
    }
    digitalWrite(pinLingth[0][valY], 0);
    valX = (valX + 1) % 4;
    if (valX == 0)
        valY = (valY + 1) % 4;
}

uint16_t getButten()
{
    static uint8_t valX = 0, valY = 0;
    digitalWrite(pinSwitch[0][valY], 0);
    delay(1);
    bool state = !digitalRead(pinSwitch[1][valX]);
    if (state)
    {
        bitSwitch = bitSwitch | (1 << (valX + valY * 4));
    }
    else if (!state && bitSwitch & (1 << (valX + valY * 4)))
    {
        bitSwitch = bitSwitch & (~((uint16_t)(1 << (valX + valY * 4))));
    }
    digitalWrite(pinSwitch[0][valY], 1);
    valX = (valX + 1) % 4;
    if (valX == 0)
        valY = (valY + 1) % 4;
    return bitSwitch;
}
uint16_t check()
{
    uint16_t ans = bitSwitch - bitLastSwitch;
    uint16_t valReturn = bitLingth;
    // 如果全點亮則輸出訊號

    Serial.println(ans, HEX);
    // 按的按鈕燈光狀態反轉
    valReturn = valReturn ^ ans;
    // 周邊燈光反轉
    for (uint8_t i = 0; i < 16; i++)
    {
        //
        bool isAns = ans & (1 << i);
        if (isAns)
        {
            // 如果不是左側邊緣,點亮左邊
            if (i % 4 != 0)
            {
                valReturn = valReturn ^ (1 << (i - 1));
            }
            // 如果不是左側邊緣,點亮右邊
            if (i % 4 != 3)
            {
                valReturn = valReturn ^ (1 << (i + 1));
            }
            // 如果不是上側邊緣,點亮上邊
            if (i > 3)
            {
                valReturn = valReturn ^ (1 << (i - 4));
            }
            // 如果不是上側邊緣,點亮上邊
            if (i < 12)
            {
                valReturn = valReturn ^ (1 << (i + 4));
            }
        }
    }
    if (valReturn == 0xFFFF)
    {
        Serial.println("完成");
        digitalWrite(pinOut, 0);
        isFinsh = true;
        timer = millis();
    }
    return valReturn;
}
void (*resetFunc)(void) = 0;
void task2_2(void *pvParam)
{

    // Serial.begin(9600);
    pinMode(pinOut, OUTPUT);
    // pinMode(12, INPUT_PULLUP);
    digitalWrite(pinOut, 1);
    for (uint8_t i = 0; i < 4; i++)
    {
        pinMode(pinSwitch[0][i], OUTPUT);
        pinMode(pinSwitch[1][i], INPUT_PULLUP);
        pinMode(pinLingth[0][i], OUTPUT);
        pinMode(pinLingth[1][i], OUTPUT);
        digitalWrite(pinSwitch[0][i], 1);
        digitalWrite(pinLingth[0][i], 0);
        digitalWrite(pinLingth[1][i], 1);
    }
    while (true)
    {
        getButten(); // 獲取按鈕狀態
        setLingth(); // 輸出燈號
        if (millis() > timer + 20000)
        {
            Serial.println("RE");
            delay(100);
            resetFunc(); // 重置系統
        }
        // 如果按鈕被按下
        if (bitLastSwitch != bitSwitch)
        {

            // 僅紀錄按下，放開不紀錄
            if (bitSwitch > bitLastSwitch)
            {

                // bitLingth = getButten();
                bitLingth = check();
            }
            bitLastSwitch = bitSwitch;
        }
    }
}
