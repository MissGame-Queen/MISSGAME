#include "Template.h"
JsonPTC *Template_JsonPTC;
uint8_t Template_WiFi_Connect_Time = 0;
uint8_t Template_WiFi_Reconnect_Time = 0;
SocketIOclient socketIO_Client;
WiFiMulti wifiMulti;
void MQTT_Callback(char *topic, byte *payload, unsigned int length)
{
  JsonDocument doc;
  JsonArray args = doc.to<JsonArray>();
  args.add(topic);
  DeserializationError error = deserializeJson(args[1], payload, length);
  String str(payload, length);
  //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "收到新訊息!Topic: %s\nMessage:%s\n", topic, str.c_str());
  if (error)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "反序列化失敗:%s，以字串模式運行!\n", error.c_str());
  }
  else
  {
    // serializeJsonPretty(doc, Serial);
    myCmdTable_Json(&doc);
  }
  doc.clear();
}
void MQTT_Subscribe(PubSubClient *MQTTClient)
{
  if (!(*Template_JsonPTC->getJsonObject())["MQTT_SUBSCRIBE"]["Value"].isNull())
  {
    for (JsonVariant item : (*Template_JsonPTC->getJsonObject())["MQTT_SUBSCRIBE"]["Value"].as<JsonArray>())
    {
      MQTTClient->subscribe(item.as<String>().c_str());
    }
  }
  if (!(*Template_JsonPTC->getJsonObject())["_MODULE_ID"]["Value"].isNull())
    MQTTClient->subscribe((*Template_JsonPTC->getJsonObject())["_MODULE_ID"]["Value"].as<String>().c_str());
}
void socketIOEvent(socketIOmessageType_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case sIOtype_DISCONNECT:
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "[IOc] SocketIO已斷開連接!");
    _T_E2JS(_STATUSLED)
    ["Color_R"] = 0;
    JsonObject obj = _T_E2JS(_STATUSLED);
    xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
  }
  break;
  case sIOtype_CONNECT:
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] SocketIO連線成功,url: %s\n", payload);
    // join default namespace (no auto join in Socket.IO V3)
    socketIO_Client.send(sIOtype_CONNECT, "/"); //! 不能刪會無法連線
    _T_E2JS(_STATUSLED)
    ["Color_R"] = 255;
    JsonObject obj = _T_E2JS(_STATUSLED);
    xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
  }
  break;
  case sIOtype_EVENT:
  {
    char *sptr = NULL;
    int id = strtol((char *)payload, &sptr, 10);
    //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] get event: %s\n", payload);
    if (id)
    {
      payload = (uint8_t *)sptr;
    }
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, payload, length);
    if (error)
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "deserializeJson() failed: %s\n", error.c_str());
      return;
    }

    String eventName = doc[0];
    //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] event name: %s , id: %d\n", eventName.c_str(), id);
    myCmdTable_Json(&doc);

    // 如果訊息id包含ACK(回呼)
    // FIXME 記得補ACK
    if (id)
    {
      /*
            // creat JSON message for Socket.IO (ack)
            JsonDocument docOut;
            JsonArray array = docOut.to<JsonArray>();
            // add payload (parameters) for the ack (callback function)
            JsonObject param1 = array.createNestedObject();
            param1["now"] = millis();

            // JSON to String (serializion)
            String output;
            output += id;
            serializeJson(docOut, output);
            //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "ACK : %s\n", output.c_str());
            // Send event
            socketIO_Client.send(sIOtype_ACK, output);
            */
    }
  }
  break;
  case sIOtype_ACK:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] get ack: %u\n", length);
    break;
  case sIOtype_ERROR:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] get error: %u\n", length);
    break;
  case sIOtype_BINARY_EVENT:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] get binary: %u\n", length);
    break;
  case sIOtype_BINARY_ACK:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[IOc] get binary ack: %u\n", length);
    break;
  }
}

/**
 * @brief 初始化系統用參數>設定系統FS>確認系統參數表是否存在
 *
 * @return int8_t
 */
int8_t Init()
{
  xSemaphoreGive(rmtMutex); // 釋放RMT資源鎖
  xSemaphoreGive(SPIMutex); // 釋放SPI資源鎖
  xSemaphoreGive(I2CMutex); // 釋放I2C資源鎖
  int8_t reVal = 1;
  if (!SPIFFS.begin(true))
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "掛載SPIFFS分區出錯啦");
    reVal = -1;
  }
  if (_T_E2JS(_FILE_CONFIG_FS).as<String>() == "SD")
  {
    if (CheckFile(_CONFIG_SD) == 1)
    {
      _T_E2JS(_FILE_CONFIG_FS) = "SD";
      return _CONFIG_SD;
    }
  }
  else if (_T_E2JS(_FILE_CONFIG_FS).as<String>() == "SPIFFS")
  {
    if (CheckFile(_CONFIG_SPIFFS) == 1)
    {
      _T_E2JS(_FILE_CONFIG_FS) = "SPIFFS";
      return _CONFIG_SPIFFS;
    }
  }
  else
  {
    if (CheckFile(_CONFIG_SD) == 1)
    {
      _T_E2JS(_FILE_CONFIG_FS) = "SD";
      return _CONFIG_SD;
    }

    if (CheckFile(_CONFIG_SPIFFS) == 1)
    {
      _T_E2JS(_FILE_CONFIG_FS) = "SPIFFS";
      return _CONFIG_SPIFFS;
    }
  }
  _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "請檢查指令表!");
  while (1)
    _DELAY_MS(5000);
  return reVal;
}
/**
 * @brief 根據Type回傳FS指標
 *
 * @param usetype _FILE_CONFIG_FS,_FILE_DATA_FS
 * @return FS*
 */
FS *getFS(String usetype)
{
  FS *prt;
  if ((Template_System_Obj[usetype]).isNull())
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "找不到參數名稱[%s]!\n", usetype.c_str());
  }
  if (((Template_System_Obj[usetype]).as<String>() == "AUTO" || (Template_System_Obj[usetype]).as<String>() == "SD"))
    prt = &SD;
  else
    prt = &SPIFFS;
  return prt;
}
/**
 * @brief 列舉檔案
 *
 * @param config 類型
 */
void listFile(configType_e config)
{
  if (_CONSOLE_PRINT_LEVEL >= _PRINT_LEVEL_INFO)
  {
    File root;
    FS *ptrFS = 0;

    if (config == _CONFIG_SD)
    {
      ptrFS = &SD;
      root = ptrFS->open("/");
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "SD分區(已使用大小/總大小):%lld/%lld\n", SD.usedBytes(), SD.totalBytes());
    }
    else if (config == _CONFIG_SPIFFS)
    {
      ptrFS = &SPIFFS;
      root = ptrFS->open("/");
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "SPIFFS分區(已使用大小/總大小):%d/%d\n", SPIFFS.usedBytes(), SPIFFS.totalBytes());
    }
    File file = root.openNextFile();
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "檔名:");
    while (file)
    {
      _CONSOLE_PORT.printf("%s [%d]\n", file.path(), file.size());
      file = root.openNextFile();
    }
  }
}
/**
 * @brief 檢查FS是否有參數表
 *
 * @param config 類型
 * @return 1 讀取成功
 * @return <0 讀取失敗
 */
int8_t CheckFile(configType_e config)
{
  enum ERROR_CODE
  {
    NO_CONFIG = 0,
    SD_ERROR = -1,
    SD_NO_CARD = -2,
    NO_SETFILER = -3,
    SPIFFS_ERROR = -4
  };
  int8_t reVal = 1;
  FS *ptrFS = 0;
  if (!SPIFFS.begin(true))
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "掛載SPIFFS分區出錯啦");
    reVal = SPIFFS_ERROR;
  }
  if (config == _CONFIG_SD)
  {
    if (!SD.begin(_T_E2JS(_PIN_SD_CS).as<uint8_t>()))
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "掛載SD卡出錯");
      reVal = SD_ERROR;
    }
    else
    {
      if (SD.cardType() == CARD_NONE)
      {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "沒有插入SD卡");
        reVal = SD_NO_CARD;
      }
      else
        ptrFS = &SD;
    }
  }
  else if (config == _CONFIG_SPIFFS)
  {
    ptrFS = &SPIFFS;
  }
  if (reVal >= 1)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "系統參數模式=%s\n", config == _CONFIG_SD ? "SD" : "SPIFFS");
    // 列舉全部檔案
    listFile(config);

    if (!ptrFS->exists("/www/index.html") || !ptrFS->exists(JsonPTC::csFileName_SYSTEM))
    {
      reVal = NO_SETFILER;
      _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "網站或指令集未上傳");

      // _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"網站index是否已上傳=%d\n", ptrFS->exists("/www/index.html"));
      // _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"指令表是否已上傳=%d\n", ptrFS->exists(csFileName_CMD));
      // _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"是否成功刪除檔案=%d\n", SPIFFS.remove("/SPIFFS.test"));
      // _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"是否成功重新命名檔案=%d\n", SPIFFS.rename("/SPIFFS.test", "/SPIFFS.test"));
      // SPIFFS.open("/SPIFFS.test","w");//開啟檔案(絕對路徑,模式:w=覆寫，r=唯讀，a=附加，w+=覆寫+讀，r+=讀取+從頭寫入，a+=附加+讀取)
    }
  }
  return reVal;
}
/**
 * @brief 讀取網路時間
 * 第一次讀取較慢，連上網路校正後就會比較快
 *
 * @return true
 * @return false
 */
bool readDate()
{

  bool rtSW = true;
  static bool isFirst = true;
  if (!isConnect(1))
    rtSW = false;
  // 避免沒網路情況下計時系統失效
  if (isFirst && rtSW)
  {
    configTime(28800, 0, "time.windows.com");
    if (getLocalTime(&_T_localtm, 1000))
    {
      isFirst = false;
      char str[64];
      strftime(str, sizeof(str), "初次連線時間：%Y/%m/%d %H:%M:%S", &_T_localtm);
      _CONSOLE_PRINTLN(_PRINT_LEVEL_NONE, str);
      _LOG_PRINTLN(_PRINT_LEVEL_NONE, str);
    }
  }
  else
  {
    // 讀取時間(讀取要等一段時間才會正確)
    if (!getLocalTime(&_T_localtm, 0))
    {
      //_CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "時間讀取錯誤");
      rtSW = false;
    }
  }
  return rtSW;
}
/**
 * @brief 判斷目前是否連線
 *
 * @return true
 * @return false
 */

/**
 * @brief 判斷目前是否連線
 *
 * @param level 2:需能連外網,1:若有分配ip,0:若回傳為WL_CONNECTED
 * @return true
 * @return false
 */
bool isConnect(uint8_t level)
{

  if (WiFi.getMode() == WIFI_MODE_NULL)
  {
    return false;
  }
  // 是否為連線狀態

  if (WiFi.status() != WL_CONNECTED)
  {
    return false;
  }
  // 检查是否获得有效的 IP 地址

  if (WiFi.localIP() == IPAddress(0, 0, 0, 0) && level >= 1)
  {
    return false;
  }

  // 是否已更新時間

  if ((_T_localtm.tm_year) + 1900 < 2024 && level >= 2)
  {
    return false;
  }

  return true;
}
/**
 * @brief 等待WiFi連線
 *
 * @param maxSecond 等待的秒數內無回應則返回否，0=永久等待
 * @param level 視情況調整，有時無法連外網所以要低於等級2
 * @return true
 * @return false
 */
bool waitConnect(uint16_t maxSecond, uint8_t level)
{
  uint16_t time = 0;
  maxSecond *= 100;
  while (!isConnect(level) && (time <= maxSecond || maxSecond == 0))
  {
    time++;
    vTaskDelay(10 / portTICK_RATE_MS);
  }

  return time > maxSecond ? false : true;
}
/**
 * @brief 嘗試重連網路
 *
 * @return true
 * @return false
 */
void WiFiReconnect()
{
    if (WiFi.getMode() != WIFI_MODE_STA && WiFi.getMode() != WIFI_MODE_APSTA)
    return;
  if (WiFi.status() == WL_CONNECTED)
    return;
      static uint32_t timer = 0;
  // 如果沒AP連接則嘗試重連N次，不行就重置系統
  if (WiFi.softAPgetStationNum() == 0 &&millis() > timer + 5000)
  {
    _T_E2JS(_STATUSLED)
    ["Color_B"] = 0;
    JsonObject obj = _T_E2JS(_STATUSLED);
    xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "重連中...%d\n", Template_WiFi_Reconnect_Time);
    wifiMulti.run();
    Template_WiFi_Reconnect_Time++;
    if (WiFi.status() == WL_CONNECTED)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "重連成功!");
      _T_E2JS(_STATUSLED)
      ["Color_B"] = 255;
      JsonObject obj = _T_E2JS(_STATUSLED);
      xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
    }
        timer = millis();
  }
  // FIXME 待修復重置系統功能
  /*
  if (Template_WiFi_Reconnect_Time > Template_WiFi_Connect_Time && Template_WiFi_Connect_Time > 0)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "重連錯誤!5秒後重開");
    vTaskDelay(5000 / portTICK_RATE_MS);
    ESP.restart();
  }
  */
}

/**
 * @brief 設定熱點和WiFi連線
 *
 * @param obj 輸入的JSON句柄
 * @return true
 * @return false
 */
bool WiFi_Connet(JsonObject *obj)
{

  // 如果_WIFI_TYPE異常則套用預設
  if ((*obj)[_E2S(_WIFI_TYPE)]["Value"].isNull() || (*obj)[_E2S(_WIFI_TYPE)]["Value"].as<uint16_t>() >= WIFI_MODE_MAX)
  {
    // 如果沒設定STA_ID則設定成AP模式
    if ((*obj)[_E2S(STA)]["Value"].isNull() || (*obj)[_E2S(STA)]["Value"].as<JsonArray>().size() < 1)
      (*obj)[_E2S(_WIFI_TYPE)]["Value"] = WIFI_MODE_AP; // AP
    // 否則為AP+STA模式
    else
      (*obj)[_E2S(_WIFI_TYPE)]["Value"] = WIFI_MODE_STA; // AP+STA
  }
  // 如果為配置模式或STA||APSTA但沒設定STA_ID則改為AP模式
  else if (_T_E2JS(_TYPE_SET) ||
           ((*obj)[_E2S(STA)]["Value"].isNull() || (*obj)[_E2S(STA)]["Value"].as<JsonArray>().size() < 1) &&
               ((*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>() == WIFI_MODE_STA ||
                (*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>() == WIFI_MODE_APSTA))
    (*obj)[_E2S(_WIFI_TYPE)]["Value"] = WIFI_MODE_AP;
  bool rtval = true;
  WiFi.setSleep(false);
  WiFi.disconnect();
  //?不知為何mode會佔用大量內存
  WiFi.mode((*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>());
  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "設定WiFi為:%d\n", (*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>());

  // 如果為AP或APSTA則設定AP相關參數
  if (((*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>() == WIFI_MODE_AP ||
       (*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>() == WIFI_MODE_APSTA))
  {
    // 手動設定位址、閘道、遮罩
    WiFi.softAPConfig(IPAddress(192, 168, 1, 1),    // IP位址
                      IPAddress(192, 168, 1, 1),    // 閘道位址
                      IPAddress(255, 255, 255, 0)); // 網路遮罩
    // 如果沒設定APNAME則套用HOSTNAME
    if ((*obj)[_E2S(AP_SSID)]["Value"].isNull() || (*obj)[_E2S(AP_SSID)]["Value"].as<String>() == "")
      (*obj)[_E2S(AP_SSID)]["Value"].set((*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<const char *>());

    /*
      Serial.print("舊MAC碼:");
      Serial.println(WiFi.macAddress());
      const byte nwemac[] {0x12, 0x34, 0x56, 0x78, 0x90, 0xAB};
      esp_wifi_set_mac(WIFI_IF_STA, nwemac);
      Serial.print("新MAC碼:");
      Serial.println(WiFi.macAddress());
    */
    int8_t status = 0;

    if (!(*obj)[_E2S(AP_PASSWORD)]["Value"].isNull() && (*obj)[_E2S(AP_PASSWORD)]["Value"].as<String>() != "")
      status = WiFi.softAP((*obj)[_E2S(AP_SSID)]["Value"].as<const char *>(), (*obj)[_E2S(AP_PASSWORD)]["Value"].as<const char *>());
    else
      status = WiFi.softAP((*obj)[_E2S(AP_SSID)]["Value"].as<const char *>());

    if (status == false)
      _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "AP模式建立失敗!");
  }
  if (((*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>() == WIFI_MODE_STA ||
       (*obj)[_E2S(_WIFI_TYPE)]["Value"].as<wifi_mode_t>() == WIFI_MODE_APSTA))
  {

    for (JsonVariant item : (*obj)[_E2S(STA)]["Value"].as<JsonArray>())
    {
      wifiMulti.addAP(item["SSID"].as<const char *>(), item["PASSWORD"].as<const char *>());
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "新增SSID:%s\n", item["SSID"].as<const char *>());
    }
    wifiMulti.run();
  }

  return rtval;
}
/**
 * @brief  WiFi 事件處理程序
 *
 * @param event 類型
 * @param info 相關資訊
 */
void onWiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info)
{
  //_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "event=%d\n",event);
  switch (event)
  {
  case ARDUINO_EVENT_WIFI_STA_START:
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "嘗試連線中...");
    break;
  case ARDUINO_EVENT_WIFI_STA_CONNECTED:
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已接入:%s\n", WiFi.SSID().c_str());
    _T_E2JS(_STATUSLED)
    ["Color_B"] = 255;
    JsonObject obj = _T_E2JS(_STATUSLED);
    xQueueSend(queueStatusLED, &obj, portMAX_DELAY);
    Template_WiFi_Reconnect_Time = 0;
  }
  break;
  case ARDUINO_EVENT_WIFI_STA_GOT_IP:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "IP address: http://%s\n", WiFi.localIP().toString().c_str());
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "RSSI強度:%d\n", WiFi.RSSI());
    break;
  case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
    WiFiReconnect();
    break;
  case ARDUINO_EVENT_WIFI_AP_START:
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已建立熱點:%s,IP: %s\n", WiFi.softAPSSID().c_str(), WiFi.softAPIP().toString().c_str());
    break;

  case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
  {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1], info.wifi_ap_staconnected.mac[2],
            info.wifi_ap_staconnected.mac[3], info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "有設備接入! MAC address:%s\n", macStr);
  }
  break;
  case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
  {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1], info.wifi_ap_staconnected.mac[2],
            info.wifi_ap_staconnected.mac[3], info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "有設備斷開! MAC address:%s\n", macStr);
  }
  break;

  case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
  {
    char macStr[18];
    sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
            info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1], info.wifi_ap_staconnected.mac[2],
            info.wifi_ap_staconnected.mac[3], info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5]);
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "分配 [%s] IP address: %s \n", macStr, IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());
  }
  break;

  default:
    break;
  }
}

/**
 * @brief WiFi連線背景執行程序
 *
 * @param pvParam 輸入JSON句柄
 */
void WiFiInit(void *pvParam)
{
  JsonObject *obj = (JsonObject *)pvParam;
  if (!obj)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "輸入參數錯誤!");
    vTaskDelete(NULL); // 任務關閉
  }
  /*
{
    ARDUINO_EVENT_WIFI_READY = 0,                // WiFi 模組就緒
    ARDUINO_EVENT_WIFI_SCAN_DONE,                // WiFi 掃描完成
    ARDUINO_EVENT_WIFI_STA_START,                // WiFi 站點模式已啟動
    ARDUINO_EVENT_WIFI_STA_STOP,                 // WiFi 站點模式已停止
    ARDUINO_EVENT_WIFI_STA_CONNECTED,            // WiFi 站點已連接至 AP
    ARDUINO_EVENT_WIFI_STA_DISCONNECTED,         // WiFi 站點已斷開連接
    ARDUINO_EVENT_WIFI_STA_AUTHMODE_CHANGE,      // WiFi 站點驗證模式變更
    ARDUINO_EVENT_WIFI_STA_GOT_IP,               // WiFi 站點獲得 IP 位址
    ARDUINO_EVENT_WIFI_STA_GOT_IP6,              // WiFi 站點獲得 IPv6 位址
    ARDUINO_EVENT_WIFI_STA_LOST_IP,              // WiFi 站點失去 IP 位址
    ARDUINO_EVENT_WIFI_AP_START,                 // WiFi 存取點模式已啟動
    ARDUINO_EVENT_WIFI_AP_STOP,                  // WiFi 存取點模式已停止
    ARDUINO_EVENT_WIFI_AP_STACONNECTED,          // WiFi 存取點模式下有站點連接
    ARDUINO_EVENT_WIFI_AP_STADISCONNECTED,       // WiFi 存取點模式下有站點斷開連接
    ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED,         // WiFi 存取點模式下為站點分配 IP 位址
    ARDUINO_EVENT_WIFI_AP_PROBEREQRECVED,        // WiFi 存取點模式收到探測請求
    ARDUINO_EVENT_WIFI_AP_GOT_IP6,               // WiFi 存取點模式獲得 IPv6 位址
    ARDUINO_EVENT_WIFI_FTM_REPORT,               // WiFi FTM（Fine Timing Measurement）報告
    ARDUINO_EVENT_ETH_START,                     // 以太網模式已啟動
    ARDUINO_EVENT_ETH_STOP,                      // 以太網模式已停止
    ARDUINO_EVENT_ETH_CONNECTED,                 // 以太網連接已建立
    ARDUINO_EVENT_ETH_DISCONNECTED,              // 以太網連接已斷開
    ARDUINO_EVENT_ETH_GOT_IP,                    // 以太網獲得 IP 位址
    ARDUINO_EVENT_ETH_GOT_IP6,                   // 以太網獲得 IPv6 位址
    ARDUINO_EVENT_WPS_ER_SUCCESS,                // WPS 配置成功
    ARDUINO_EVENT_WPS_ER_FAILED,                 // WPS 配置失敗
    ARDUINO_EVENT_WPS_ER_TIMEOUT,                // WPS 配置超時
    ARDUINO_EVENT_WPS_ER_PIN,                    // WPS PIN 配置成功
    ARDUINO_EVENT_WPS_ER_PBC_OVERLAP,            // WPS PBC 配置重疊
    ARDUINO_EVENT_SC_SCAN_DONE,                  // 安全連接掃描完成
    ARDUINO_EVENT_SC_FOUND_CHANNEL,              // 安全連接發現通道
    ARDUINO_EVENT_SC_GOT_SSID_PSWD,              // 安全連接獲得 SSID 和密碼
    ARDUINO_EVENT_SC_SEND_ACK_DONE,              // 安全連接傳送 ACK 完成
    ARDUINO_EVENT_PROV_INIT,                     // 啟動裝置配對
    ARDUINO_EVENT_PROV_DEINIT,                   // 結束裝置配對
    ARDUINO_EVENT_PROV_START,                    // 開始裝置配對
    ARDUINO_EVENT_PROV_END,                      // 結束裝置配對
    ARDUINO_EVENT_PROV_CRED_RECV,                // 收到裝置配對憑證
    ARDUINO_EVENT_PROV_CRED_FAIL,                // 裝置配對憑證失敗
    ARDUINO_EVENT_PROV_CRED_SUCCESS,             // 裝置配對憑證成功
    ARDUINO_EVENT_MAX
}
*/
  Template_WiFi_Connect_Time = (*obj)[_E2S(_WIFI_CONNECT_TIME)].isNull() ? 20 : (*obj)[_E2S(_WIFI_CONNECT_TIME)]["Value"].as<uint16_t>();
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_START);
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STACONNECTED);
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STADISCONNECTED);
  WiFi.onEvent(onWiFiEvent, WiFiEvent_t::ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED);

  String strMAC = WiFi.macAddress();
  // 如果沒設定WIFI_HOSTNAME則默認為設備號碼
  if ((*obj)[_E2S(WIFI_HOSTNAME)].isNull() || (*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<String>() == "")
  {
    strMAC.replace(":", "_");
    (*obj)[_E2S(WIFI_HOSTNAME)]["Value"] = "_" + strMAC;
  }
  else
    strMAC = (*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<String>();
  //  _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, (*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<String>());
  if (!WiFi.setHostname((*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<const char *>()))
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "設置設備名稱時出錯了!");
  else
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已設置設備名稱: %s\n", (*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<const char *>());

  // 如果沒設定WIFI_MDNSNAME則默認為設備號碼或HOSTNAME
  if ((*obj)[_E2S(WIFI_MDNSNAME)].isNull() || (*obj)[_E2S(WIFI_MDNSNAME)]["Value"].as<String>() == "")
    (*obj)[_E2S(WIFI_MDNSNAME)]["Value"].set((*obj)[_E2S(WIFI_HOSTNAME)]["Value"].as<String>());

  WiFi_Connet(obj); //?注意順序 實體名稱>連線處理>mDNS>Sever
  if (!MDNS.begin((*obj)[_E2S(WIFI_MDNSNAME)]["Value"].as<const char *>()))
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "設置mDNS時出錯了!");
  else
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已設置mDNS: http://%s.local/\n", (*obj)[_E2S(WIFI_MDNSNAME)]["Value"].as<const char *>());
  ServerInit();
  /*_CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "任務資訊:\n名稱:taskWiFiConnet\n優先度為:%d\n占用資源為:%d\n執行核心為:%d\n",
                  uxTaskPriorityGet(NULL), uxTaskGetStackHighWaterMark(NULL), xPortGetCoreID());*/

  /*
    bool doCheckmDNS = false;
    while (doCheckmDNS)
    {
      // 花費3秒太久
      if (!MDNS.queryService("http", "tcp"))
      {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "mDNS異常!重啟mDNS服務...");
        MDNS.end();
        MDNS.begin((*obj)["WIFI_MDNSNAME"]["Value"].as<const char *>());
      }
    }
    */
  vTaskDelete(NULL);
}
void deleteOldFile(size_t sdCardCapacityLimit, String format, String path)
{
  uint8_t typeFS = 0;
  if (_T_E2JS(_FILE_DATA_FS) == "SD")
  {
    if (SD.cardType() != CARD_NONE)
      typeFS = 1;
  }

  // 定义文件对象
  File myFile;
  // 判斷剩餘容量並刪除檔案騰出空間
  // 取剩餘容量
  size_t freeSpace = typeFS ? SD.totalBytes() - SD.usedBytes() : SPIFFS.totalBytes() - SPIFFS.usedBytes();
  // 刪除最舊紀錄直至符合容量
  while (freeSpace < sdCardCapacityLimit)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "設定容量:%d，目前容量:%d\n", sdCardCapacityLimit, freeSpace);
    FS *prtFS = 0;
    typeFS ? prtFS = &SD : prtFS = &SPIFFS;
    File root = prtFS->open(path);
    struct tm oldDate = {0}; // 初始化为零
    while (freeSpace < sdCardCapacityLimit)
    {
      // 輪詢檔案取最舊日期
      File entry = root.openNextFile();
      if (!entry)
      {
        break;
      }
      struct tm fileDate = {0}; // 初始化为零
      // 从文件名中提取日期信息，格式为 "YYYYMMDD"
      int year = 0, month = 0, day = 0;
      uint8_t returnNumber = sscanf(entry.path(), format.c_str(), &year, &month, &day);
      fileDate.tm_year = year - 1900; // 年份需减去1900
      fileDate.tm_mon = month - 1;    // 月份从0开始
      fileDate.tm_mday = day;
      // 如果有抓到3筆回傳且檔案日期比目前最舊日期還舊
      if (returnNumber == 3 && (difftime(mktime(&oldDate), mktime(&fileDate)) > 0))
      {
        oldDate = fileDate;
      }
      entry.close();
    }
    root.close();
    char filename[20];
    if (oldDate.tm_year == 0)
    {
      _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "查無最舊資料!");
      return;
    }
    snprintf(filename, sizeof(filename), format.c_str(),
             oldDate.tm_year + 1900, oldDate.tm_mon + 1, oldDate.tm_mday);

    if (prtFS->remove(filename))
      _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "刪除檔案: %s\n", filename);
    else
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_ERROR, "錯誤!無法刪除檔案:%s\n", filename);
      vTaskDelay(10000 / portTICK_RATE_MS);
    }
    freeSpace = typeFS ? SD.totalBytes() - SD.usedBytes() : SPIFFS.totalBytes() - SPIFFS.usedBytes();
  }
}

JsonPTC::JsonPTC()
{
  _csDoc_CMD = new JsonDocument;
  _doCleanPtr = true;
}
JsonPTC::JsonPTC(JsonDocument *doc_CMD)
{
  _doCleanPtr = false;
  _csDoc_CMD = doc_CMD;
}
JsonPTC::~JsonPTC()
{

  if (_doCleanPtr)
    delete _csDoc_CMD;
}
/**
 * @brief 讀取.json到RAM
 *
 * @param prFS FS類別指針
 * @return int8_t 狀態碼
 */
int8_t JsonPTC::Begin(FS *fs, const char *fileName, bool doFilter)
{
  _csFS = fs;
  int8_t rtVal = 1;

  JsonDocument *ptrDoc = getJsonDocument();

  ptrDoc->clear();
  if (!_csFS->exists(fileName))
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "找不到%s!\n", fileName);
    rtVal = -1;
  }
  else
  {
    File file = _csFS->open(fileName, FILE_READ);
    DeserializationError error = deserializeJson(*ptrDoc, file);
    file.close();
    if (error)
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化失敗:%s\n", error.c_str());
      rtVal = -2;
    }
    else
    {
      _csObj_CMD = ptrDoc->as<JsonObject>();
      if (_csObj_CMD.isNull())
      {
        rtVal = -3;
        _CONSOLE_PRINTF(_PRINT_LEVEL_ERROR, "指標為空!\n");
        serializeJsonPretty(ptrDoc->as<JsonObject>(), Serial);
      }
      else if (doFilter)
      {
        // 過濾多餘資料僅保留Value
        JsonDocument filter;
        filter["*"]["Value"] = true; // 通配符，匹配所有顶级键
        DeserializationError error = deserializeJson(*ptrDoc, ptrDoc->as<String>(), DeserializationOption::Filter(filter));
      }
    }
  }
  return rtVal;
}
void JsonPTC::setCMD(String (*function)(JsonDocument *doc))
{
  _classCMD = function;
}
/**
 * @brief 獲取資料庫的根鍵指標
 *
 * @return JsonObject*
 */
JsonObject *JsonPTC::getJsonObject()
{
  return &_csObj_CMD;
}
JsonDocument *JsonPTC::getJsonDocument()
{
  // 檢查 _csDoc_CMD 是否為空
  if (_csDoc_CMD != nullptr)
  {
    return _csDoc_CMD;
  }
  else
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "指標為空!");
    serializeJsonPretty(*_csDoc_CMD, Serial);
    while (1)
      _DELAY_MS(3000);
  }
}
/**
 * @brief 將指令表的預設值覆寫到值
 *
 * @return int8_t 狀態碼
 */
int8_t JsonPTC::RstConfig()
{
  int8_t rtVal = 1;
  JsonObject *obj = getJsonObject();
  rtVal = Begin(_csFS, csFileName_SYSTEM, false);
  if (rtVal < 0)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "重置指令表錯誤!");
    rtVal = -1;
  }
  else
  {
    for (JsonPair item : *obj)
    {
      // 如果沒有設定值則帶入預設值
      if ((*obj)[item.key().c_str()]["Value"].isNull())
        // 覆寫成預設值
        (*obj)[item.key().c_str()]["Value"].set(item.value()["Default"]);
      // 如果有帶寄存器則順便寫進去
      if (!item.value()["Register"].isNull() && item.value()["Register"].as<String>() != "")
        (*obj)[item.value()["Register"].as<String>()].set(String(item.key().c_str()));
    }
  }
  return rtVal;
}
/**
 * @brief 將資料庫的資料存到對應位置
 *
 * @return int8_t int8_t 狀態碼
 */
int8_t JsonPTC::SaveConfig()
{
  int8_t rtVal = 1;
  if (_csFS->exists(csFileName_CMD))
    _csFS->remove(csFileName_CMD);
  // vTaskDelay(100 / portTICK_PERIOD_MS); // 增加延遲
  File file = _csFS->open(csFileName_CMD, FILE_WRITE);
  // vTaskDelay(100 / portTICK_PERIOD_MS); // 增加延遲
  if (!file)
  {
    _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, "無法開啟檔案");
    return -1;
  }
  else
  {
    // 不需要在這裡使用 flush
    serializeJsonPretty(*getJsonDocument(), file);
    // vTaskDelay(50 / portTICK_PERIOD_MS); // 增加延遲
    file.close(); // 要close才會正式存檔
    file = _csFS->open(csFileName_CMD, FILE_READ);
    size_t bytesWritten = file.size();
    file.close();
    // vTaskDelay(50 / portTICK_PERIOD_MS); // 增加延遲
    if (bytesWritten == 0)
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_ERROR, "寫入失敗\n");
      rtVal = -2;
    }
    else if (bytesWritten <= 4)
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_ERROR, "SD卡損壞，請更換SD卡!\n");
      rtVal = -3;
    }
  }
  file.close();
  return rtVal;
}

/**
 * @brief Json權限寫入或執行
 *如果無限制則寫入、執行設定值
 * @param obj
 * 格式:
{
 "TEST" : {"Value" : 0},
 _E2S(STA_PASSWORD) : {"Value" : 123},
 "_TH_OFFTIME" : {"Value" : 456},
 "_TH_TEMPERATURE" : {"Value" : 789},
 "ALARM_CLOCK" : {
   "Value" : [
     {
       "Time" : "07:55",
       "Long" : 2000
     },
     {
       "Time" : "08:00",
       "Long" : 4000
     }
   ]
 }
 }
Value會寫入、Function會執行
 * @param function 執行函式
 * (JSON[KEY],String)
 */
/*
void JsonPTC::Write(JsonDocument *obj)
{
  bool haveSave = false;
  for (JsonPair item : obj->as<JsonObject>())
  {
    const char *keyName = item.key().c_str();
    if (!_csObj_CFG.containsKey(keyName))
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "查無%s此鍵!\n", keyName);
    else
    {
      // 如果無限制寫入則寫入或執行
      String havelimit;
      havelimit = _csObj_CMD[keyName].containsKey("Limit") ? _csObj_CMD[keyName]["Limit"].as<String>() : "";
      if ((havelimit != "" && (havelimit.indexOf('W') != -1 || havelimit.indexOf('w') != -1)))
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%s]禁止寫入!(%s)\n", keyName, havelimit.c_str());
      else
      {
        JsonObject keyVal = item.value();
        if (keyVal.containsKey("Function"))
        {
          JsonDocument doc;
          doc[item.key()].set(item.value());
          _classCMD(&doc);
        }
        else if (keyVal.containsKey("Value"))
        {
          _csObj_CFG[keyName]["Value"].set(keyVal["Value"]);
        }

        else if (keyVal.containsKey("Save"))
          haveSave = true;
      }
    }
  }
  if (haveSave)
    Save(obj);
}
*/
/**
 * @brief Json權限讀取
 *如果無限制則回傳目前值，否則為"*，或者無此KEY為"null"
 * @param obj
 * 格式:
[
 {"TEST" : {"Value" : 0}},
 {_E2S(STA_PASSWORD) : {"Value" : 123}},
 {"_TH_OFFTIME" : {"Value" : 456}},
 {"_TH_TEMPERATURE" : {"Value" : 789}},
 {"ALARM_CLOCK" : {
   "Value" : [
     {
       "Time" : "07:55",
       "Long" : 2000
     },
     {
       "Time" : "08:00",
       "Long" : 4000
     }
   ]
 }}
 ]
 */
/*
void JsonPTC::Read(JsonDocument *obj)
{
  for (JsonPair item : obj->as<JsonObject>())
  {
    const char *keyName = item.key().c_str();
    if (!_csObj_CFG.containsKey(keyName))
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "查無[%s]此鍵!\n", keyName);
      item.value()["Value"] = "null";
    }
    else
    {
      // 如果無限制寫入則寫入或執行
      String havelimit;
      havelimit = _csObj_CMD[keyName].containsKey("Limit") ? _csObj_CMD[keyName]["Limit"].as<String>() : "";
      if ((havelimit != "" && (havelimit.indexOf('R') != -1 || havelimit.indexOf('r') != -1)))
      {
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%s]禁止讀取!(%s)\n", keyName, havelimit.c_str());
        item.value()["Value"] = "*";
      }
      else
      {
        item.value()["Value"].set(_csObj_CFG[keyName]["Value"]);
      }
    }
  }
}
*/
/*
void JsonPTC::Save(JsonDocument *obj)
{
  FS *prtFS = getFS();
  File file = prtFS->open(csFileName_CFG, FILE_READ);
  if (file)
  {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING, "反序列化錯誤:%s\n", error.c_str());
      return;
    }
    file.close();
    for (JsonPair item : obj->as<JsonObject>())
    {
      const char *keyName = item.key().c_str();
      if (!_csObj_CFG.containsKey(keyName))
        _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "查無%s此鍵!\n", keyName);
      else
      {
        // 如果無限制寫入則寫入或執行
        String havelimit;
        havelimit = _csObj_CMD[keyName].containsKey("Limit") ? _csObj_CMD[keyName]["Limit"].as<String>() : "";
        if ((havelimit != "" && (havelimit.indexOf('S') != -1 || havelimit.indexOf('s') != -1)))
          _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "[%s]禁止存檔!(%s)\n", keyName, havelimit.c_str());
        else
        {
          JsonObject keyVal = item.value();
          if (keyVal.containsKey("Save"))
          {
            JsonVariant valueVariant = keyVal["Save"];
            _csObj_CFG[keyName]["Value"].set(valueVariant);
            doc[keyName]["Value"].set(valueVariant);
          }
        }
      }
    }
    file = prtFS->open(csFileName_CFG, FILE_WRITE);
    serializeJsonPretty(doc, file);
    file.close();
  }
}
*/