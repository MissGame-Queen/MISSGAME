#include "myFunction.h"
/*
void writeQRCODE(OLED_QRcode_t *oledQrcode, uint16_t maxsize)
{
  static const uint16_t NUM_ERROR_CORRECTION_CODEWORDS[4][40] = {
      // 1,  2,  3,  4,  5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36,   37,   38,   39,   40    Error correction level
      {17, 28, 44, 64, 88, 112, 130, 156, 192, 224, 264, 308, 352, 384, 432, 480, 532, 588, 650, 700, 750, 816, 900, 960, 1050, 1110, 1200, 1260, 1350, 1440, 1530, 1620, 1710, 1800, 1890, 1980, 2100, 2220, 2310, 2430}, // Low
      {13, 22, 36, 52, 72, 96, 108, 132, 160, 192, 224, 260, 288, 320, 360, 408, 448, 504, 546, 600, 644, 690, 750, 810, 870, 952, 1020, 1050, 1140, 1200, 1290, 1350, 1440, 1530, 1590, 1680, 1770, 1860, 1950, 2040},    // Medium
      {10, 16, 26, 36, 48, 64, 72, 88, 110, 130, 150, 176, 198, 216, 240, 280, 308, 338, 364, 416, 442, 476, 504, 560, 588, 644, 700, 728, 784, 812, 868, 924, 980, 1036, 1064, 1120, 1204, 1260, 1316, 1372},             // Quartile
      {7, 10, 15, 20, 26, 36, 40, 48, 60, 72, 80, 96, 104, 120, 132, 144, 168, 180, 196, 224, 224, 252, 270, 300, 312, 336, 360, 390, 420, 450, 480, 510, 540, 570, 570, 600, 630, 660, 720, 750},                         // High
  };

  if (oledQrcode->Text != "")
  {

    bool ifoversize = false;
    // 計算版本大小
    for (byte i = 0; i < 40; i++)
    {
      if (NUM_ERROR_CORRECTION_CODEWORDS[oledQrcode->Qrcode.ecc][i] > oledQrcode->Text.length())
      {
        oledQrcode->Qrcode.version = i + 1;
        i = 100;
      }
    }
    // 如果超出顯示範圍自動調整容錯
    uint16_t minsize = min(oledQrcode->display->getDisplayHeight() - oledQrcode->y, oledQrcode->display->getDisplayWidth() - oledQrcode->x);
    minsize = min(minsize, maxsize);

    if (oledQrcode->Qrcode.version * 4 + 17 + 2 > minsize)
    {
      ifoversize = true;
      for (int8_t j = 3; j >= 0; j--)
      {
        for (int8_t i = 39; i >= 0; i--)
        {
          if (NUM_ERROR_CORRECTION_CODEWORDS[j][i] > oledQrcode->Text.length() && (i + 1) * 4 + 17 + 2 <= minsize)
          {
            oledQrcode->Qrcode.version = i + 1;
            oledQrcode->Qrcode.ecc = j;
            i = -100;
            j = -100;
            ifoversize = false;
            // _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"oledQrcode->Qrcode.version=%d,oledQrcode->Qrcode.size=%d,oledQrcode->Qrcode.ecc=%d\n",oledQrcode->Qrcode.version,oledQrcode->Qrcode.version * 4 + 17+2,oledQrcode->Qrcode.ecc);
          }
        }
      }
    }
    oledQrcode->display->setDrawColor(1);
    // 如果沒適合的QRCODE 則顯示X
    if (ifoversize)
    {
      oledQrcode->display->drawBox(0, 0, 63, 63);
      oledQrcode->display->drawLine(0, 0, 63, 63);
      oledQrcode->display->drawLine(63, 0, 0, 63);
    }
    else
    {
      uint8_t qrcodeData[qrcode_getBufferSize(oledQrcode->Qrcode.version)];
      qrcode_initText(&oledQrcode->Qrcode, qrcodeData, oledQrcode->Qrcode.version, oledQrcode->Qrcode.ecc, oledQrcode->Text.c_str());

      byte gine = minsize / (oledQrcode->Qrcode.size + 2);
      byte X, Y;
      (oledQrcode->x >= 0) ? X = oledQrcode->x : X = (oledQrcode->display->getDisplayWidth() - (oledQrcode->Qrcode.size * gine + 2)) / 2;
      (oledQrcode->y >= 0) ? Y = oledQrcode->y : Y = (oledQrcode->display->getDisplayHeight() - (oledQrcode->Qrcode.size * gine + 2)) / 2;

      oledQrcode->display->drawBox(X, Y, oledQrcode->Qrcode.size * gine + 2, oledQrcode->Qrcode.size * gine + 2); // 反白
      oledQrcode->display->setDrawColor(0);
      for (byte x = X; x - X < oledQrcode->Qrcode.size * gine; x++)
      {
        for (byte y = Y; y - Y < oledQrcode->Qrcode.size * gine; y++)
        {
          bool zz = !qrcode_getModule(&oledQrcode->Qrcode, (x - X) / gine, (y - Y) / gine);
          if (!zz)
            oledQrcode->display->drawPixel(x + 1, y + 1);
        }
      }
    }
  }
}
*/

/**
 * @brief 將輸入的0~1536轉換成七彩的RGB
 *
 * @param value
 * @return uint32_t
 */
uint32_t setRainbowRGB(uint16_t value)
{
  uint8_t r = 0, g = 0, b = 0;
  if (value < 256) // 紅到黃
  {
    r = 255;
    g = value;
  }
  else if (value >= 256 && value < 512) // 黃到綠
  {
    r = (255 - (value - 256));
    g = 255;
  }
  else if (value >= 512 && value < 768) // 綠到碧
  {
    g = 255;
    b = (value - 512);
  }
  else if (value >= 768 && value < 1024) // 碧到藍
  {
    g = (255 - (value - 768));
    b = 255;
  }
  else if (value >= 1024 && value < 1280) // 藍到紫
  {
    r = (value - 1024);
    b = 255;
  }
  else if (value >= 1280 && value < 1536) // 紫到紅
  {
    r = 255;
    b = 255 - (value - 1280);
  }
  else
    ;

  return (r << 16) + (g << 8) + (b);
}

uint32_t setBrightnessRGB(uint32_t value, uint8_t brightness)
{
  uint8_t b = value & 0xFF,
          g = (value >> 8) & 0xFF,
          r = (value >> 16) & 0xFF;

  if (brightness != 255)
  {

    if (r != 0)
      r = (r * brightness) >> 8;
    if (g != 0)
      g = (g * brightness) >> 8;
    if (b != 0)
      b = (b * brightness) >> 8;
  }

  return b + (g << 8) + (r << 16);
}
/**
 * @brief
 *
 * @param strip
 * @param config{
 *
 * }
 */
void doLightRGB(Adafruit_NeoPixel *strip, JsonObject config)
{
  unsigned long nowTime = millis();
  uint8_t type = config["Type"].as<uint8_t>();
  uint32_t color = config["Color_B"].as<uint8_t>() +
                   (config["Color_G"].as<uint8_t>() << 8) +
                   (config["Color_R"].as<uint8_t>() << 16);
  uint8_t brightness = config["Brightness"].as<uint8_t>();
  if (type & (_Colorful + _Breathe + _Flashing))
  {

    uint16_t timeON = config["Brightness_Cycel_ON"].as<uint16_t>(),
             timeOFF = config["Brightness_Cycel_OFF"].as<uint16_t>();
    uint16_t value = nowTime % (timeON + timeOFF);
    if (type & _Breathe)
    {
      value <= timeON ? brightness = map(value, 0, timeON, 0, config["Brightness"].as<uint8_t>())
                      : brightness = map(value - timeON, 0, timeOFF, config["Brightness"].as<uint8_t>(), 0);
    }
    else if (type & _Flashing)
    {
      value <= timeON ? brightness = config["Brightness"].as<uint8_t>()
                      : brightness = 0;
    }
    if (type & _Colorful)
    {
      uint16_t cycle = config["Color_Cycle"].as<uint16_t>();
      color = setRainbowRGB(map(millis() % cycle, 0, cycle, 0, 1536));
    }
    color = setBrightnessRGB(color, brightness);
  }
  else
  {
    color = setBrightnessRGB(color, brightness);
  }
  for (uint8_t i = 0; i < strip->numPixels(); i++)
  {
    strip->setPixelColor(i, color);
  }
  if (xSemaphoreTake(rmtMutex, portMAX_DELAY))
  {
    strip->show();
    xSemaphoreGive(rmtMutex);
  }
}
void I2CScanner()
{
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
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
    else if (error == 4 && false)
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
  delay(5000); // wait 5 seconds for next scan
};
void ESP32_Info()
{
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\n"));
  // 获取芯片信息
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);
  // 打印芯片信息
  Serial.printf("Chip model: ESP32\n"); // 通常硬件型号不会在这里打印，但可以标记为 ESP32
  Serial.printf("CPU cores: %d\n", chip_info.cores);
  Serial.printf("CPU frequency: %dMHz\n", getCpuFrequencyMhz());
  // 打印芯片特性
  Serial.print("Features: ");
  if (chip_info.features & CHIP_FEATURE_WIFI_BGN)
    Serial.print("WiFi/ ");
  if (chip_info.features & CHIP_FEATURE_BLE)
    Serial.print("BLE/ ");
  if (chip_info.features & CHIP_FEATURE_BT)
    Serial.print("BT/ ");
  if (chip_info.features & CHIP_FEATURE_EMB_FLASH)
    Serial.print("Embedded Flash/ ");
  Serial.println();
  // 打印芯片版本
  Serial.printf("芯片版本: %d\n", chip_info.revision);

  // 打印Flash大小
  Serial.printf("Flash size: %dMB\n", spi_flash_get_chip_size() / (1024 * 1024));

  // 打印RAM大小
  size_t ram_size = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
  Serial.printf("RAM size: %dKB\n", ram_size / 1024);

  // 打印PSRAM大小（如果存在）
  size_t psram_size = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
  if (psram_size > 0)
  {
    Serial.printf("PSRAM size: %dMB\n", psram_size / (1024 * 1024));
    if (!psramInit())
    {
      Serial.println("Failed to initialize PSRAM");
    }

    if (!psramFound())
    {
      Serial.println("PSRAM not found");
    }
  }
  else
  {
    Serial.println("PSRAM not available");
  }
  Serial.printf("Total heap: %d\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM: %d\n", ESP.getFreePsram());
}