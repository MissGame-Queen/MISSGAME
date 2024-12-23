#include "taskTemplate.h"
QueueHandle_t queueStatusLED = xQueueCreate(1, sizeof(JsonObject));
// BUG 不知為何若WiFi處在STA+AP模式下無法連線
/**
 * @brief MQTT運行任務
 *
 * @param pvParam
 */
void taskMQTT(void *pvParam)
{
  waitConnect(0, 1);
  WiFiClient WiFiClient;
  PubSubClient MQTTClient(WiFiClient);

  // 設定 MQTT 伺服器
  JsonObject *prtDoc = (JsonObject *)pvParam;
  String clientId = (*prtDoc)[_E2S(MQTT_CLIENT_NAME)]["Value"].as<String>();
  String ip_port = (*prtDoc)[_E2S(MQTT_BROKER_URL)]["Value"].as<String>();
  int colonIndex = ip_port.lastIndexOf(':'); // 找到最後一個冒號的位置
  if (colonIndex == -1)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "MQTT網址錯誤!%s\n", ip_port.c_str());
    vTaskDelete(NULL);
  }
  // 變數不能放條件內 不知為何會連不到
  String ipStr = ip_port.substring(0, colonIndex);
  uint16_t port = String(ip_port.substring(colonIndex + 1)).toInt(); // 將字符串轉換為uint16_t類型
  MQTTClient.setServer(ipStr.c_str(), port);

  // 設定回呼函式
  MQTTClient.setCallback(MQTT_Callback);
  uint16_t dalayTime = (*prtDoc)[_E2S(_MQTT_DELAYTIME)]["Value"].as<uint16_t>();
  while (true)
  {
    // 保持連線
    if (isConnect(FUNCTION_CODE_HAVE_IP))
    {
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
          //MQTTClient.publish("test/topic", String("{\"data\":\"Hello!I am " + clientId + "\"}").c_str());
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
    }
    _DELAY_MS(dalayTime);
  }
}
void taskSocketIO(void *pvParam)
{
  waitConnect(0, 1);
  JsonObject *prtDoc = (JsonObject *)pvParam;
  String ip_port = (*prtDoc)[_E2S(SOCKETIO_URL)]["Value"].as<String>();
  int colonIndex = ip_port.lastIndexOf(':'); // 找到最後一個冒號的位置
  // 變數不能放條件內 不知為何會連不到
  if (colonIndex == -1)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "SocketIO網址錯誤!%s\n", ip_port.c_str());
    vTaskDelete(NULL);
  }
  String ipStr = ip_port.substring(0, colonIndex);
  uint16_t port = String(ip_port.substring(colonIndex + 1)).toInt(); // 將字符串轉換為uint16_t類型
  const char *url = "/socket.io/?EIO=4";                             // DEFAULT_URL;
  String sslFingerprint = "";                                        // SSL Certificate Fingerprint
  // FIXME如果後續有需要SSL再編寫
  sslFingerprint != "" ? socketIO_Client.beginSSL(ipStr, port, url, sslFingerprint.c_str()) : socketIO_Client.begin(ipStr, port, url);

  // Handle Authentication
  /*FIXME如果後續有需要身分驗證再編寫
  if (useAuth)
  {
    socketIO_Client.setAuthorization(serverUsername, serverPassword);
  }
  */
  // event handler

  uint16_t dalayTime = (*prtDoc)[_E2S(_SOCKETIO_DELAYTIME)]["Value"].as<uint16_t>();
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "正在嘗試SocketIO連線...{%s,%d}\n", ipStr.c_str(), port);
  socketIO_Client.onEvent(socketIOEvent);
  while (1)
  {
    socketIO_Client.loop();
    _DELAY_MS(dalayTime);
  }
}
/**
 * @brief 時間任務
 * 
 * 
 * @param pvParam 
 */
void taskReadDate(void *pvParam)
{
  while (true)
  {
    WiFiReconnect();
    readDate();
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}
/**
 * @brief 自動訪問網址獲取最新韌體更新自身
 *
 * @param pvParam 輸入伺服器端專案資料夾絕對位址
 * {
 * pvParam["Path"].set("D:/QR_COED");
 * }
 */
void taskUpdateFirmware(void *pvParam)
{
  waitConnect(0);
  HTTPClient http;
  JsonDocument *prtDoc = (JsonDocument *)pvParam; // 取得韌體所在目錄{"Path":"D:/QR_COED"}
  String fullUrl = _T_E2JS(_FIRMWAREURL).as<String>() + "/getNewFirmwareVer";

  // String fullUrl = "http://192.168.15.128/getNewFirmwareVer";
  //_CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, fullUrl);
  while (true)
  {
    http.begin(fullUrl); // 設定網址
    // http.begin("ttpunch.asuscomm.com", 800, "/getNewFirmwareVer");
    http.addHeader("Content-Type", Template_HTTP_ContentType_Json);
    int httpCode = http.POST(prtDoc->as<String>());
    if (httpCode == HTTP_CODE_OK)
    {
      JsonDocument doc;
      DeserializationError error = deserializeJson(doc, http.getStream());
      if (error)
        _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化錯誤: %s\n", error.f_str());
      else
      {
        doc["status"] = 0;
        for (JsonVariant item : doc["File"].as<JsonArray>())
        {
          //_CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, item.as<String>());
          File file = getFS()->open(item["Name"].as<const char *>(), FILE_READ);
          //_CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, file.size());

          if (!file || file.size() != item["Size"].as<size_t>())
          {
            doc["FileArray"].add(item["Name"].as<String>());
            doc["status"] = doc["status"].as<uint16_t>() + 1;
          }
          file.close();
        }
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "讀取更新資訊:");
        serializeJsonPretty(doc, Serial);
        doc.remove("File");
        doc["Path"] = (*prtDoc)["Path"];
        if (doc["Ver"].as<uint16_t>() > _T_E2JS(_VER).as<uint16_t>())
        {
          doc["status"] = doc["status"].as<uint16_t>() + 1;
          doc["Firmware"] = true;
        }
      }
      http.end();

      if (doc["status"].as<uint16_t>() > 0)
      {
        fullUrl = _T_E2JS(_FIRMWAREURL).as<String>() + "/updata";
        http.begin(fullUrl);
        http.addHeader("Content-Type", Template_HTTP_ContentType_Json);
        JsonDocument docPost;
        // 覆寫data檔案
        for (JsonVariant item : doc["FileArray"].as<JsonArray>())
        {
          if (item.as<String>() == "/config.json")
            continue;
          _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, item.as<String>());
          docPost["Path"] = doc["Path"];
          docPost["File"] = item.as<String>();
          httpCode = http.POST(docPost.as<String>()); // POST設備類型並索取該類型固件

          if (httpCode == HTTP_CODE_OK)
          {
            // String content = http.getString();
            File file = getFS()->open(docPost["File"].as<const char *>(), FILE_WRITE);
            if (!file)
            {
              _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "檔案開啟錯誤:%d\n", docPost["File"].as<const char *>());
              continue;
            }
            Stream *ptrStream = http.getStreamPtr();

            while (ptrStream->available())
            {
              char c = ptrStream->read();
              file.write(c);
            }
            file.close();
          }
          else if (httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR)
          {
            _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "伺服器回應錯誤!");

            DeserializationError error = deserializeJson(doc, http.getStream());
            if (error)
              _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化錯誤: %s\n", error.f_str());
            else
            {
              serializeJsonPretty(doc, _CONSOLE_PORT);
            }
          }
          else
            _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "[%d]HTTP ERROR:%s\n", httpCode, HTTPClient::errorToString(httpCode).c_str());
        }
        // 覆寫韌體
        if (doc.containsKey("Firmware"))
        {
          docPost["Path"] = doc["Path"];
          docPost["Firmware"] = doc["Firmware"];
          byte bufData[1024];
          httpCode = http.POST(docPost.as<String>()); // POST設備類型並索取該類型固件

          if (httpCode == HTTP_CODE_OK)
          {
            Stream *ptrStream = http.getStreamPtr();
            const int totalSize = http.getSize();
            int bytesRead = 0;
            const int chunkSize = sizeof(bufData);

            if (totalSize <= 0)
            {
              _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "韌體大小錯誤");
              http.end();
              return;
            }

            // 開始更新
            if (Update.begin())
            {
              _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "韌體大小%d，開始更新韌體...\n", totalSize);
              // 從伺服器逐步下載固件數據
              while (bytesRead < totalSize)
              {
                int chunkSizeRead = ptrStream->readBytes(bufData, std::min(chunkSize, totalSize - bytesRead));
                if (chunkSizeRead > 0)
                {
                  Update.write(bufData, chunkSizeRead);
                  bytesRead += chunkSizeRead;
                }
                else
                {
                  _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "讀取韌體資料失敗");
                  break;
                }
                _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "韌體更新進度: %.2f%%\r", (float)bytesRead / totalSize * 100);
              }

              // 結束更新
              if (Update.end(true))
              {
                _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "\n韌體更新成功!3秒後重新啟動...");
                _DELAY_MS(3000);
                ESP.restart();
              }
              else
              {
                _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "\n韌體更新失敗:");
                Update.printError(Serial);
              }
            }
            else
            {
              _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "無法開始韌體更新:");
              Update.printError(_CONSOLE_PORT);
            }
            http.end(); // 結束HTTP連線
          }
          else if (httpCode == HTTP_CODE_INTERNAL_SERVER_ERROR)
          {
            DeserializationError error = deserializeJson(doc, http.getStream());
            if (error)
              _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化錯誤: %s\n", error.f_str());
            else
            {
              serializeJsonPretty(doc, _CONSOLE_PORT);
            }
          }
          else
            _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "[%d]HTTP ERROR:%s\n", httpCode, HTTPClient::errorToString(httpCode).c_str());
        }

        //_CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, fullUrl);

        http.end();
      }
    }
    else
      _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "%s\n[%d]HTTP ERROR:%s\n", fullUrl.c_str(), httpCode, HTTPClient::errorToString(httpCode).c_str());

    _DELAY_MS(86400000);
  }
}
void taskStatusLED(void *pvParam)
{
  JsonDocument doc;
  JsonObject config;
  if (xQueueReceive(queueStatusLED, &config, portMAX_DELAY) == pdPASS)
  {
    // serializeJsonPretty(config, Serial);
  }
  uint8_t pin;
  uint16_t delaytime;

  if (config["Pin"].isNull())
  {
    pin = 14;
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "沒設定Pin，改為默認值%d\n", pin);
  }
  else
    pin = config["Pin"].as<uint8_t>();

  Adafruit_NeoPixel strip(1, pin, NEO_GRB + NEO_KHZ800);
  strip.begin();
  if (xSemaphoreTake(rmtMutex, portMAX_DELAY))
  {
    strip.show();
    xSemaphoreGive(rmtMutex);
  }
  if (config["DelayTime"].isNull())
  {
    delaytime = 100;
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "沒設定DelayTime,設定為%dms!\n", delaytime);
  }
  if (config["Color_R"].isNull() ||
      config["Color_G"].isNull() ||
      config["Color_B"].isNull())
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "沒設定Color結束狀態燈!\n");
    vTaskDelete(NULL);
  }
  else
  {
    delaytime = config["DelayTime"].as<uint16_t>();
  }
  if (config["Type"].isNull())
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "沒設定Type結束狀態燈!\n");
    _DELAY_MS(1000);
    vTaskDelete(NULL);
  }
  else
  {

    while (1)
    {
      if (xQueueReceive(queueStatusLED, &config, 0) == pdPASS)
        ;
      doLightRGB(&strip, config);
      // Serial.printf("%d,%d,%d\n", config["Color_R"].as<uint8_t>(), config["Color_G"].as<uint8_t>(), config["Color_B"].as<uint8_t>());

      _DELAY_MS(delaytime);
    }
  }
}
