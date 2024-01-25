#include "FM_505.h"
void FM505::Begin(HardwareSerial *serial)
{
    csSerial = serial;
    csSerial->begin(38400);
}
void FM505::Encoder(JsonObject obj)
{
    if (obj.containsKey("Command"))
    {
        String strWrite = "";
        strWrite += '\n';
        strWrite += obj["Command"].as<String>();
        if (obj.containsKey("Data"))
        {
            uint8_t numParameter = 0;
            for (JsonVariant item : obj["Data"].as<JsonArray>())
            {

                if (numParameter == 0)
                    ;
                else if (numParameter == 2 && obj["Command"].as<String>() == "W")
                    ;
                else
                {
                    strWrite += ",";
                }
                if (numParameter == 2 && obj["Command"].as<String>() == "W")
                    ;
                else if (numParameter == 3 && obj["Command"].as<String>() == "W")
                {

                    String strMsg = item.as<String>();
                    uint8_t strLength = strMsg.length() / 2 + ((strMsg.length() % 2) > 0 ? 1 : 0);
                    char buffer[3]; // 2 個字符 + 結尾的 null 字符
                    sprintf(buffer, "%02X", strLength);
                    strWrite += buffer;
                    strWrite += ",";
                    for (uint8_t i = 0; i < strLength * 2; i++)
                    {
                        byte charAscii[2];
                        HEX2Ascii(i < strMsg.length() ? strMsg[i] : 0x20, charAscii);
                        strWrite += char(charAscii[0]);
                        strWrite += char(charAscii[1]);
                    }
                    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, item.as<String>());
                }
                else
                    strWrite += item.as<String>();
                numParameter++;
            }
        }
        strWrite += '\r';
        csSerial->print(strWrite);
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, strWrite);
        csstrDecoder = "";
        getStringSerial();
        //_CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, getStringDecoder());
    }
}
void FM505::Decoder(JsonDocument *doc)
{
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, csstrDecoder);
    if (csstrDecoder[0] == '\n' &&
        csstrDecoder[csstrDecoder.length() - 2] == '\r' &&
        csstrDecoder[csstrDecoder.length() - 1] == '\n')
    {

        (*doc)["Command"].set(String(csstrDecoder[1]));

        String strData = "";
        if (csstrDecoder.length() > 7)
        {
            strData += char(csstrDecoder[2]);
            strData += char(csstrDecoder[3]);
            strData += char(csstrDecoder[4]);
            strData += char(csstrDecoder[5]);
        }

        if ((*doc)["Command"].as<String>() == "Z")
        {
            strData = "";
            for (uint8_t i = 0; i < csstrDecoder.length() - 4; i++)
                strData += char(csstrDecoder[2 + i]);
            _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, strData);
        }

        else if (strData != "<OK>")
        {
            strData = "";
            for (uint8_t i = 0; i < csstrDecoder.length() - 4; i += 2)
            {
                strData += char(Ascii2HEX(csstrDecoder[2 + i], csstrDecoder[2 + i + 1]));
            }
        }
        (*doc)["Data"].set(strData);

    }
    else
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "格式錯誤!\n");

        for (uint8_t i = 0; i < csstrDecoder.length(); i++)
        {
            Serial.printf("%02x,", csstrDecoder[i]);
        }
        Serial.println();

        (*doc)["Command"].set("E");
        (*doc)["Data"].set("格式錯誤!");
    }
}
void FM505::getStringSerial()
{
    uint16_t i = 0;
    while (i < 100 && csSerial->available() < 1)
    {
        i++;
        _DELAY_MS(1);
    }
    if (csSerial->available())
    {
        csstrDecoder += csSerial->readString();
        getStringSerial();
    }
}

void FM505::CMD(CMD_List list, JsonDocument *indoc)
{
    JsonDocument  doc;
    
    doc["Command"].to<JsonObject>();
    doc["Data"].to<JsonArray>();
    /*
    doc.createNestedObject("Command");
    doc.createNestedArray("Data");
    */
    switch (list)
    {
    case FM505_FirmwareVersion:
        doc["Command"] = "V";
        break;
    case FM505_Reader_ID:
        doc["Command"] = "S";
        break;
    case FM505_Tag_EPC_ID:
        doc["Command"] = "Q";
        break;
    case FM505_Tag_EPC_ID_Multi:
        doc["Command"] = "U";
        break;
    default:
        break;
    }
    Encoder(doc.as<JsonObject>());
    getStringSerial();

}
String FM505::getStringDecoder()
{
    return csstrDecoder;
}