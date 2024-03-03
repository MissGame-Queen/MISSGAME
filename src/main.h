#ifndef MAIN_H
#define MAIN_H
#include <Template.h>
#define _E2JS(x) (*Template_JsonPTC->getJsonObject())[#x]["Value"]


enum StrRegister
{
    STA_SSID,      // 想連接的WIFI名稱{TTPunch}
    STA_PASSWORD,  // 想連接的WIFI密碼{033865819}
    AP_SSID,       // 連結到設備用的WIFI名稱{Ragic_QRCODE}
    AP_PASSWORD,   // 連結到設備用的WIFI密碼{00000000}
    WIFI_HOSTNAME, // 設備名稱{Ragic_QRCODE}
    WIFI_MDNSNAME, // 網域的主機名稱(後面加上.local即可透過瀏覽器瀏覽){Ragic_QRCODE}

};
enum ModbusRegister
{
    _INFORMATION,     // 版本資料{21001}
    _RESET,           // 回原廠設定
    _SAVE,            // 儲存設定
    _MODBUS_EN,       // Modbus啟用{1}
    _MODBUS_ID,       // 設備ID{1}
    _MODBUS_BAUD,     // 鮑率{1152}
    _MODBUS_PROTOCOL, // 通訊協議編號{0}
    _MODULE_TYPE,     // 模組類型{1}
    _MQTT_EN,
    _MQTT_CLIENT_ID,
    _MQTT_BROKER_PORT,
    _SOCKETIO_EN,
    _SOCKETIO_PORT,
    _OLED_EN,           // OLED啟用{1}
    _OLED_TYPE,         // OLED型號設定 [方向3][型號3]… [方向0][型號0]{43520}
    _OLED_REFRESH,      // OLED刷新
    _OLED_PAGE,         // OLED顯示畫面{0}
    _WIFI_CONNECT_TIME, // 嘗試連線次數{20}
    _WIFI_TYPE,         // 連線模式(0:OFF,1:STA,2:AP,3:APSTA){3}
    _RFID_EN,           // 是否啟用RFID{0}
    _TH_TEMPERATURE,    // 溫度
    _TH_HUMIDITY,       // 濕度
    _DATANUM
};

#endif