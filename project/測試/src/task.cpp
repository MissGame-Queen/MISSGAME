#include "task.h"
QueueHandle_t queuePCM5102 = xQueueCreate(1, sizeof(String));
SemaphoreHandle_t xMutex = xSemaphoreCreateMutex();
MPU9250 mpu;
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

  Audio *audio = (Audio *)pvParam;
  // Audio *audio = new Audio(false, I2S_DAC_CHANNEL_DISABLE, I2S_NUM_1);
  // audio->setPinout(pinBCLK, pinLRC, pinDOUT);
  // audio->setVolume(21); // 0...21

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
            _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "播放音樂!%s\n", doc["name"].as<String>().c_str());
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

void taskKeyword()
{
  Adafruit_MCP23X17 mcp;
  const char charKeywrod[4][4]{
      {'1', '2', '3', 'A'},
      {'4', '5', '6', 'B'},
      {'7', '8', '9', 'C'},
      {'*', '0', '#', 'D'}};
  enum loopType_e
  {
    Quadrant_1,
    Quadrant_2,
    Quadrant_3,
    Quadrant_4,
  };
  uint8_t charIndex = 0;
  loopType_e type = Quadrant_4;
  if (!mcp.begin_I2C(0x20))
  {
    Serial.println("找不到MCP230X7!");
    while (1)
      delay(100);
  }
  for (byte i = 0; i < 8; i++)
  {
    if (i < 4)
    {
      mcp.pinMode(i, OUTPUT);
      mcp.digitalWrite(i, HIGH);
    }
    else
      mcp.pinMode(i, INPUT_PULLUP); // turn on a 100K pullup internally
  }
  while (1)
  {
    switch (type)
    {
    case Quadrant_1:
      for (int8_t i = 0; i <= 3; i++)
      {
        mcp.digitalWrite(i, LOW);
        for (byte j = 4; j <= 7; j++)
        {
          if (!mcp.digitalRead(j))
            Serial.println(charKeywrod[charIndex / 4][charIndex % 4]);
          charIndex++;
        }
        mcp.digitalWrite(i, HIGH);
      }
      break;
      break;
    case Quadrant_2:
      for (int8_t i = 3; i >= 0; i--)
      {
        mcp.digitalWrite(i, LOW);
        for (byte j = 4; j <= 7; j++)
        {
          if (!mcp.digitalRead(j))
            Serial.println(charKeywrod[charIndex / 4][charIndex % 4]);
          charIndex++;
        }
        mcp.digitalWrite(i, HIGH);
      }
      break;
      break;
    case Quadrant_3:
      for (int8_t i = 0; i <= 3; i++)
      {
        mcp.digitalWrite(i, LOW);
        for (byte j = 7; j >= 4; j--)
        {
          if (!mcp.digitalRead(j))
            Serial.println(charKeywrod[charIndex / 4][charIndex % 4]);
          charIndex++;
        }
        mcp.digitalWrite(i, HIGH);
      }
      break;
      break;
    case Quadrant_4:
      for (int8_t i = 3; i >= 0; i--)
      {
        mcp.digitalWrite(i, LOW);
        for (byte j = 7; j >= 4; j--)
        {
          if (!mcp.digitalRead(j))
            Serial.println(charKeywrod[charIndex / 4][charIndex % 4]);
          charIndex++;
        }
        mcp.digitalWrite(i, HIGH);
      }
      break;
    }
    charIndex = 0;
    vTaskDelay(50);
  }
}
void taskMPU9250()
{

  Serial.begin(115200);
  Wire.begin();
  delay(2000);

  if (!mpu.setup(0x68))
  { // change to your own address
    while (1)
    {
      Serial.println("MPU 連線失敗。請使用 `connection_check` 範例檢查您的連線。");
      delay(5000);
    }
  }

  // calibrate anytime you want to
  Serial.println("加速陀螺儀校準將在5秒後開始。");
  Serial.println("請將裝置保持在平面上。");
  mpu.verbose(true);
  delay(5000);
  mpu.calibrateAccelGyro();

  Serial.println("磁力校準將在5秒後開始。");
  Serial.println("請以 8 字形揮舞設備直至完成。");
  delay(5000);
  mpu.calibrateMag();

  Serial.println("<校準參數>");
  Serial.printf("加速偏差[g]{X:%f,Y:%f,Z:%f}\n",
                mpu.getAccBiasX() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY,
                mpu.getAccBiasY() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY,
                mpu.getAccBiasZ() * 1000.f / (float)MPU9250::CALIB_ACCEL_SENSITIVITY);
  Serial.printf("陀螺儀偏移[deg/s]{X:%f,Y:%f,Z:%f}\n",
                mpu.getGyroBiasX() / (float)MPU9250::CALIB_GYRO_SENSITIVITY,
                mpu.getGyroBiasY() / (float)MPU9250::CALIB_GYRO_SENSITIVITY,
                mpu.getGyroBiasZ() / (float)MPU9250::CALIB_GYRO_SENSITIVITY);
  Serial.printf("磁偏差[mG]{X:%f,Y:%f,Z:%f}\n", mpu.getMagBiasX(), mpu.getMagBiasY(), mpu.getMagBiasZ());

  Serial.printf("磁力刻度[]{X:%f,Y:%f,Z:%f}\n", mpu.getMagScaleX(), mpu.getMagScaleY(), mpu.getMagScaleZ());

  mpu.verbose(false);

  while (1)
  {
    if (mpu.update())
    {
      static uint32_t prev_ms = millis();
      if (millis() > prev_ms + 25)
      {
        Serial.printf("偏航[%.2f]俯仰[%.2f]橫滾[%.2f]\n", mpu.getYaw(), mpu.getPitch(), mpu.getRoll());
        prev_ms = millis();
      }
    }
  }
}

void taskE_paper()
{
#define COLORED 0
#define UNCOLORED 1
  Epd epd;
  unsigned char image[1024];
  Paint paint(image, 0, 0);

  unsigned long time_start_ms;
  unsigned long time_now_s;

  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("e-Paper init and clear");
  epd.LDirInit();
  epd.Clear();

  paint.SetWidth(200);
  paint.SetHeight(24);

  Serial.println("e-Paper paint");
  paint.Clear(COLORED);
  paint.DrawStringAt(30, 4, "Hello world!", &Font16, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 10, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawStringAt(30, 4, "e-Paper Demo", &Font16, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 0, 30, paint.GetWidth(), paint.GetHeight());

  paint.SetWidth(64);
  paint.SetHeight(64);

  paint.Clear(UNCOLORED);
  paint.DrawRectangle(0, 0, 40, 50, COLORED);
  paint.DrawLine(0, 0, 40, 50, COLORED);
  paint.DrawLine(40, 0, 0, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 60, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledRectangle(0, 0, 40, 50, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 16, 130, paint.GetWidth(), paint.GetHeight());

  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(32, 32, 30, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 120, 130, paint.GetWidth(), paint.GetHeight());
  epd.DisplayFrame();
  delay(2000);

  Serial.println("e-Paper show pic");
  epd.HDirInit();
  // epd.Display(IMAGE_DATA);

  // Part display
  epd.HDirInit();
  epd.DisplayPartBaseImage(IMAGE_DATA);

  paint.SetWidth(50);
  paint.SetHeight(60);
  paint.Clear(UNCOLORED);

  char i = 0;
  char str[10][10] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
  for (i = 0; i < 10; i++)
  {
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(10, 10, str[i], &Font24, COLORED);
    epd.SetFrameMemoryPartial(paint.GetImage(), 80, 70, paint.GetWidth(), paint.GetHeight());
    epd.DisplayPartFrame();
    delay(100);
  }

  Serial.println("e-Paper clear and goto sleep");
  epd.HDirInit();
  epd.Clear();
  epd.Sleep();
}
void taskWS2812()
{
  Adafruit_NeoPixel strip(512, 12, NEO_GRB + NEO_KHZ800);
  strip.begin();
  for (uint16_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i,2,2,2);
  }
  strip.show();
  while(1);
}