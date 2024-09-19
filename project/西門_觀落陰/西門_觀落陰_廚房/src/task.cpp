#include "task.h"
QueueHandle_t queueJson = xQueueCreate(1, sizeof(String));
QueueHandle_t queuePCM5102 = xQueueCreate(1, sizeof(String));
QueueHandle_t queueMCP230x7_Input = xQueueCreate(1, sizeof(uint32_t));
QueueHandle_t queueMCP230x7_Output = xQueueCreate(1, sizeof(uint32_t));
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
String strAudio = "";

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

void task(void *pvParam)
{
    const uint8_t mcpAddress[] = {0x27};
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

                strAudio = "{\"name\": \"/mp3/2-1.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
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
                strAudio = "{\"name\": \"/mp3/2-2.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
                intPlay = 0;
            }
            if (millis() > 4000 + timer && intPlay == 0)
            {
                dataOutput_32t = dataOutput_32t | pinMCP_Output_RunLight;
                intPlay++;
                strAudio = "{\"name\": \"/mp3/2-3.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
                timer = millis();
            }
            else if (millis() > 16000 + timer && intPlay == 1)
            {
                intPlay++;
                strAudio = "{\"name\": \"/mp3/2-4.wav\"}";
                xQueueSend(queuePCM5102, &strAudio, portMAX_DELAY);
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
void task2(void *pvParam)
{

    //Serial.begin(9600);
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
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[task]MCP Output=%08X\n", dataInput_32t);
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
