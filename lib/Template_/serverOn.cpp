#include "serverOn.h"
/**
 * @brief 定義回調函數
 *
 */
void ServerInit()

{
  // 定義甚麼網址將打開甚麼檔案
  // Template_server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("index.html");
  Template_server.serveStatic("/img", SPIFFS, "/www/img/");
  Template_server.serveStatic("/favicon.ico", SPIFFS, "/www/favicon.ico/");
  /*
    // setAuthentication()中分别填入用户名和密码以限定訪問條件
    Template_server.serveStatic("/", SPIFFS, "/www/").setAuthentication("user", "pass");
    //設定執行條件
    Template_server.on("/", HTTP_GET, callback1).setFilter(ON_AP_FILTER); // ESP32工作在AP模式时起效
    Template_server.on("/", HTTP_GET, callback2).setFilter(ON_STA_FILTER); // ESP32工作在STA模式时起效

    Template_server.rewrite("/", "/ap").setFilter(ON_AP_FILTER); // ESP32工作在AP模式时起效
    Template_server.rewrite("/", "/sta").setFilter(ON_STA_FILTER); // ESP32工作在STA模式时起效

    bool userFilter(AsyncWebServerRequest *request){return true;} // 自定义规则
    Template_server.on("/", HTTP_GET, callback).setFilter(userFilter); // 当userFilter返回true时起效

    //通用情況註冊
    Template_server.onNotFound(onRequestCallBack);
    Template_server.onFileUpload(onUploadCallBack);
    Template_server.onRequestBody(onBodyCallBack);
    Template_server.on("/about", HTTP_GET, [](AsyncWebServerRequest * request) {
    //讀取HTPP請求訊息
    request->version();       // uint8_t: 0 = HTTP/1.0, 1 = HTTP/1.1
    request->method();        // enum:    HTTP_GET, HTTP_POST, HTTP_DELETE, HTTP_PUT, HTTP_PATCH, HTTP_HEAD, HTTP_OPTIONS
    // HTTP_GET = 0b00000001, HTTP_POST = 0b00000010, HTTP_DELETE = 0b00000100, HTTP_PU = 0b00001000, HTTP_PATCH = 0b00010000, HTTP_HEAD = 0b00100000, HTTP_OPTIONS = 0b01000000, HTTP_ANY = 0b01111111
    request->url();           // String:  URL of the request (not including host, port or GET parameters)
    request->host();          // String:  The requested host (can be used for virtual hosting)
    request->contentType();   // String:  ContentType of the request (not avaiable in Handler::canHandle)
    request->contentLength(); // size_t:  ContentLength of the request (not avaiable in Handler::canHandle)
    request->multipart();     // bool:    True if the request has content type "multipart"
    or
    for(int i=0;i<request->headers();i++){
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"HEADER[%s]: %s\n", request->headerName(i).c_str(), request->header(i).c_str());
    }

    //跳轉到指定網頁
    request->redirect("/login"); //to local url
    request->redirect("https://www.baidu.com/"); //to external url

    //訪問指定頁面時跳轉到指定內部頁面
    Template_server.rewrite("/", "/lalala"); // 用户访问"/"时，服务器内部会执行"/lalala"的回调函数

    // 用户访问/目录下文件时返回SPIFFS中/www/路径下同名文件
    // 比如用户访问/page.htm时，将会返回SPIFFS中/www/page.htm文件
    // 比如用户访问/favicon.ico时，将会返回SPIFFS中/www/favicon.ico文件
    // 特殊情况：用户访问/时，默认将会返回SPIFFS中/www/index.htm文件（如果存在）
    Template_server.serveStatic("/", SPIFFS, "/www/");
    // 用户访问/时，默认返回将变成SPIFFS中/www/default.html文件
    Template_server.serveStatic("/", SPIFFS, "/www/").setDefaultFile("default.html");

    // 设定缓存事件600秒，客户端获取过的文件将在客户端缓存600秒
    // 600秒内需要已缓存的文件将直接使用缓存，不再从服务器获取
    Template_server.serveStatic("/", SPIFFS, "/www/").setCacheControl("max-age=600");

    // 使用下面方式就可以随时更改Cache-Control
    AsyncStaticWebHandler* handler = &Template_server.serveStatic("/", SPIFFS, "/www/").setCacheControl("max-age=600");
    handler->setCacheControl("max-age=30");
    handler->setDefaultFile("default.html");
    request->send(200, Template_HTTP_ContentType_Text, "是在哈囉喔？");
    });
  */
  //[ ]根目錄
  Template_server.on(
      "/", HTTP_GET, [](AsyncWebServerRequest *request)
      {            
          FS *prtFS = 0;
   _T_E2JS(_FILE_CONFIG_FS).as<String>() == "SD" ? prtFS = &SD : prtFS = &SPIFFS;
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
  //[ ]重開機
  Template_server.on(
      "/restart", HTTP_GET, [](AsyncWebServerRequest *request)
      {
    String senddata = "重開機中....請稍後點擊回首頁\n";
    senddata += "<a href='/'>回首頁</a>";
    request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, senddata);
    vTaskDelay(pdMS_TO_TICKS(3000));
    // 重新啟動ESP32 
    ESP.restart(); });
  //[ ]韌體更新
  Template_server.on(
      "/firmwareupload", HTTP_POST, [](AsyncWebServerRequest *request)
      {
        String senddata="";
        bool isOK=true;
    // 檢查是否有附帶文件
        if (!request->hasParam("firmware", true,true))
        {
          senddata = "未選擇固件文件！\n";
          isOK = false;
        }
        else if (Update.hasError())
        {
          senddata = "更新失敗!\n";
          isOK = false;
        }
        else
        {
          senddata = "更新成功!\n";
        }
    senddata += "3秒後重開機並導向首頁!\n<a href='/'>回首頁</a>";
    if(isOK)
    request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Json, "{\"status\":\"OK\"}");
    else
    request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Json, "{\"status\":\"ERROR\"}");
    vTaskDelay(pdMS_TO_TICKS(100));
    //request->send(HTTP_CODE_SEE_OTHER, Template_HTTP_ContentType_Text, "/");
    // 重新啟動ESP32
   if(isOK) ESP.restart(); },
      firmwareUpdate);
  //[ ]上傳檔案到SPIFFS
  Template_server.on(
      "/upload", HTTP_POST, [](AsyncWebServerRequest *request)
      {
        String senddata = ((Update.hasError()) ? "上傳失敗～\n" : "上傳成功！\n");
        senddata += "<a href='/'>回首頁</a>";
        request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, senddata); },
      configUpdate);
  /*
    //[ ]搜索周邊WiFi
    Template_server.on(
        "/WIFI_Search", HTTP_GET, [](AsyncWebServerRequest *request)
        {
                // 在此處處理"/WIFI_Search"的GET請求
                // 回傳包含網路信號強度和網路名稱清單的JSON回應

                // 假設你已獲得網路信號強度和網路名稱清單的資訊
                esp_task_wdt_reset();
                // 增加看门狗定时器的超时时间为20秒
                esp_task_wdt_init(40000, true);
                int networkCount = WiFi.scanNetworks();
                esp_task_wdt_reset();
                // 增加看门狗定时器的超时时间为10秒
                esp_task_wdt_init(10000, true);
                // 建立一個JSON物件並設定網路信號強度和網路名稱清單的屬性
                JsonDocument json;
                JsonArray networkArray = json["networkList"].to<JsonArray>();
                for (int i = 0; i < networkCount; i++)
                {
                  networkArray.add(WiFi.SSID(i));
                }
                json["RSSI"] = WiFi.RSSI();
                json["WIFI_MODE"] = WiFi.getMode();
                json["SSID"] = WiFi.SSID();
                json["IP"] = WiFi.localIP().toString();

                // 將JSON物件轉換為字串
                String jsonStr;
                serializeJson(json, jsonStr);
                 request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Json, jsonStr); });
    //[ ]連線到特定WiFi
    // FIXME記得修復純AP模式下能修改參數的方法
    Template_server.on(
        "/WIFI_Connect", HTTP_POST, [](AsyncWebServerRequest *request)
        {
                String network;
                String password;
                if (request->hasArg("network") && request->hasArg("password"))
                {
                  // 读取ID和密码参数
                  network = request->arg("network");
                  password = request->arg("password");
                }
                else if (request->hasParam("network") && request->hasParam("password"))
                {
                  network = request->getParam("network")->value();
                  password = request->getParam("password")->value();
                }
                else
                {
                  _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING,"缺少參數!");

                  // 获取原始 POST 数据
                  if (request->contentLength())
                  {
                    AsyncWebParameter *postParam;
                    String postData;
                    int paramsCount = request->params();
                    for (int i = 0; i < paramsCount; i++)
                    {
                      postParam = request->getParam(i);
                      if (postParam->isFile())
                      {
                        // 忽略文件类型参数
                        continue;
                      }
                      postData += postParam->name() + "=" + postParam->value();
                      if (i < paramsCount - 1)
                      {
                        postData += "&";
                      }
                    }
                    _CONSOLE_PRINTF(_PRINT_LEVEL_WARNING,"POST data: %s\n" , postData.c_str());
                  }

                  request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, "缺少參數");
                }
                 //FIXME 如果斷開重連request會失效，且如果等太久再呼叫request會記憶體錯誤

                  _CONSOLE_PRINTF(_PRINT_LEVEL_INFO,"嘗試重連%s，%s\n",network.c_str(),password.c_str());

                WiFi.begin(network.c_str(), password.c_str());
                byte Timeout = 0;
                while (WiFi.status() != WL_CONNECTED && Timeout <= 12)
                {
                  Timeout++;
                  vTaskDelay(500 / portTICK_RATE_MS);
                  if (Timeout > 11)
                  {
                    request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, "連結錯誤!");
                    return;
                  }
                }
                String strIP=WiFi.localIP().toString();
                 request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text,strIP); });
    Template_server.on(
        "/FunctionJson", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *ptdata, size_t len, size_t index, size_t total)
        {
          String rtStr = "";
          JsonDocument jsonDocument;
          DeserializationError error = deserializeJson(jsonDocument, ptdata, len);
          if (error)
          {
            rtStr = "反序列化錯誤:" + String(error.c_str());
            _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, rtStr);
            request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, rtStr);
          }
          else
          {
            rtStr = myCmdTable_Json(&jsonDocument);
            // serializeJsonPretty(jsonDocument, rtStr);
            AsyncResponseStream *response = request->beginResponseStream(Template_HTTP_ContentType_Json);
            response->setCode(HTTP_CODE_OK);
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->println(rtStr);
            request->send(response);
          }
        });
    Template_server.on(
        "/getCMD", HTTP_GET, [](AsyncWebServerRequest *request)
        {
      FS *prtFS = getFS();
      if (!request->hasParam("JS"))
      {
        File file = prtFS->open("/www/listCMD.html", "r");
        if (file)
        {
          file.close();
          request->send(*prtFS, "/www/listCMD.html", Template_HTTP_ContentType_Text, false, NULL);
        }
        else
        {
          request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "無法讀取HTML檔案!");
        }
      }
      else
      {
        File file = prtFS->open("/CommandTable.json", "r");
        if (!file)
          request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, "檔案開啟錯誤!");
        size_t docSize = ((file.size() / 1024) + 2) * 1024;
        JsonDocument doc;
        {
        JsonDocument filter;
        // 反序列化(解析)JSON
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        if (error)
        {
          String strError = "反序列化錯誤:" + String(error.c_str());
          _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, strError);
          request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, strError);
        }

        for (JsonPair keyValue : doc.as<JsonObject>())
        {
          if (keyValue.value()["Description"].isNull())
            continue;
          filter[keyValue.key()]["Description"] = true;
          filter[keyValue.key()]["Limit"] = true;
          filter[keyValue.key()]["Type"] = true;
          filter[keyValue.key()]["Default"] = true;
        }
        deserializeJson(doc, doc.as<String>(), DeserializationOption::Filter(filter));
        filter.clear();
        for (JsonPair keyValue : doc.as<JsonObject>())
        {
          if (keyValue.value()["Description"].isNull())
            continue;
          JsonDocument  obj;
          obj[keyValue.key()]["Value"] = 0;
          filter[keyValue.key()]["Value"]=0;
        }
        Template_JsonPTC->Read(&filter);
        for (JsonPair pair : filter.as<JsonObject>())
        {
          if ((doc[pair.key()]["Limit"].as<String>().indexOf('R') != -1 || doc[pair.key()]["Limit"].as<String>().indexOf('r') != -1))
            doc[pair.key()]["Default"] = "";
          doc[pair.key()]["Value"] = pair.value()["Value"];
        }
        }
            AsyncResponseStream *response = request->beginResponseStream(Template_HTTP_ContentType_Json);
            response->setCode(HTTP_CODE_OK);
            response->addHeader("Access-Control-Allow-Origin", "*");
            response->println(doc.as<String>());
            request->send(response);
      } });
  */
  /**
   * @brief Json控制協議
   *[{key1:{value:123}},{key2:[123,456]}]
   */
  /*
  Template_server.on(
      "/POSTJson", HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *ptdata, size_t len, size_t index, size_t total)
      {
        FS *prtFS = getFS();
        String rtStr = "";
        File file = prtFS->open("/CommandTable.json", "r");
        if (!file)
          request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, "檔案開啟錯誤!");
        size_t docSize = ((file.size() / 1024) + 2) * 1024;
        JsonDocument jsonDocument;
        DeserializationError error = deserializeJson(jsonDocument, ptdata, len);
        if (error)
        {
          rtStr = "反序列化錯誤:" + String(error.c_str());
          _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, rtStr);
          request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, rtStr);
        }
        else
        {
          Template_JsonPTC->setCMD(myCmdTable_Json);
          Template_JsonPTC->Write(&jsonDocument);
          Template_JsonPTC->Read(&jsonDocument);
          serializeJsonPretty(jsonDocument, rtStr);
          AsyncResponseStream *response = request->beginResponseStream(Template_HTTP_ContentType_Json);
          response->setCode(HTTP_CODE_OK);
          response->addHeader("Access-Control-Allow-Origin", "*");
          response->println(rtStr);
          request->send(response);
        }
      });
        //[ ]覆寫SPIFFS分區 //!無效

  Template_server.on(
      "/spiffsupload", HTTP_POST, [](AsyncWebServerRequest *request)
      {
      String senddata = ((Update.hasError()) ? "SPIFFS 数据更新失败～\n" : "SPIFFS 数据更新成功！\n");
      senddata += "<a href='/'>回到首页</a>";
      request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, senddata);
      vTaskDelay(pdMS_TO_TICKS(3000));
      ESP.restart(); },
      spiffsUpdate);
  Template_server.on(
      "/GETJson", HTTP_POST,
      [](AsyncWebServerRequest *request) {},
      NULL,
      [](AsyncWebServerRequest *request, uint8_t *ptdata, size_t len, size_t index, size_t total)
      {
        FS *prtFS = getFS();
        String rtStr = "";
        File file = prtFS->open("/CommandTable.json", "r");
        if (!file)
          request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, "檔案開啟錯誤!");
        size_t docSize = ((file.size() / 1024) + 2) * 1024;
        JsonDocument jsonDocument;
        DeserializationError error = deserializeJson(jsonDocument, ptdata, len);
        if (error)
        {
          String strError = "反序列化錯誤:" + String(error.c_str());
          _CONSOLE_PRINTLN(_PRINT_LEVEL_WARNING, strError);
          request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Json, strError);
        }
        else
        {
          Template_JsonPTC->Read(&jsonDocument);
          serializeJsonPretty(jsonDocument, rtStr);
        }
        AsyncResponseStream *response = request->beginResponseStream(Template_HTTP_ContentType_Json);
        response->setCode(HTTP_CODE_OK);
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->println(rtStr);
        request->send(response);
      });
  Template_server.on("/websocket-endpoint", HTTP_GET, [](AsyncWebServerRequest *request)
                     {
    // 处理WebSocket握手请求
    // 设置CORS头
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", "WebSocket Handshake Successful");
    response->addHeader("Access-Control-Allow-Origin", "*");
    request->send(response); });
*/
  /*
Template_server.on("/getVer", HTTP_GET, [](AsyncWebServerRequest *request)
         { request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, _T_E2JS(_VER)); });

Template_server.on("/getFirmware", HTTP_GET, getFirmware);
Template_server.on("/doUpdateFirmware", HTTP_GET, [](AsyncWebServerRequest *request)
{
int8_t state = updateFirmware();
switch (state) {
case 2:
request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, "固件已是最新版本!");
break;
case 1:
request->send(HTTP_CODE_OK, Template_HTTP_ContentType_Text, "更新完成!3秒後重啟...");
delay(3000);
// 重新启动ESP32
ESP.restart();
break;
case -1:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "无法下载CommandTable.json文件");
break;
case -2:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "无法打开CommandTable.json文件");
break;
case -3:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "无法初始化SPIFFS");
break;
case -4:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "固件更新失败");
break;
case -5:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "无法开始固件更新");
break;
case -6:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "固件下载失败");
break;
case -7:
request->send(HTTP_CODE_INTERNAL_SERVER_ERROR, Template_HTTP_ContentType_Text, "获取远程版本号失败");
break;
default:
request->send(HTTP_CODE_BAD_REQUEST, Template_HTTP_ContentType_Text, "ERROR!");
break;
} });
*/

  // 設定 404 路由
  Template_server.onNotFound([](AsyncWebServerRequest *request)
                             {
    // 如果是 OPTIONS 請求，添加 CORS 標頭並回應 200 OK
    if (request->method() == HTTP_OPTIONS) {
    AsyncResponseStream *response = request->beginResponseStream(Template_HTTP_ContentType_Text);
    response->setCode(HTTP_CODE_OK);
  response->addHeader("Access-Control-Allow-Origin", "*");
  response->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
  response->addHeader("Access-Control-Allow-Headers", "*");
request->send(response);
    } else {
      // 否則，正常處理 404 錯誤
      request->send(HTTP_CODE_NOT_FOUND,Template_HTTP_ContentType_Text, "查無此網頁!");
    } });
  // 註冊線上SPIFFS編輯服務 /edit
  FS *prtFS = 0;
  _T_E2JS(_FILE_CONFIG_FS).as<String>() == "SD" ? prtFS = &SD : prtFS = &SPIFFS;
  Template_server.addHandler(new SPIFFSEditor(*prtFS, "admin", "admin"));
  Template_server.begin();
}
/**
 * @brief 上傳設定檔
 *
 * @param request 伺服器物件指標
 * @param filename 檔案名稱
 * @param index 第幾梯檔案
 * @param data 檔案指標
 * @param len 本次檔案大小
 * @param len 是否最後一梯檔案
 */
void configUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  FS *ptrFS;
  _T_E2JS(_FILE_CONFIG_FS).as<String>() == "SD" ? ptrFS = &SD : ptrFS = &SPIFFS;
  if (!index)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "開始上傳：%s\n", filename.c_str());
    ptrFS->open("/" + filename, "w");
  }
  if (len)
  {
    request->_tempFile.write(data, len);
  }
  if (final)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "%s已上傳，檔案大小:%u\n", filename.c_str(), (index + len));
    request->_tempFile.close();
  }
}
/**
 * @brief 上傳韌體檔
 *
 * @param request 伺服器物件指標
 * @param filename 檔案名稱
 * @param index 第幾梯檔案
 * @param data 檔案指標
 * @param len 本次檔案大小
 * @param final 是否最後一梯檔案
 */
void firmwareUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  static size_t totalSize = 0;
  static size_t writtenSize = 0;
  if (filename.length() == 0)
  {
    // 如果 filename 为空，表示没有携带文件，不执行后续逻辑
    _CONSOLE_PRINTLN(_PRINT_LEVEL_INFO, "無夾帶檔案!");
    return;
  }
  if (!index)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "更新韌體：%s\n", filename.c_str());
    if (!Update.begin())
    {
      Update.printError(Serial);
    }
    totalSize = request->contentLength();
    writtenSize = 0;
  }
  if (len)
  {
    Update.write(data, len);
    writtenSize += len;
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "刷新進度: %.2f%%\r", (writtenSize / (float)totalSize) * 100);
  }
  if (final)
  {
    if (Update.end(true))
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "成功寫入%u位元組。\n重新啟動ESP32～\n", index + len);
    }
    else
    {
      Update.printError(Serial);
    }
  }
}
/**
 * @brief 覆寫整個SPIFFS區
 *
 * @param request
 * @param filename
 * @param index
 * @param data
 * @param len
 */
void spiffsUpdate(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
{
  if (!index)
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "更新 SPIFFS 数据：%s\n", filename.c_str());
    if (!Update.begin(UPDATE_SIZE_UNKNOWN))
    {
      if (_CONSOLE_PRINT_LEVEL >= _PRINT_LEVEL_ERROR)
      {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "覆寫SPIFFS錯誤!");
        Update.printError(Serial);
      }
    }
    /*
    if (SPIFFS.begin())
    {
        SPIFFS.format(); // 删除SPIFFS中的所有文件
    }
    else
    {
        _CONSOLE_PRINTLN(_PRINT_LEVEL_ERROR, "SPIFFS.begin() ERROR!");
        return false;
    }
    */
  }

  if (len)
  {
    Update.write(data, len);
  }

  if (final)
  {
    if (Update.end(true))
    {
      _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "成功写入%u字节。\n重新启动 ESP32～\n", index + len);
    }
    else
    {
      Update.printError(Serial);
    }
  }
}
/**
 * @brief 讀取自身固件發送
 *
 * @param request
 */
void getFirmware(AsyncWebServerRequest *request)
{
  uint32_t firmwareSize = ESP.getSketchSize();
  uint32_t firmwareAddress = ESP.getSketchSize() - firmwareSize;

  static uint8_t buffer[1024];
  size_t bytesRead = 0;
  size_t totalBytesSent = 0;

  // 循环读取固件数据并发送给客户端
  do
  {
    uint32_t address = firmwareAddress + bytesRead;
    size_t remainingBytes = firmwareSize - bytesRead;
    size_t chunkSize = min(sizeof(buffer), remainingBytes);
    spi_flash_read(address, reinterpret_cast<uint32_t *>(buffer), chunkSize);

    totalBytesSent += chunkSize;

    // 将缓冲区的数据发送给客户端
    request->send_P(200, "application/octet-stream", buffer, chunkSize);

    bytesRead += chunkSize;
  } while (bytesRead < firmwareSize);
  if (request->hasParam("name"))
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已向 %s 发送固件数据：%u 字节\n", request->getParam("name")->value().c_str(), totalBytesSent);
  }
  else
  {
    _CONSOLE_PRINTF(_PRINT_LEVEL_INFO, "已发送固件数据：%u 字节\n", totalBytesSent);
  }
}
