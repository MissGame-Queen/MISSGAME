#ifndef FM_505_h
#define FM_505_h
#include <Template.h>
class FM505
{
public:
    enum CMD_List
    {
        FM505_FirmwareVersion,  // 硬體版本
        FM505_Reader_ID,        // 天線ID
        FM505_Tag_EPC_ID,       // 讀取TAG的EPC
        FM505_Tag_EPC_ID_Multi, // 大量讀取TAG的EPC
    };
    FM505()
    {
    }
    void Begin(HardwareSerial *serial);
    void Encoder(JsonObject obj);
    void Decoder(JsonDocument *doc);
    void CMD(CMD_List list, JsonDocument *indoc = nullptr);
    String getStringDecoder();
    void getStringSerial();

    /**
     * @brief 字元轉BYTE
     *
     * @param h 高位字元
     * @param l 低位字元
     * @return uint8_t
     */
    uint8_t Ascii2HEX(char h, char l)
    {
        byte data = 0;
        if (h >= '0' && h <= '9')
            data += h - '0';
        else if (h >= 'A' && h <= 'F')
            data += h - 'A' + 10;
        else if (h >= 'a' && h <= 'f')
            data += h - 'a' + 10;
        data = data << 4;
        if (l >= '0' && l <= '9')
            data += l - '0';
        else if (l >= 'A' && l <= 'F')
            data += l - 'A' + 10;
        else if (l >= 'a' && l <= 'f')
            data += l - 'a' + 10;
        return data;
    }
    /**
     * @brief BYTE轉字元
     *
     * @param hexdata 8位元資料
     * @param asciidata 回傳的字元陣列指標
     */
    void HEX2Ascii(byte hexdata, byte *asciidata)
    {
        for (byte i = 0; i < 2; i++)
        {
            if (i)
                *(asciidata + i) = hexdata & 0x0F;
            else
                *(asciidata + i) = hexdata >> 4;
            if (*(asciidata + i) < 10)
                *(asciidata + i) += '0';
            else
                *(asciidata + i) += 'A' - 10;
        }
        // Serial.printf("%02X,",hexdata);
    }

private:
    HardwareSerial *csSerial;
    String csstrDecoder = "";
};
#endif