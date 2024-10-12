#include "main.h"
JsonDocument doc;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  uint8_t testBypass = 0;
  doc["MCP23017"]["Address"].to<JsonArray>();
  doc["MCP23017"]["Address"].add(0x27);
  doc["MCP23017"]["Address"].add(0x26);
  doc["MCP23017"]["Address"].add(0x25);
  doc["MCP23017"]["Address"].add(0x24);
  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          2048,
                          (void *)&doc,
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
