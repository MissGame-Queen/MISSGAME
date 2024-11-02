void yield(void);
#include <W600WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
static const char* ssid     = "MissGAME_SOG";
static const char* password = "missgame";
static const char* host = "192.168.0.8:1833";

void taskMQTT(void *pvParam){
delay(3000);
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();

  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());



  WiFiClient WiFiClient;
  PubSubClient MQTTClient(WiFiClient);
  // 設定 MQTT 伺服器
  JsonObject *prtDoc = (JsonObject *)pvParam;
  String clientId = (*prtDoc)[_E2S(_MQTT_CLIENT_NAME)]["Value"].as<String>();
  String ip_port = (*prtDoc)[_E2S(MQTT_BROKER_URL)]["Value"].as<String>();
  int colonIndex = ip_port.lastIndexOf(':'); // 找到最後一個冒號的位置
  if (colonIndex == -1)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "MQTT網址錯誤!%s\n", ip_port.c_str());
    vTaskDelete(NULL);
  }
  // 變數不能放條件內 不知為何會連不到
  String ipStr = ip_port.substring(0, colonIndex);
  String portStr = ip_port.substring(colonIndex + 1); // 提取冒號後的子字符串
  uint16_t port = portStr.toInt();                    // 將字符串轉換為uint16_t類型
  MQTTClient.setServer(ipStr.c_str(), port);

  // 設定回呼函式
  MQTTClient.setCallback(MQTT_Callback);
  uint16_t dalayTime = (*prtDoc)[_E2S(_MQTT_DELAYTIME)]["Value"].as<uint16_t>();
  while (true)
  {
    // 保持連線
    // 重新連線到 MQTT 代理
    if (!MQTTClient.connected())
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "正在嘗試MQTT連線...{%s,%d}\n", ipStr.c_str(), port);
      if (clientId = "")
        clientId = "esp32-" + WiFi.macAddress();
      if (MQTTClient.connect(clientId.c_str()))
      {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "已連結到MQTT代理!");

        _T_E2JS(_STATUSLED)
        ["Color_R"] = 255;
        JsonObject obj = _T_E2JS(_STATUSLED);
        xQueueSend(queueStatusLED, &obj, portMAX_DELAY);

        MQTT_Subscribe(&MQTTClient);
        // MQTTClient.publish("test/topic", String("{\"data\":\"Hello!I am " + clientId + "\"}").c_str());
      }
      else
      {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "錯誤代碼:%d，5秒後重連....\n", MQTTClient.state());
        _T_E2JS(_STATUSLED)
        ["Color_R"] = 0;
        JsonObject obj = _T_E2JS(_STATUSLED);
        xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
        _DELAY_MS(5000);
      }
    }
    MQTTClient.loop();
    _DELAY_MS(dalayTime);
  }

}

void setup() {
  
}


void loop() {
  delay(10000);
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = "/arduino/w60x.php";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available() || client.connected()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
    Serial.print("\r");
  }

  Serial.println();
  Serial.println("closing connection");
  client.stop();
}

