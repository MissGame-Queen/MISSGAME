#include <Template.h>
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>
#include "Audio.h"
#include <DFRobotDFPlayerMini.h>
const uint8_t pinfragments[] = {21, 19, 18, 5, 17, 16, 4, 32, 33, 25, 26, 27, 14, 12, 35};
const uint8_t pinRemoteControl[] = {39, 34};
const uint8_t pinSound = 13;
const uint8_t pinSoundTX = 15;
const uint8_t pinSoundRX = 13;
const uint8_t pinDoor = 23;
const uint8_t pinLight = 22;
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
void printDetail(uint8_t type, int value)
{
  switch (type)
  {
  case TimeOut:
    Serial.println(F("Time Out!"));
    break;
  case WrongStack:
    Serial.println(F("Stack Wrong!"));
    break;
  case DFPlayerCardInserted:
    Serial.println(F("Card Inserted!"));
    break;
  case DFPlayerCardRemoved:
    Serial.println(F("Card Removed!"));
    break;
  case DFPlayerCardOnline:
    Serial.println(F("Card Online!"));
    break;
  case DFPlayerUSBInserted:
    Serial.println("USB Inserted!");
    break;
  case DFPlayerUSBRemoved:
    Serial.println("USB Removed!");
    break;
  case DFPlayerPlayFinished:
    Serial.print(F("Number:"));
    Serial.print(value);
    Serial.println(F(" Play Finished!"));
    break;
  case DFPlayerError:
    Serial.print(F("DFPlayerError:"));
    switch (value)
    {
    case Busy:
      Serial.println(F("Card not found"));
      break;
    case Sleeping:
      Serial.println(F("Sleeping"));
      break;
    case SerialWrongStack:
      Serial.println(F("Get Wrong Stack"));
      break;
    case CheckSumNotMatch:
      Serial.println(F("Check Sum Not Match"));
      break;
    case FileIndexOut:
      Serial.println(F("File Index Out of Bound"));
      break;
    case FileMismatch:
      Serial.println(F("Cannot Find File"));
      break;
    case Advertise:
      Serial.println(F("In Advertise"));
      break;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

DFRobotDFPlayerMini myDFPlayer;
void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, pinSoundRX, pinSoundTX);
  bool state = false;
  state = myDFPlayer.begin(Serial2, /*isACK = */ true, /*doReset = */ true);

  if (state)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "DFPlayer Mini 已連線\n");
    myDFPlayer.setTimeOut(500); // Set serial communictaion time out 500ms
    // myDFPlayer.outputDevice(DFPLAYER_DEVICE_SD);
    // myDFPlayer.volume(15);
    myDFPlayer.play(1);
  }
  Serial.println("TEST");
  for (size_t i = 0; i < sizeof(pinfragments); i++)
    pinMode(pinfragments[i], INPUT_PULLUP);
  for (size_t i = 0; i < sizeof(pinRemoteControl); i++)
    pinMode(pinRemoteControl[i], INPUT_PULLUP);

  pinMode(pinSound, OUTPUT);
  pinMode(pinDoor, OUTPUT);
  pinMode(pinLight, OUTPUT);
  digitalWrite(pinDoor, HIGH);
  digitalWrite(pinSound, HIGH);
}
uint8_t status = 0;
Adafruit_NeoPixel pixels(360, pinLight, NEO_GRB + NEO_KHZ800);

uint8_t lastnum = 0;
enum status_e
{
  Reset,
  Standby,
  Start,
  Finish
};

void loop()
{
  static uint32_t timer = 0;
  if (!digitalRead(pinRemoteControl[0]))
  {
    status = Reset;
    timer = millis();
  }
  if (!digitalRead(pinRemoteControl[1]))
  {
    status = Start;
    timer = millis();
  }
  // Serial.println(status);
  switch (status)
  {
  case Reset:
    status++;
    lastnum = 0;
    break;
  case Standby:
  {
    uint8_t num = 0;
    for (uint8_t i = 0; i < sizeof(pinfragments); i++)
    {
      if (!digitalRead(pinfragments[i]))
      {
        num++;
        for (uint8_t j = 0; j < 24; j++)
          pixels.setPixelColor(i * 24 + j, 255, 255, 0);
        // Serial.println(pinfragments[i]);
      }
      else
      {
        for (uint8_t j = 0; j < 24; j++)
          pixels.setPixelColor(i * 24 + j, 0);
      }
    }
    if (num != lastnum)
    {
      if (num > lastnum)
      {
        /*
        digitalWrite(pinSound, LOW);
        vTaskDelay(200);
        digitalWrite(pinSound, HIGH);
        vTaskDelay(200);
        */
        /*
         myDFPlayer.disableLoop();
         myDFPlayer.stop();
         */
         myDFPlayer.play(1);
      }
      lastnum = num;
      Serial.println(num);
    }
    if (num == sizeof(pinfragments))
    {
      status++;
      timer = millis();
      lastnum = 0;
      digitalWrite(pinDoor, 0);
      vTaskDelay(100);
      digitalWrite(pinDoor, 1);
      vTaskDelay(100);
    }
    break;
  }
  case Start:
  {
    for (uint16_t i = 0; i < pixels.numPixels(); i++)
    {
      pixels.setPixelColor(i, setRainbowRGB(map((i + map(millis() % 2000, 0, 2000, 0, pixels.numPixels())) % pixels.numPixels(), 0, pixels.numPixels(), 1, 1535)));
    }
    if (millis() > timer + 11500)
    {
      for (uint16_t i = 0; i < pixels.numPixels(); i++)
      {
        pixels.setPixelColor(i, 55, 55, 0);
      }
      status++;
    }
  }

  break;
  case Finish:
    break;
  }
  pixels.show();
  if (myDFPlayer.available())
  {
    printDetail(myDFPlayer.readType(), myDFPlayer.read()); // Print the detail message from DFPlayer to handle different errors and states.
  }
  vTaskDelay(100);
}
