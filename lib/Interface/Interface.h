#ifndef INTERFACE_H
#define INTERFACE_H
#include "Template.h"
#include <Wire.h>
class Interface : public Stream
{
private:
    JsonDocument _csDocConfig;
    JsonArray _csArrayData;
    TwoWire *_csI2C;
    HardwareSerial *_csSerial;
    const char *strI2C = "I2C";
    const char *strSerial = "Serial";

public:
    int8_t csType = -1;
    Interface(/* args */)
    {
    }
    //~Interface();
    int8_t Begin(HardwareSerial *serial, JsonObject *config);
    int8_t Begin(TwoWire *i2c, JsonObject *config);
    // void Begin(SPIClass *spi, JsonObject *config);
    // void Begin(Stream *stream, JsonObject *config);

    int available();
    int read();
    int peek();
    size_t write(uint8_t data);
};

#endif