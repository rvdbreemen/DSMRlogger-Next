/* 
***************************************************************************  
**  Program  : restAPI, part of DSMRlogger-Next
**  Version  : v2.1.0-rc0
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

// ******* Global Vars *******
uint32_t  antiWearTimer = 0;

char fieldName[40] = "";

char fieldsArray[50][35] = {{0}}; // to lookup fields 
int  fieldsElements      = 0;

int  actualElements = 20;
char actualArray[][35] = { "timestamp"
                          ,"energy_delivered_tariff1","energy_delivered_tariff2"
                          ,"energy_returned_tariff1","energy_returned_tariff2"
                          ,"power_delivered","power_returned"
                          ,"voltage_l1","voltage_l2","voltage_l3"
                          ,"current_l1","current_l2","current_l3"
                          ,"power_delivered_l1","power_delivered_l2","power_delivered_l3"
                          ,"power_returned_l1","power_returned_l2","power_returned_l3"
                          ,"gas_delivered"
                          ,"\0"};
int  infoElements = 7;
char infoArray[][35]   = { "identification","p1_version","equipment_id","electricity_tariff"
                          ,"gas_device_type","gas_equipment_id"
                          , "\0" };
  
bool onlyIfPresent  = false;

//=======================================================================
void processAPI() 
{
  char fName[40] = "";
  char URI[50]   = "";
  String words[10];

  strncpy( URI, httpServer.uri().c_str(), sizeof(URI) );

  if (httpServer.method() == HTTP_GET)
        DebugTf("from[%s] URI[%s] method[GET] \r\n"
                                  , httpServer.client().remoteIP().toString().c_str()
                                        , URI); 
  else  DebugTf("from[%s] URI[%s] method[PUT] \r\n" 
                                  , httpServer.client().remoteIP().toString().c_str()
                                        , URI); 

#ifdef USE_SYSLOGGER
  if (ESP.getFreeHeap() < 5000) // to prevent firmware from crashing!
#else
  if (ESP.getFreeHeap() < 7000) // to prevent firmware from crashing!
#endif
  {
    DebugTf("==> Bailout due to low heap (%d bytes))\r\n", ESP.getFreeHeap() );
    writeToSysLog("from[%s][%s] Bailout low heap (%d bytes)"
                                  , httpServer.client().remoteIP().toString().c_str()
                                  , URI
                                  , ESP.getFreeHeap() );
    httpServer.send(500, "text/plain", "500: internal server error (low heap)\r\n"); 
    return;
  }

  int8_t wc = splitString(URI, '/', words, 10);
  
  if (Verbose2) 
  {
    DebugT(">>");
    for (int w=0; w<wc; w++)
    {
      Debugf("word[%d] => [%s], ", w, words[w].c_str());
    }
    Debugln(" ");
  }

  if (words[1] != "api")
  {
    sendApiNotFound(URI);
    return;
  }

  if (words[2] == "v0" && words[3] == "sm" && words[4] == "actual")
  {
    //--- depreciated api. left here for backward compatibility
    onlyIfPresent = true;
    copyToFieldsArray(actualArray, actualElements);
    sendJsonV0Fields();
    return;
  }
  if (words[2] != "v1")
  {
    sendApiNotFound(URI);
    return;
  }

  if (words[3] == "dev")
  {
    handleDevApi(URI, words[4].c_str(), words[5].c_str(), words[6].c_str());
  }
  else if (words[3] == "hist")
  {
    handleHistApi(URI, words[4].c_str(), words[5].c_str(), words[6].c_str());
  }
  else if (words[3] == "sm")
  {
    handleSmApi(URI, words[4].c_str(), words[5].c_str(), words[6].c_str());
  }
  else sendApiNotFound(URI);
  
} // processAPI()


//====================================================
void handleDevApi(const char *URI, const char *word4, const char *word5, const char *word6)
{
  //DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (strcmp(word4, "info") == 0)
  {
    sendDeviceInfo();
  }
  else if (strcmp(word4, "time") == 0)
  {
    sendDeviceTime();
  }
  else if (strcmp(word4, "settings") == 0)
  {
    if (httpServer.method() == HTTP_PUT || httpServer.method() == HTTP_POST)
    {
      //------------------------------------------------------------ 
      // json string: {"name":"settingInterval","value":9}  
      // json string: {"name":"settingTelegramInterval","value":123.45}  
      // json string: {"name":"settingTelegramInterval","value":"abc"}  
      //------------------------------------------------------------ 
      // so, why not use ArduinoJSON library?
      // I say: try it yourself ;-) It won't be easy
      String wOut[5];
      String wPair[5];
      String jsonIn  = httpServer.arg(0).c_str();
      char field[25] = "";
      char newValue[101]="";
      jsonIn.replace("{", "");
      jsonIn.replace("}", "");
      jsonIn.replace("\"", "");
      int8_t wp = splitString(jsonIn.c_str(), ',',  wPair, 5) ;
      for (int i=0; i<wp; i++)
      {
        //DebugTf("[%d] -> pair[%s]\r\n", i, wPair[i].c_str());
        int8_t wc = splitString(wPair[i].c_str(), ':',  wOut, 5) ;
        //DebugTf("==> [%s] -> field[%s]->val[%s]\r\n", wPair[i].c_str(), wOut[0].c_str(), wOut[1].c_str());
        if (wOut[0].equalsIgnoreCase("name"))  strlcpy(field, wOut[1].c_str(), sizeof(field));
        if (wOut[0].equalsIgnoreCase("value")) strlcpy(newValue, wOut[1].c_str(), sizeof(newValue));
      }
      //DebugTf("--> field[%s] => newValue[%s]\r\n", field, newValue);
      updateSetting(field, newValue);
      httpServer.send(200, "application/json", httpServer.arg(0));
      writeToSysLog("DSMReditor: Field[%s] changed to [%s]", field, newValue);
    }
    else
    {
      sendDeviceSettings();
    }
  }
  else if (strcmp(word4, "debug") == 0)
  {
    sendDeviceDebug(URI, word5);
  }
  else sendApiNotFound(URI);
  
} // handleDevApi()


//====================================================
void handleHistApi(const char *URI, const char *word4, const char *word5, const char *word6)
{
  int8_t  fileType     = 0;
  char    fileName[20] = "";
  
  //DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (   strcmp(word4, "hours") == 0 )
  {
    fileType = HOURS;
    strlcpy(fileName, HOURS_FILE, sizeof(fileName));
  }
  else if (strcmp(word4, "days") == 0 )
  {
    fileType = DAYS;
    strlcpy(fileName, DAYS_FILE, sizeof(fileName));
  }
  else if (strcmp(word4, "months") == 0)
  {
    fileType = MONTHS;
    if (httpServer.method() == HTTP_PUT || httpServer.method() == HTTP_POST)
    {
      //------------------------------------------------------------ 
      // json string: {"recid":"29013023"
      //               ,"edt1":2601.146,"edt2":"9535.555"
      //               ,"ert1":378.074,"ert2":208.746
      //               ,"gdt":3314.404}
      //------------------------------------------------------------ 
      char      record[DATA_RECLEN + 1] = "";
      uint16_t  recSlot;

      String jsonIn  = httpServer.arg(0).c_str();
      DebugTln(jsonIn);
      
      recSlot = buildDataRecordFromJson(record, jsonIn);
      
      //--- update MONTHS
      writeDataToFile(MONTHS_FILE, record, recSlot, MONTHS);
      //--- send OK response --
      httpServer.send(200, "application/json", httpServer.arg(0));
      
      return;
    }
    else 
    {
      strlcpy(fileName, MONTHS_FILE, sizeof(fileName));
    }
  }
  else 
  {
    sendApiNotFound(URI);
    return;
  }
  if (strcmp(word5, "desc") == 0)
        sendJsonHist(fileType, fileName, actTimestamp, true);
  else  sendJsonHist(fileType, fileName, actTimestamp, false);

} // handleHistApi()


//====================================================
void handleSmApi(const char *URI, const char *word4, const char *word5, const char *word6)
{
  char    tlgrm[1200] = "";
  uint8_t p=0;  
  bool    stopParsingTelegram = false;

  //DebugTf("word4[%s], word5[%s], word6[%s]\r\n", word4, word5, word6);
  if (strcasecmp(word4, "info") == 0)
  {
    //sendSmInfo();
    onlyIfPresent = false;
    copyToFieldsArray(infoArray, infoElements);
    sendJsonFields(word4);
  }
  else if (strcasecmp(word4, "actual") == 0)
  {
    //sendSmActual();
    onlyIfPresent = true;
    copyToFieldsArray(actualArray, actualElements);
    sendJsonFields(word4);
  }
  else if (strcasecmp(word4, "fields") == 0)
  {
    fieldsElements = 0;
    onlyIfPresent = false;

    if (strlen(word5) > 0)
    {
       memset(fieldsArray,0,sizeof(fieldsArray));
       strlcpy(fieldsArray[0], "timestamp",34);
       strlcpy(fieldsArray[1],  word5, 35);
       fieldsElements = 2;
    }
    sendJsonFields(word4);
  }
  else if (strcasecmp(word4, "telegram") == 0)
  {
    showRaw = true;
    slimmeMeter.enable(true);
    Serial.setTimeout(5000);  // 5 seconds must be enough ..
    memset(tlgrm, 0, sizeof(tlgrm));
    int l = 0;
    // The terminator character is discarded from the serial buffer.
    l = Serial.readBytesUntil('/', tlgrm, sizeof(tlgrm));
    // now read from '/' to '!'
    // The terminator character is discarded from the serial buffer.
    l = Serial.readBytesUntil('!', tlgrm, sizeof(tlgrm));
    Serial.setTimeout(1000);  // seems to be the default ..
    DebugTf("read [%d] bytes\r\n", l);
    if (l == 0) 
    {
      httpServer.send(200, "application/plain", "no telegram received");
      showRaw = false;
      return;
    }

    tlgrm[l++] = '!';
#if !defined( USE_PRE40_PROTOCOL )
    // next 6 bytes are "<CRC>\r\n"
    for (int i=0; ( i<6 && (i<(sizeof(tlgrm)-7)) ); i++)
    {
      tlgrm[l++] = (char)Serial.read();
    }
#else
    tlgrm[l++]    = '\r';
    tlgrm[l++]    = '\n';
#endif
    tlgrm[(l +1)] = '\0';
    // shift telegram 1 char to the right (make room at pos [0] for '/')
    for (int i=strlen(tlgrm); i>=0; i--) { tlgrm[i+1] = tlgrm[i]; yield(); }
    tlgrm[0] = '/'; 
    showRaw = false;
    if (Verbose1) Debugf("Telegram (%d chars):\r\n/%s", strlen(tlgrm), tlgrm);
    httpServer.send(200, "application/plain", tlgrm);

  }
  else sendApiNotFound(URI);
  
} // handleSmApi()


//=======================================================================
void sendDeviceInfo() 
{
  char compileOptions[200] = "";

#ifdef USE_REQUEST_PIN
    strConcat(compileOptions, sizeof(compileOptions), "[USE_REQUEST_PIN]");
#endif
#if defined( USE_PRE40_PROTOCOL )
    strConcat(compileOptions, sizeof(compileOptions), "[USE_PRE40_PROTOCOL]");
#elif defined( USE_BELGIUM_PROTOCOL )
    strConcat(compileOptions, sizeof(compileOptions), "[USE_BELGIUM_PROTOCOL]");
#else
    strConcat(compileOptions, sizeof(compileOptions), "[USE_DUTCH_PROTOCOL]");
#endif
#ifdef USE_UPDATE_SERVER
    strConcat(compileOptions, sizeof(compileOptions), "[USE_UPDATE_SERVER]");
#endif
#ifdef USE_MQTT
    strConcat(compileOptions, sizeof(compileOptions), "[USE_MQTT]");
#endif
#ifdef USE_MINDERGAS
    strConcat(compileOptions, sizeof(compileOptions), "[USE_MINDERGAS]");
#endif
#ifdef USE_INFLUXDB
    strConcat(compileOptions, sizeof(compileOptions), "[USE_INFLUXDB]");
#endif
#ifdef USE_SYSLOGGER
    strConcat(compileOptions, sizeof(compileOptions), "[USE_SYSLOGGER]");
#endif
#ifdef USE_NTP_TIME
    strConcat(compileOptions, sizeof(compileOptions), "[USE_NTP_TIME]");
#endif

  sendStartJsonObj("devinfo");

  sendNestedJsonObj("author", "Willem Aandewiel (www.aandewiel.nl)");
  sendNestedJsonObj("fwversion", _FW_VERSION);

  snprintf(cMsg, sizeof(cMsg), "%s %s", __DATE__, __TIME__);
  sendNestedJsonObj("compiled", cMsg);
  sendNestedJsonObj("hostname", settingHostname);
  sendNestedJsonObj("ipaddress", WiFi.localIP().toString().c_str());
  sendNestedJsonObj("macaddress", WiFi.macAddress().c_str());
  sendNestedJsonObj("indexfile", settingIndexPage);
  sendNestedJsonObj("freeheap", ESP.getFreeHeap(), "bytes");
  sendNestedJsonObj("maxfreeblock", ESP.getMaxFreeBlockSize(), "bytes");
  sendNestedJsonObj("chipid", String( ESP.getChipId(), HEX ).c_str());
  sendNestedJsonObj("coreversion", String( ESP.getCoreVersion() ).c_str() );
  sendNestedJsonObj("sdkversion", String( ESP.getSdkVersion() ).c_str());
  sendNestedJsonObj("cpufreq", ESP.getCpuFreqMHz(), "MHz");
  sendNestedJsonObj("sketchsize", formatFloat( (ESP.getSketchSize() / 1024.0), 3), "kB");
  sendNestedJsonObj("freesketchspace", formatFloat( (ESP.getFreeSketchSpace() / 1024.0), 3), "kB");

  if ((ESP.getFlashChipId() & 0x000000ff) == 0x85) 
        snprintf(cMsg, sizeof(cMsg), "%08X (PUYA)", ESP.getFlashChipId());
  else  snprintf(cMsg, sizeof(cMsg), "%08X", ESP.getFlashChipId());
  sendNestedJsonObj("flashchipid", cMsg);  // flashChipId
  sendNestedJsonObj("flashchipsize", formatFloat((ESP.getFlashChipSize() / 1024.0 / 1024.0), 3), "MB");
  sendNestedJsonObj("flashchiprealsize", formatFloat((ESP.getFlashChipRealSize() / 1024.0 / 1024.0), 3), "MB");

  SPIFFS.info(SPIFFSinfo);
  sendNestedJsonObj("spiffssize", formatFloat( (SPIFFSinfo.totalBytes / (1024.0 * 1024.0)), 0), "MB");

  sendNestedJsonObj("flashchipspeed", formatFloat((ESP.getFlashChipSpeed() / 1000.0 / 1000.0), 0), "MHz");

  FlashMode_t ideMode = ESP.getFlashChipMode();
  sendNestedJsonObj("flashchipmode", flashMode[ideMode]); 
#if defined(ARDUINO_ESP8266_NODEMCU)
  const char* strBoardtype = "ESP8266_NODEMCU";
#elif defined(ARDUINO_ESP8266_GENERIC)
  const char* strBoardtype = "ESP8266_GENERIC";
#elif defined(ESP8266_ESP01)
  const char* strBoardtype = "ESP8266_ESP01";
#elif defined(ESP8266_ESP12)
  const char* strBoardtype = "ESP8266_ESP12";
#else
  const char* strBoardtype = "Unknown board";
#endif
  sendNestedJsonObj("boardtype", strBoardtype);
  sendNestedJsonObj("compileoptions", compileOptions);
  sendNestedJsonObj("ssid", WiFi.SSID().c_str());
#ifdef SHOW_PASSWRDS
  sendNestedJsonObj("pskkey", WiFi.psk());   
#endif
  sendNestedJsonObj("wifirssi", WiFi.RSSI());
  sendNestedJsonObj("uptime", upTime());
  sendNestedJsonObj("oled_type",        (int)settingOledType);
  sendNestedJsonObj("oled_flip_screen", (int)settingOledFlip);
  sendNestedJsonObj("smhasfaseinfo",    (int)settingSmHasFaseInfo);
  sendNestedJsonObj("telegraminterval", (int)settingTelegramInterval);
  sendNestedJsonObj("telegramcount",    (int)telegramCount);
  sendNestedJsonObj("telegramerrors",   (int)telegramErrors);

#ifdef USE_MQTT
  snprintf(cMsg, sizeof(cMsg), "%s:%04d", settingMQTTbroker, settingMQTTbrokerPort);
  sendNestedJsonObj("mqttbroker", cMsg);
  sendNestedJsonObj("mqttinterval", settingMQTTinterval);
  if (mqttIsConnected)
        sendNestedJsonObj("mqttbroker_connected", "yes");
  else  sendNestedJsonObj("mqttbroker_connected", "no");
#endif
#ifdef USE_MINDERGAS
  snprintf(cMsg, sizeof(cMsg), "%s:%d", timeLastResponse, intStatuscodeMindergas);
  sendNestedJsonObj("mindergas_response",     txtResponseMindergas);
  sendNestedJsonObj("mindergas_status",       cMsg);
#endif

#ifdef USE_INFLUXDB
  sendNestedJsonObj("influxdb_hostname",           settingInfluxDBhostname);
  sendNestedJsonObj("influxdb_port",              (int)settingInfluxDBport);
  sendNestedJsonObj("influxdb_databasename",      settingInfluxDBdatabasename);
#endif


  sendNestedJsonObj("reboots", (int)nrReboots);
  sendNestedJsonObj("lastreset", lastReset);

  httpServer.sendContent("\r\n]}\r\n");

} // sendDeviceInfo()


//=======================================================================
void sendDeviceTime() 
{
  sendStartJsonObj("devtime");
  sendNestedJsonObj("timestamp", actTimestamp); 
  sendNestedJsonObj("time", buildDateTimeString(actTimestamp, sizeof(actTimestamp)).c_str()); 
  sendNestedJsonObj("epoch", (int)now());

  sendEndJsonObj();

} // sendDeviceTime()


//=======================================================================
void sendDeviceSettings() 
{
  DebugTln("sending device settings ...\r");

  sendStartJsonObj("settings");
  
  sendJsonSettingObj("hostname",          settingHostname,        "s", sizeof(settingHostname) -1);
  sendJsonSettingObj("ed_tariff1",        settingEDT1,            "f", 0, 10,  5);
  sendJsonSettingObj("ed_tariff2",        settingEDT2,            "f", 0, 10,  5);
  sendJsonSettingObj("er_tariff1",        settingERT1,            "f", 0, 10,  5);
  sendJsonSettingObj("er_tariff2",        settingERT2,            "f", 0, 10,  5);
  sendJsonSettingObj("gd_tariff",         settingGDT,             "f", 0, 10,  5);
  sendJsonSettingObj("electr_netw_costs", settingENBK,            "f", 0, 100, 2);
  sendJsonSettingObj("gas_netw_costs",    settingGNBK,            "f", 0, 100, 2);
  sendJsonSettingObj("sm_has_fase_info",  settingSmHasFaseInfo,   "i", 0, 1);
  sendJsonSettingObj("tlgrm_interval",    settingTelegramInterval,"i", 2, 60);
  sendJsonSettingObj("oled_type",         settingOledType,        "i", 0, 2);
  sendJsonSettingObj("oled_screen_time",  settingOledSleep,       "i", 1, 300);
  sendJsonSettingObj("oled_flip_screen",  settingOledFlip,        "i", 0, 1);
  sendJsonSettingObj("index_page",        settingIndexPage,       "s", sizeof(settingIndexPage) -1);
  sendJsonSettingObj("mqtt_broker",       settingMQTTbroker,      "s", sizeof(settingMQTTbroker) -1);
  sendJsonSettingObj("mqtt_broker_port",  settingMQTTbrokerPort,  "i", 1, 65535);
  sendJsonSettingObj("mqtt_user",         settingMQTTuser,        "s", sizeof(settingMQTTuser) -1);
  sendJsonSettingObj("mqtt_passwd",       settingMQTTpasswd,      "s", sizeof(settingMQTTpasswd) -1);
  sendJsonSettingObj("mqtt_toptopic",     settingMQTTtopTopic,    "s", sizeof(settingMQTTtopTopic) -1);
  sendJsonSettingObj("mqtt_interval",     settingMQTTinterval,    "i", 0, 600);
#ifdef USE_MINDERGAS
  sendJsonSettingObj("mindergastoken",  settingMindergasToken,    "s", sizeof(settingMindergasToken) -1);
#endif
#ifdef USE_INFLUXDB
  sendJsonSettingObj("influxdb_hostname",           settingInfluxDBhostname,       "s", sizeof(settingInfluxDBhostname)-1);
  sendJsonSettingObj("influxdb_port",              (int)settingInfluxDBport,      "i", 1, 65535);
  sendJsonSettingObj("influxdb_databasename",      settingInfluxDBdatabasename,   "s", sizeof(settingInfluxDBdatabasename)-1);
#endif
  sendEndJsonObj();

} // sendDeviceSettings()


//=======================================================================
void sendDeviceDebug(const char *URI, String tail) 
{
#ifdef USE_SYSLOGGER
  String lLine = "";
  int lineNr = 0;
  int tailLines = tail.toInt();

  DebugTf("list [%d] debug lines\r\n", tailLines);
  sysLog.status();
  sysLog.setDebugLvl(0);
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  if (tailLines > 0)
        sysLog.startReading((tailLines * -1));  
  else  sysLog.startReading(0, 0);  
  while( (lLine = sysLog.readNextLine()) && !(lLine == "EOF")) 
  {
    lineNr++;
    snprintf(cMsg, sizeof(cMsg), "%s\r\n", lLine.c_str());
    httpServer.sendContent(cMsg);

  }
  sysLog.setDebugLvl(1);

#else
  sendApiNotFound(URI);
#endif

} // sendDeviceDebug()

//=======================================================================
struct buildJsonApiV0SmActual
{
    bool  skip = false;
    
    template<typename Item>
    void apply(Item &i) {
      skip = false;
      String Name = Item::name;
      //-- for dsmr30 -----------------------------------------------
#if defined( USE_PRE40_PROTOCOL )
      if (Name.indexOf("gas_delivered2") == 0) Name = "gas_delivered";
#endif
      if (!isInFieldsArray(Name.c_str(), fieldsElements))
      {
        skip = true;
      }
      if (!skip)
      {
        if (i.present()) 
        {
          //String Unit = Item::unit();
          sendNestedJsonV0Obj(Name.c_str(), typecastValue(i.val()));
        }
      }
  }

};  // buildJsonApiV0SmActual()


//=======================================================================
void sendJsonV0Fields() 
{
  objSprtr[0]    = '\0';
  
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send(200, "application/json", "{\r\n");
  DSMRdata.applyEach(buildJsonApiV0SmActual());
  httpServer.sendContent("\r\n}\r\n");

} // sendJsonV0Fields()


//=======================================================================
struct buildJsonApi 
{
    bool  skip = false;
    
    template<typename Item>
    void apply(Item &i) {
      skip = false;
      String Name = Item::name;
      //-- for dsmr30 -----------------------------------------------
#if defined( USE_PRE40_PROTOCOL )
      if (Name.indexOf("gas_delivered2") == 0) Name = "gas_delivered";
#endif
      if (!isInFieldsArray(Name.c_str(), fieldsElements))
      {
        skip = true;
      }
      if (!skip)
      {
        if (i.present()) 
        {
          String Unit = Item::unit();
        
          if (Unit.length() > 0)
          {
            sendNestedJsonObj(Name.c_str(), typecastValue(i.val()), Unit.c_str());
          }
          else 
          {
            sendNestedJsonObj(Name.c_str(), typecastValue(i.val()));
          }
        }
        else if (!onlyIfPresent)
        {
          sendNestedJsonObj(Name.c_str(), "-");
        }
      }
  }

};  // buildJsonApi()


//=======================================================================
void sendJsonFields(const char *Name) 
{
  sendStartJsonObj(Name);
  DSMRdata.applyEach(buildJsonApi());
  sendEndJsonObj();

} // sendJsonFields()


//=======================================================================
void sendJsonHist(int8_t fileType, const char *fileName, const char *timeStamp, bool desc) 
{
  uint8_t startSlot, nrSlots, recNr  = 0;
  char    typeApi[10];


  if (DUE(antiWearTimer))
  {
    writeDataToFiles();
    writeLastStatus();
  }
    
  switch(fileType) {
    case HOURS:   startSlot       = timestampToHourSlot(timeStamp, strlen(timeStamp));
                  nrSlots         = _NO_HOUR_SLOTS_;
                  strlcpy(typeApi, "hours", 9);
                  break;
    case DAYS:    startSlot       = timestampToDaySlot(timeStamp, strlen(timeStamp));
                  nrSlots         = _NO_DAY_SLOTS_;
                  strlcpy(typeApi,  "days", 9);
                  break;
    case MONTHS:  startSlot       = timestampToMonthSlot(timeStamp, strlen(timeStamp));
                  nrSlots         = _NO_MONTH_SLOTS_;
                  strlcpy(typeApi, "months", 9);
                  break;
  }

  sendStartJsonObj(typeApi);

  if (desc)
        startSlot += nrSlots +1; // <==== voorbij actuele slot!
  else  startSlot += nrSlots;    // <==== start met actuele slot!

  DebugTf("sendJsonHist startSlot[%02d]\r\n", (startSlot % nrSlots));

  for (uint8_t s = 0; s < nrSlots; s++)
  {
    if (desc)
          readOneSlot(fileType, fileName, s, (s +startSlot), true, "hist") ;
    else  readOneSlot(fileType, fileName, s, (startSlot -s), true, "hist") ;
  }
  sendEndJsonObj();
  
} // sendJsonHist()


bool isInFieldsArray(const char* lookUp, int elemts)
{
  if (elemts == 0) return true;

  for (int i=0; i<elemts; i++)
  {
    //if (Verbose2) DebugTf("[%2d] Looking for [%s] in array[%s]\r\n", i, lookUp, fieldsArray[i]); 
    if (strcmp(lookUp, fieldsArray[i]) == 0) return true;
  }
  return false;
  
} // isInFieldsArray()


void copyToFieldsArray(const char inArray[][35], int elemts)
{
  int i = 0;
  memset(fieldsArray,0,sizeof(fieldsArray));
  //if (Verbose2) DebugTln("start copying ....");
  
  for ( i=0; i<elemts; i++)
  {
    strncpy(fieldsArray[i], inArray[i], 34);
    //if (Verbose1) DebugTf("[%2d] => inArray[%s] fieldsArray[%s]\r\n", i, inArray[i], fieldsArray[i]); 

  }
  fieldsElements = i;
  
} // copyToFieldsArray()


bool listFieldsArray(char inArray[][35])
{
  int i = 0;

  for ( i=0; strlen(inArray[i]) == 0; i++)
  {
    DebugTf("[%2d] => inArray[%s]\r\n", i, inArray[i]); 
  }
  
} // listFieldsArray()


//====================================================
void sendApiNotFound(const char *URI)
{
  httpServer.sendHeader("Access-Control-Allow-Origin", "*");
  httpServer.setContentLength(CONTENT_LENGTH_UNKNOWN);
  httpServer.send ( 404, "text/html", "<!DOCTYPE HTML><html><head>");

  strlcpy(cMsg, "<style>body { background-color: lightgray; font-size: 15pt;}", sizeof(cMsg));
  strConcat(cMsg, sizeof(cMsg), "</style></head><body>");
  httpServer.sendContent(cMsg);

  strlcpy(cMsg, "<h1>DSMR-logger</h1><b1>",   sizeof(cMsg));
  httpServer.sendContent(cMsg);

  strlcpy(cMsg, "<br>[<b>",   sizeof(cMsg));
  strConcat(cMsg, sizeof(cMsg), URI);
  strConcat(cMsg, sizeof(cMsg), "</b>] is not a valid ");
  httpServer.sendContent(cMsg);
  
  strlcpy(cMsg, "<a href=",   sizeof(cMsg));
  strConcat(cMsg, sizeof(cMsg), "\"https://mrwheel-docs.gitbook.io/dsmrloggerapi/beschrijving-restapis\">");
  strConcat(cMsg, sizeof(cMsg), "restAPI</a> call.");
  httpServer.sendContent(cMsg);
  
  strlcpy(cMsg, "</body></html>\r\n", sizeof(cMsg));
  httpServer.sendContent(cMsg);

  writeToSysLog("[%s] is not a valid restAPI call!!", URI);
  
} // sendApiNotFound()



/***************************************************************************
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit
* persons to whom the Software is furnished to do so, subject to the
* following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
* OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
* THE USE OR OTHER DEALINGS IN THE SOFTWARE.
* 
***************************************************************************/
