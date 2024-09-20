#include "main.h"
// optional

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  uint8_t testBypass = 0;

  xTaskCreatePinnedToCore(task,
                          "task",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);                       
  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);

  Serial.print("剩餘內存: ");
  Serial.println(ESP.getFreeHeap());

  taskPCM5102((void *)&testBypass);
}
void loop()
{
}
