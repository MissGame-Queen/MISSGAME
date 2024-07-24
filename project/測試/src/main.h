#ifndef MAIN_H
#define MAIN_H
#include <Template.h>
#include "task.h"
#define _E2JS(x) (*Template_JsonPTC->getJsonObject())[#x]["Value"]
// 描述{預設值,禁止的權限[R=讀取,W=寫入,S=寫入+存檔],HTML顯示的類型[默認text]}
enum StrRegister
{
    AP_SSID,          // 連結到設備用的WIFI名稱{}
    AP_PASSWORD,      // 連結到設備用的WIFI密碼{00000000,R,password}
    WIFI_HOSTNAME,    // 設備名稱{}
    WIFI_MDNSNAME,    // 網域的主機名稱(後面加上.local即可透過瀏覽器瀏覽){}
    FIRMWAREURL,      // 自動更新韌體的目標網址(空=不自動更新)例:FirmwareUpdata.local{}
    MQTT_BROKER_URL,  // MQTT服務器網址(localhost.local){192.168.1.104}
    SOCKETIO_URL,     // SocketIO伺服器網址(localhost.local){192.168.1.104}
    MQTT_CLIENT_NAME, // MQTT客戶端名稱{}
};
enum ModbusRegister
{
    _INFORMATION,        // 版本資料{24001,W}
    _RESET,              // 回原廠設定{,S}
    _SAVE,               // 儲存設定{,S}
    _MODBUS_EN,          // Modbus啟用{1}
    _MODBUS_ID,          // MODBUS ID{1}
    _MODBUS_BAUD,        // MODBUS鮑率{1152}
    _MODBUS_PROTOCOL,    // MODBUS通訊協議編號{0}
    _WIFI_TYPE,          // 連線模式(0:OFF,1:STA,2:AP,3:APSTA){3}
    _WIFI_CONNECT_TIME,  // 嘗試連線次數{0}
    _MQTT_BROKER_PORT,   // MQTT端口{1833}
    _MQTT_DELAYTIME,     // MQTT的loop時間(ms){100}
    _SOCKETIO_PORT,      // SocketIO端口{3000}
    _SOCKETIO_DELAYTIME, // SocketIO端口的loop時間(ms){100}

    _MODULE_TYPE,     // 模組類型{1}
    _MODULE_ID,       // 模組在環境中唯一ID{99}
    _TESTMODE,        // 開機是否進入測試模式
    _SEVER_DEG_START, // 開始角度
    _SEVER_DEG_END,   // 結束角度
    _LIGHT_0,         // 自定義亮度0
    _LIGHT_1,         // 自定義亮度1
    _LIGHT_2,         // 自定義亮度2
    _LIGHT_3,         // 自定義亮度3
    _BATTERY_VAL,     // 電池電壓
    _DATANUM
};

#endif