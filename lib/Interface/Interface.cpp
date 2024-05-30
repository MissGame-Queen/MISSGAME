#include "Interface.h"
#define _USE_JSONARRAY false
int8_t Interface::Begin(HardwareSerial *serial, JsonObject *config)
{
    int8_t rtVal = 1;
    _csDocConfig.set(*config);
    _csDocConfig["Type"] = strSerial;
    _csSerial = serial;
    if (!_csDocConfig.containsKey("baud"))
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "未填寫baud!");
        rtVal = -1;
    }
    else
        _csSerial->begin(_csDocConfig["baud"].as<uint32_t>());

    _csArrayData = _csDocConfig["Data"].to<JsonArray>();
    return rtVal;
}
int8_t Interface::Begin(TwoWire *i2c, JsonObject *config)
{
    int8_t rtVal = 1;
    _csDocConfig.set(*config);
    _csDocConfig["Type"] = strI2C;
    _csI2C = i2c;
    _csI2C->begin();
    if (!_csDocConfig.containsKey("Address"))
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "未填寫Address!");
        rtVal = -1;
    }
    if (_csDocConfig.containsKey("setClock"))
        _csI2C->setClock(_csDocConfig["setClock"].as<uint32_t>());
    _csArrayData = _csDocConfig["Data"].to<JsonArray>();
    return rtVal;
}

int Interface::available()
{
    int rtData = 0;
#if _USE_JSONARRAY
    return _csArrayData.size();
#else
    if (_csDocConfig["Type"].as<String>() == strSerial)
        rtData = _csSerial->available();
    else if (_csDocConfig["Type"].as<String>() == strI2C)
        rtData = _csI2C->available();
#endif
    return rtData;
}
int Interface::read()
{
    int rtData = 0;
#if _USE_JSONARRAY
    rtData = _csArrayData[0];
    _csArrayData.remove(0);
#else
    if (_csDocConfig["Type"].as<String>() == strSerial)
    {
        rtData = _csSerial->read();
    }
    else if (_csDocConfig["Type"].as<String>() == strI2C)
    {
        
        _csI2C->requestFrom(_csDocConfig["Address"].as<uint8_t>(), 1);
        rtData = _csI2C->read();
    }
#endif

    return rtData;
}
int Interface::peek()
{

    int rtData = 0;
#if _USE_JSONARRAY
    rtData = _csArrayData[0];
#else
    if (_csDocConfig["Type"].as<String>() == strSerial)
    {
        rtData = _csSerial->peek();
    }
    else if (_csDocConfig["Type"].as<String>() == strI2C)
    {
        _csI2C->requestFrom(_csDocConfig["Address"].as<uint8_t>(), 1);
        _csI2C->peek();
    }
#endif
    return rtData;
}

size_t Interface::write(uint8_t data)
{
    if (_csDocConfig["Type"].as<String>() == strSerial)
    {
        _csSerial->write(data);
    }
    else if (_csDocConfig["Type"].as<String>() == strI2C)
    {
        _csI2C->beginTransmission(_csDocConfig["Address"].as<uint8_t>());
        _csI2C->write(data);
        _csI2C->endTransmission();
    }
    return 1;
}