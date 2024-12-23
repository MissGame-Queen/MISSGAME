#ifndef MYFUNCTION_H
#define MYFUNCTION_H
//#include <Template.h>
#include "Init.h"
//?========================列舉========================
enum eRGB_Type
{
    _Monochrome = 0, // 單色
    _Colorful = 1,   // 七彩
    _Brightness = 0, // 固定亮度
    _Breathe = 2,    // 呼吸
    _Flashing = 4,   // 閃爍
};
//?========================結構========================
// OLED的QRcode顯示參數結構
/*
typedef struct OLED_QRcode_t
{
    QRCode Qrcode;
    U8G2 *display;
    int16_t x = 0, y = 0;
    String Text;
} OLED_QRcode_t;
*/
//?========================函數========================
//void writeQRCODE(OLED_QRcode_t *Qrcode, uint16_t maxsize = 64);
uint32_t setRainbowRGB(uint16_t value);
uint32_t setBrightnessRGB(uint32_t value, uint8_t brightness=255);
void doLightRGB(Adafruit_NeoPixel* strip, JsonObject config);
void I2CScanner();
void ESP32_Info();
#endif
