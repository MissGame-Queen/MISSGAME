#include <Adafruit_MCP23X17.h>
#include <MPU9250.h>

#include <Wire.h>
void I2C_Test()
{
  byte error, address;
  int nDevices;
  Serial.println("\nI2C Scanning...");

  nDevices = 0;
  for (address = 1; address < 127; address++)
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error == 4)
    {
      Serial.print("Unknown error at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.println(address, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  // wait 5 seconds for next scan
}
MPU9250 mpu;


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
void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Wire.begin();
  I2C_Test();
  // taskKeyword();
  taskMPU9250();
}

void loop()
{
}
