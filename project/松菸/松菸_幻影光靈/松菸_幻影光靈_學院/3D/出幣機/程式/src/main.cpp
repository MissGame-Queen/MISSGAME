#include <Arduino.h>
#include <Servo.h>
#include <SPI.h>

Servo myServo; //   Create Servo object to control the servo
void setup()
{
  myServo.attach(12); //   Servo is connected to digital pin 9
  pinMode(13, OUTPUT);
  pinMode(2, INPUT_PULLUP);
}
void loop()
{
  while (!digitalRead(2))
  {
    int16_t d = 7;
    myServo.write(d); //   Rotate servo  clockwise
    Serial.println(d);
    delay(600);
    d = d + 96;
    digitalWrite(13, 1);
    myServo.write(d); //   Rotate servo  clockwise
    Serial.println(d);
    delay(600);
  }
}