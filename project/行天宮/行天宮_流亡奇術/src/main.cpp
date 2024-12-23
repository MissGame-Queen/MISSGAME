#include "main.h"
#define _TYPE 2
JsonDocument doc;

void setup()
{
  Serial.begin(115200);
  Wire.begin();
  uint8_t testBypass = 0;
#if _TYPE == 0
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
  xTaskCreatePinnedToCore(FloorMechanism,
                          "FloorMechanism",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#elif _TYPE == 1
  doc["MCP23017"]["Address"].to<JsonArray>();
  doc["MCP23017"]["Address"].add(0x20);
  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          2048,
                          (void *)&doc,
                          1,
                          NULL,
                          0);
  xTaskCreatePinnedToCore(Dialla,
                          "Dialla",
                          4096,
                          (void *)&testBypass,
                          1,
                          NULL,
                          0);
#elif _TYPE == 2
  doc["MCP23017"]["Address"].to<JsonArray>();
  doc["MCP23017"]["Address"].add(0x20);
  xTaskCreatePinnedToCore(taskMCP230x7,
                          "taskMCP230x7",
                          2048,
                          (void *)&doc,
                          1,
                          NULL,
                          0);
  xTaskCreatePinnedToCore(FalseSun,
                          "FalseSun",
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
