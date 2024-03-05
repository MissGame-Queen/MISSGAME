#include "task.h"
Servo myServo; //   Create Servo object to control the servo
/**
 * @brief 出幣機程式
 *
 */
void CoinDispenser(uint16_t time)
{
    const uint8_t pinServo = 13, pinLED = 12, pinTrigger = 15;
    if (time > 0)
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被觸發了~");
        for (uint16_t i = 0; i < time; i++)
        {
            int16_t d = 103;
            digitalWrite(pinLED, 0);
            myServo.write(d); //   Rotate servo  clockwise
            // Serial.println(d);
            _DELAY_MS(600);
            d -= 96;
            digitalWrite(pinLED, 1);
            myServo.write(d); //   Rotate servo  clockwise
            // Serial.println(d);
            _DELAY_MS(600);
        }
        return;
    }
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機模式~");
    myServo.attach(pinServo); //   Servo is connected to digital pin 9
    pinMode(pinLED, OUTPUT);
    pinMode(pinTrigger, INPUT_PULLUP);
    while (1)
    {
        while (!digitalRead(pinTrigger))
        {
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "出幣機被觸發了~");
            int16_t d = 103;
            digitalWrite(pinLED, 0);
            myServo.write(d); //   Rotate servo  clockwise
            // Serial.println(d);
            _DELAY_MS(600);
            d -= 96;
            digitalWrite(pinLED, 1);
            myServo.write(d); //   Rotate servo  clockwise
            // Serial.println(d);
            _DELAY_MS(600);
        }
        _DELAY_MS(200);
    }
}