#include <Init.h>
SemaphoreHandle_t rmtMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t SPIMutex = xSemaphoreCreateMutex();
SemaphoreHandle_t I2CMutex = xSemaphoreCreateMutex();
const char *Template_HTTP_ContentType_Text = "text/html; charset=utf-8";
const char *Template_HTTP_ContentType_Json = "application/json; charset=utf-8";
struct tm _T_localtm = {
    2024 - 1900, // tm_year: 設定為 2024 - 1900
    10 - 1,      // tm_mon: 設定為 9 (10 月)
    12,          // tm_mday: 設定為 12 (日)
    0,           // tm_hour: 設定為 0 (時)
    0,           // tm_min: 設定為 0 (分)
    0,           // tm_sec: 設定為 0 (秒)
    0,           // tm_wday: 不重要的值 (通常可由系統計算)
    0,           // tm_yday: 不重要的值 (通常可由系統計算)
    -1           // tm_isdst: 設定為 -1 表示自動計算日光節約時間
};
AsyncWebServer Template_server(80);
JsonObject Template_System_Obj;
