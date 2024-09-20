#include <MissGame.h>
#include "task.h"
#define _TYPE 1
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
#if _TYPE == 1
  xTaskCreatePinnedToCore(task,
                          "task",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#elif _TYPE == 2
  xTaskCreatePinnedToCore(task2,
                          "task2",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#elif _TYPE == 22
  xTaskCreatePinnedToCore(task2_2,
                          "task2_2",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#endif
  taskPCM5102((void *)&testBypass);
}
void loop()
{
}
