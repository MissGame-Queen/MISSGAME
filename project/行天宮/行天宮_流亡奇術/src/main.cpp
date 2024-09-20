#include "main.h"
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  uint8_t testBypass = 0;

  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          2048,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
  xTaskCreatePinnedToCore(task,
                          "task",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);

  taskPCM5102((void *)&testBypass);
}
void loop()
{
}
