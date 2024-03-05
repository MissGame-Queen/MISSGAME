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
  // WiFi.begin("C3122", "000C3122");
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
/*
WiFiMulti wifiMulti;
SocketIOclient socketIO;
void testsocketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case sIOtype_DISCONNECT:
    Serial.printf("[IOc] Disconnected!\n");
    break;
  case sIOtype_CONNECT:
    Serial.printf("[IOc] Connected to url: %s\n", payload);

    // join default namespace (no auto join in Socket.IO V3)
    socketIO.send(sIOtype_CONNECT, "/");
    break;
  case sIOtype_EVENT:
  {
    char *sptr = NULL;
    int id = strtol((char *)payload, &sptr, 10);
    Serial.printf("[IOc] get event: %s id: %d\n", payload, id);
    if (id)
    {
      payload = (uint8_t *)sptr;
    }
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    String eventName = doc[0];
    Serial.printf("[IOc] event name: %s\n", eventName.c_str());

    // Message Includes a ID for a ACK (callback)
    if (id)
    {
      // creat JSON message for Socket.IO (ack)
      DynamicJsonDocument docOut(1024);
      JsonArray array = docOut.to<JsonArray>();

      // add payload (parameters) for the ack (callback function)
      JsonObject param1 = array.createNestedObject();
      param1["now"] = millis();

      // JSON to String (serializion)
      String output;
      output += id;
      serializeJson(docOut, output);

      // Send event
      socketIO.send(sIOtype_ACK, output);
    }
  }
  break;
  case sIOtype_ACK:
    Serial.printf("[IOc] get ack: %u\n", length);
    break;
  case sIOtype_ERROR:
    Serial.printf("[IOc] get error: %u\n", length);
    break;
  case sIOtype_BINARY_EVENT:
    Serial.printf("[IOc] get binary: %u\n", length);
    break;
  case sIOtype_BINARY_ACK:
    Serial.printf("[IOc] get binary ack: %u\n", length);
    break;
  }
}

void test2()
{

  // Serial.begin(921600);
  Serial.begin(115200);

  // Serial.setDebugOutput(true);
  Serial.setDebugOutput(true);

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--)
  {
    Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  wifiMulti.addAP("MissGame_B2", "missgame");

  // WiFi.disconnect();
  while (wifiMulti.run() != WL_CONNECTED)
  {
    delay(100);
  }

  String ip = WiFi.localIP().toString();
  Serial.printf("[SETUP] WiFi Connected %s\n", ip.c_str());

  // server address, port and URL
  socketIO.begin("192.168.1.104", 3000, "/socket.io/?EIO=4");

  // event handler
  socketIO.onEvent(socketIOEvent);

  unsigned long messageTimestamp = 0;
  while (1)
  {
    socketIO.loop();

    uint64_t now = millis();

    if (now - messageTimestamp > 2000)
    {
      messageTimestamp = now;

      // creat JSON message for Socket.IO (event)
      DynamicJsonDocument doc(1024);
      JsonArray array = doc.to<JsonArray>();

      // add evnet name
      // Hint: socket.on('event_name', ....
      array.add("event_name");

      // add payload (parameters) for the event
      JsonObject param1 = array.createNestedObject();
      param1["now"] = (uint32_t)now;

      // JSON to String (serializion)
      String output;
      serializeJson(doc, output);

      // Send event
      socketIO.sendEVENT(output);

      // Print JSON for debugging
      Serial.println(output);
    }
  }
}

*/