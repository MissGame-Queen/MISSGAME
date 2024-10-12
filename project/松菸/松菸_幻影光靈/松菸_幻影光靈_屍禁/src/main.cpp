#include "main.h"
JsonDocument doc;
#define _TYPE 1 // 1:通靈板程式,2:4按鈕程式
void setup()
{
  Serial.begin(115200);
  Wire.begin();
  uint8_t testBypass = 0;

#if _TYPE == 1
  doc["MCP23017"]["Address"].to<JsonArray>();
  doc["MCP23017"]["Address"].add(0x20);
  
  xTaskCreatePinnedToCore(taskOuijaBoard,
                          "taskOuijaBoard",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#elif _TYPE == 2
  xTaskCreatePinnedToCore(task4SequenceButtons,
                          "task4SequenceButtons",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#endif
  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          4096,
                          (void *)&doc,
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
