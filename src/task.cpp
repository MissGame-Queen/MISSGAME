#include "task.h"
void task_Audio(void *pvParam)
{
    waitConnect();
    audio.setPinout(pinAudio_BCK, pinAudio_WS, pinAudio_DO);
    audio.setVolume(21); // 0...21
    // audio.connecttospeech("開機完成", "zh");
    while (true)
    {
        audio.loop();
        _DELAY_MS(1);
    }
}
void task_LED(void *pvParam)
{

    strip.begin();
    for (uint8_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, 0);
    }
    strip.show();

    while (true)
    {
        switch (_E2JS(_LED_TYPE).as<uint16_t>())
        {
        default:
            for (uint8_t i = 0; i < strip.numPixels(); i++)
            {
                uint32_t cycle;
                switch (ledType[i])
                {
                case 0:
                    //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "I:%d,R:%d,G:%d,B:%d\n", i, ledR[i], ledG[i], ledB[i]);

                        strip.setPixelColor(i, strip.Color(
                                                   ledR[i],
                                                   ledG[i],
                                                   ledB[i]));
                    break;
                case 1:
                    cycle = ((millis() - ledTime[i]) % (ledON[i] + ledOFF[i]));
                    if (cycle > ledON[i])
                        strip.setPixelColor(i, 0);
                    else
                        strip.setPixelColor(i, strip.Color(
                                                   ledR[i],
                                                   ledG[i],
                                                   ledB[i]));
                    break;
                case 2:
                    float brightness = 0.0;
                    cycle = ((millis() - ledTime[i]) % (ledON[i] + ledOFF[i]));
                    if (cycle > ledON[i])
                        brightness = map(cycle,
                                         ledON[i], (ledON[i] + ledOFF[i]),
                                         255, 0);

                    else
                        brightness = map(cycle,
                                         0, (ledON[i]),
                                         0, 255);
                    brightness /= 255.0;
                    strip.setPixelColor(i, strip.Color(
                                               ledR[i] * brightness,
                                               ledG[i] * brightness,
                                               ledB[i] * brightness));
                    break;
                }
            }

            strip.show();
        }

        _DELAY_MS(10);
    }
}
void task_MQTT(void *pvParam)
{
  waitConnect(0);
  // 設定 MQTT 伺服器
  clientMQTT.setServer(_E2JS(MQTT_SERVER).as<const char *>(), _E2JS(MQTT_PORT).as<uint16_t>());
  // 設定回呼函式
  clientMQTT.setCallback(callback_MQTT);
  while (true)
  {
    // 保持連線
    reconnect_MQTT();

    // 監聽 MQTT 訊息
    clientMQTT.loop();
    _DELAY_MS(1);
    // 做其他的事情...
  }
}