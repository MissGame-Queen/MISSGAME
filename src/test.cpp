#include "test.h"
void test()
{
    SPIFFS.begin();
    listFile(configType_e(0));
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START);
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
    WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1),    // IP位址
                      IPAddress(192, 168, 1, 1),    // 閘道位址
                      IPAddress(255, 255, 255, 0)); // 網路遮罩
    int8_t status = 0;
    status = WiFi.softAP("[" + WiFi.macAddress() + "]", "00000000");
    //WiFi.begin("C3122", "000C3122");
    WiFi.mode(wifi_mode_t(2));

    AsyncWebServer server(80);
    server.addHandler(new SPIFFSEditor(SPIFFS, "admin", "admin"));
    server.on(
        "/", HTTP_GET, [](AsyncWebServerRequest *request)
        {            
          FS *prtFS = 0;
 prtFS = &SPIFFS;
          File file = prtFS->open("/www/index.html", "r");
          String strSend = file.readString(); 
           int mode = WiFi.getMode();
  String modeText;
  switch (mode) {
    case WIFI_OFF:
      modeText = "WiFi Off";
      break;
    case WIFI_STA:
      modeText = "STA (Station) Mode";
      break;
    case WIFI_AP:
      modeText = "AP (Access Point) Mode";
      break;
    case WIFI_AP_STA:
      modeText = "AP and STA Mode";
      break;
    default:
      modeText = "Unknown Mode";
  }  
        strSend.replace("{WIFI_MODE}", modeText);          
        strSend.replace("{SSID}", WiFi.SSID().c_str());
        strSend.replace("{LOCALIP}",WiFi.localIP().toString().c_str());
        strSend.replace("{RSSI}",String(WiFi.RSSI()).c_str());          
          file.close();          
          request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, strSend); });
    server.begin();
    while (1)
        ;
}