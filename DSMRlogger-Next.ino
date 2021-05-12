/*
***************************************************************************  
**  Program  : DSMRlogger-Next (the Next version of DSMR Logger)
*/
#include "version.h"
#define _FW_VERSION _VERSION
/*
**  The Next development:  
**          DSMRlogger-Next - Copyright (c) 2020 Robert van den Breemen
**
**  To go beyond the original with new features and fixing some issues.  
**
**  Based on the original:
**          DSMRLoggerAPI - Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*
* DSMRlogger-Next instructions:
* -   Install InfluxDBclient library    (https://github.com/tobiasschuerg/InfluxDB-Client-for-Arduino)

*
***************************************************************************      
  Arduino-IDE settings for DSMR-logger Version 4 (ESP-12):

    - Board: "Generic ESP8266 Module"
    - Builtin Led: "2"  // GPIO02 for Wemos and ESP-12
    - Flash mode: "DOUT" | "DIO"    // changes only after power-off and on again!
    - Flash size: "4MB (FS: 2MB OAT:~1019KB)"  << LET OP! 2MB SPIFFS
    - DebugT port: "Disabled"
    - DebugT Level: "None"
    - IwIP Variant: "v2 Lower Memory"
    - Reset Method: "none"   // but will depend on the programmer!
    - Crystal Frequency: "26 MHz" 
    - VTables: "Flash"
    - Flash Frequency: "40MHz"
    - CPU Frequency: "80 MHz" (or 160MHz)
    - Upload Speed: "115200"                                                                                                                                                                                                                                                 
    - Erase Flash: "Only Sketch"
    - Port: <select correct port>
*/

//================================================================================
// There is a problem in the WebServer library!!!
//
// https://github.com/espressif/arduino-esp32/issues/3652#issuecomment-617058198
//
// -- changed WebServer.cpp in
// ~/Library/Arduino15/packages/esp32/hardware/esp32/1.0.4/libraries/WebServer/src
//
//--------------------------------------------------------------------------
//316  if (_currentClient.available()) {
//       if (_parseRequest(_currentClient)) {
//         // because HTTP_MAX_SEND_WAIT is expressed in milliseconds,
//         // it must be divided by 1000
//         _currentClient.setTimeout(HTTP_MAX_SEND_WAIT / 1000);
//         _contentLength = CONTENT_LENGTH_NOT_SET;
//         _handleRequest();
//
//         /* this comment out for chrome/maxthon browsers
//         if (_currentClient.connected()) {
//           _currentStatus = HC_WAIT_CLOSE;
//           _statusChange = millis();
//           keepCurrentClient = true;
//         } 
//         */
//       }
//     } else { // !_currentClient.available()
//     ......
//--------------------------------------------------------------------------
// original file in ../srcORG/

/*
**  You can find more info in the following links (all in Dutch): 
**   https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/
**   https://mrwheel-docs.gitbook.io/dsmrloggerapi/
**   https://mrwheel.github.io/DSMRloggerWS/
*/
/******************** compiler options  ********************************************/
#define USE_REQUEST_PIN           // define if it's a esp8266 with GPIO 12 connected to SM DTR pin
#define USE_UPDATE_SERVER         // define if there is enough memory and updateServer to be used
//  #define USE_BELGIUM_PROTOCOL      // define if Slimme Meter is a Belgium Smart Meter
//  #define USE_PRE40_PROTOCOL        // define if Slimme Meter is pre DSMR 4.0 (2.2 .. 3.0)
//  #define USE_NTP_TIME              // define to generate Timestamp from NTP (Only Winter Time for now)
#define HAS_NO_SLIMMEMETER        // define for testing only!
#define USE_INFLUXDB              // define if you want to use Influxdb (configure through webinterface)
#define USE_MQTT                  // define if you want to use MQTT (configure through webinterface)
#define USE_MINDERGAS             // define if you want to update mindergas (configure through webinterface)
//  #define USE_SYSLOGGER               // define if you want to use the sysLog library for debugging
//  #define SHOW_PASSWRDS               // well .. show the PSK key and MQTT password, what else?
//  #define USE_WEMOSLOLIN32            //define if it is a WEMOS LOLIN32 with OLED (requires different IO pins and I2Cadres)
/******************** don't change anything below this comment **********************/

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

#include "DSMRlogger-Next.h"

struct showValues {
  template<typename Item>
  void apply(Item &i) {
    TelnetStream.print("showValues: ");
    if (i.present()) 
    {
      TelnetStream.print(Item::name);
      TelnetStream.print(F(": "));
      TelnetStream.print(i.val());
      TelnetStream.print(Item::unit());
    } else 
    {
      TelnetStream.print(F("<no value>"));
    }
    TelnetStream.println();
  }
};


//===========================================================================================
void displayStatus() 
{
  if (settingOledType > 0)
  {
    switch(msgMode) { 
      case 1:   snprintf(cMsg, sizeof(cMsg), "Up:%-15.15s", upTime().c_str());
                break;
      case 2:   snprintf(cMsg, sizeof(cMsg), "WiFi RSSI:%4d dBm", WiFi.RSSI());
                break;
      case 3:   snprintf(cMsg, sizeof(cMsg), "Heap:%7d Bytes", ESP.getFreeHeap());
                break;
      case 4:   if (WiFi.status() != WL_CONNECTED)
                      snprintf(cMsg, sizeof(cMsg), "**** NO  WIFI ****");
                else  snprintf(cMsg, sizeof(cMsg), "IP %s", WiFi.localIP().toString().c_str());
                break;
      default:  snprintf(cMsg, sizeof(cMsg), "Telgrms:%6d/%3d", telegramCount, telegramErrors);
                break;
    }

    oled_Print_Msg(3, cMsg, 0);
    msgMode= (msgMode+1) % 5; //modular 5 = number of message displayed (hence it cycles thru the messages
  }  
} // displayStatus()


#ifdef USE_SYSLOGGER
//===========================================================================================
void openSysLog(bool empty)
{
  if (sysLog.begin(500, 100, empty))  // 500 lines use existing sysLog file
  {   
    DebugTln("Succes opening sysLog!");
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
      oled_Print_Msg(3, "Syslog OK!", 500);
    }
  }
  else
  {
    DebugTln("Error opening sysLog!");
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
      oled_Print_Msg(3, "Error Syslog", 1500);
    }
  }

  sysLog.setDebugLvl(1);
  sysLog.setOutput(&TelnetStream);
  sysLog.status();
  sysLog.write("\r\n");
  for (int q=0;q<3;q++)
  {
    sysLog.write("******************************************************************************************************");
  }
  writeToSysLog("Last Reset Reason [%s]", getResetReason().c_str());
  writeToSysLog("actTimestamp[%s], nrReboots[%u], Errors[%u]", actTimestamp
                                                             , nrReboots
                                                             , slotErrors);

  sysLog.write(" ");

} // openSysLog()
#endif

//===========================================================================================
void setup() 
{

  //================ Serial Debug ================================

  DEBUG_PORT.begin(115200, SERIAL_8N1);                   //DEBUG
  Debugf("\n\nBooting [%s]", _FW_VERSION);
  for(int i=10; i>0; i--)
  {
    delay(300);
    Debugf(".");
  }
  Debugf("\r\n");
  DebugFlush();  //flush out all serial data

  //setup hardware buildin led and flash_button
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(FLASH_BUTTON, INPUT);
  
  //--- setup randomseed the right way
  #if defined(ESP8266) 
    //--- This is 8266 HWRNG used to seed the Random PRNG
    //--- Read more: https://config9.com/arduino/getting-a-truly-random-number-in-arduino/
    randomSeed(RANDOM_REG32); 
  #elif defined(ESP32)
    randomSeed(esp_random());
  #endif
  strlcpy(settingHostname, _DEFAULT_HOSTNAME, sizeof(settingHostname));
  

//================ oLed =======================================
  if (settingOledType > 0)
  {
    oled_Init();
    oled_Clear();  // clear the screen so we can paint the menu.
    oled_Print_Msg(0, "<DSMRlogger-Next>", 0);
    oled_Print_Msg(1, _SEMVER_FULL , 0);
    oled_Print_Msg(2, "The next DSMRlogger", 0);
    oled_Print_Msg(3, " >> Enjoy logging! <<", 1000);
    yield();
  }
  else  // don't blink if oled-screen attatched
  {
    for(int i=8; i>0; i--) 
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      Debug(".");
      delay(500);
    }
    Debugln();
  }
  digitalWrite(LED_BUILTIN, LED_OFF);  // HIGH is OFF
  lastReset = getResetReason();
  Debugf("\n\nReset reason....[%s]\r\n", lastReset.c_str());  

  
//=============start Networkstuff==================================
  if (settingOledType > 0)
  {
    if (settingOledFlip)  oled_Init();  // only if true restart(init) oled screen
    oled_Clear();                       // clear the screen 
    oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
    oled_Print_Msg(1, "Verbinden met WiFi", 500);
  }
  digitalWrite(LED_BUILTIN, LED_ON);
  startWiFi(settingHostname, 240);  // timeout 4 minuten
  Debugln(F("Wifi started..."));
  Debug (F("Connected to " )); Debugln (WiFi.SSID());
  Debug (F("IP address: " ));  Debugln (WiFi.localIP());
  Debug (F("IP gateway: " ));  Debugln (WiFi.gatewayIP());

  if (settingOledType > 0)
  {
    oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
    oled_Print_Msg(1, WiFi.SSID().c_str(), 0);
    snprintf(cMsg, sizeof(cMsg), "IP %s", WiFi.localIP().toString().c_str());
    oled_Print_Msg(2, cMsg, 1500);
  }
  else
  {
    digitalWrite(LED_BUILTIN, LED_OFF);
    for (int L=0; L < 10; L++) {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(200);
    }
  }
  digitalWrite(LED_BUILTIN, LED_OFF);

//-----------------------------------------------------------------
#ifdef USE_SYSLOGGER
  openSysLog(false);
  snprintf(cMsg, sizeof(cMsg), "SSID:[%s],  IP:[%s], Gateway:[%s]", WiFi.SSID().c_str()
                                                                  , WiFi.localIP().toString().c_str()
                                                                  , WiFi.gatewayIP().toString().c_str());
  writeToSysLog("%s", cMsg);
#endif
//-----------------------------------------------------------------

  startMDNS(settingHostname);
  if (settingOledType > 0)
  {
    oled_Print_Msg(3, "mDNS gestart", 1500);
  }

  Debugf("Starting Telnet");    
  startTelnet();
  if (settingOledType > 0)
  {
    oled_Print_Msg(0, " <DSMRloggerAPI>", 0);
    oled_Print_Msg(3, "telnet (poort 23)", 2500);
  }
  Debugln("Debug open for business on port 23");
  
//=============end Networkstuff======================================

//============= start SPIFFS ========================================
  if (SPIFFS.begin()) 
  {
    DebugTln(F("SPIFFS Mount succesfull\r"));
    SPIFFSmounted = true;
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
      oled_Print_Msg(3, "SPIFFS mounted", 1500);
    }    
  } else { 
    DebugTln(F("SPIFFS Mount failed\r"));   // Serious problem with SPIFFS 
    SPIFFSmounted = false;
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
      oled_Print_Msg(3, "SPIFFS FAILED!", 2000);
    }
  }

//------ read status file for last Timestamp --------------------
  strncpy(actTimestamp, "040302010101X", sizeof(actTimestamp));
  //==========================================================//
  // writeLastStatus();  // only for firsttime initialization //
  //==========================================================//
  readLastStatus(); // place it in actTimestamp
  // set the time to actTimestamp!
  actT = epoch(actTimestamp, strlen(actTimestamp), true);
  DebugTf("===> actTimestamp[%s]-> nrReboots[%u] - Errors[%u]\r\n\n", actTimestamp
                                                                    , nrReboots++
                                                                    , slotErrors);                                                                    
  readSettings(true);

//============= end SPIFFS ========================================

#if defined(USE_NTP_TIME)                                   //USE_NTP
//================ startNTP =========================================
  if (settingOledType > 0)                                  //USE_NTP
  {                                                         //USE_NTP
    oled_Print_Msg(3, "setup NTP server", 100);             //USE_NTP
  }                                                         //USE_NTP
                                                            //USE_NTP
  if (!startNTP())                                          //USE_NTP
  {                                                         //USE_NTP
    DebugTln(F("ERROR!!! No NTP server reached!\r\n\r"));   //USE_NTP
    if (settingOledType > 0)                                //USE_NTP
    {                                                       //USE_NTP
      oled_Print_Msg(0, " <DSMRlogger-Next>", 0);              //USE_NTP
      oled_Print_Msg(2, "geen reactie van", 100);           //USE_NTP
      oled_Print_Msg(2, "NTP server's", 100);               //USE_NTP 
      oled_Print_Msg(3, "Reboot DSMR-logger", 2000);        //USE_NTP
    }                                                       //USE_NTP
    delay(2000);                                            //USE_NTP
    esp_reboot();                                          //USE_NTP
    delay(3000);                                            //USE_NTP
  }                                                         //USE_NTP
  if (settingOledType > 0)                                  //USE_NTP
  {                                                         //USE_NTP
    oled_Print_Msg(0, " <DSMRlogger-Next>", 0);                //USE_NTP
    oled_Print_Msg(3, "NTP gestart", 1500);                 //USE_NTP
  }                                                         //USE_NTP
  prevNtpHour = hour();                                     //USE_NTP
                                                            //USE_NTP
#endif  //USE_NTP_TIME                                      //USE_NTP
//================ end NTP =========================================

  snprintf(cMsg, sizeof(cMsg), "Last reset reason: [%s]\r", getResetReason().c_str());
  DebugTln(cMsg);

  Debugf("\nGebruik 'telnet %s ' voor verdere debugging\r\n",WiFi.localIP().toString().c_str());

//=============now test if SPIFFS is correct populated!============
  if (DSMRfileExist(settingIndexPage, false) )
  {
    if (strcmp(settingIndexPage, "DSMRindex.html") != 0)
    {
      if (settingIndexPage[0] != '/')
      {
        char tempPage[50] = "/";
        strlcat(tempPage, settingIndexPage, 49);
        strlcpy(settingIndexPage, tempPage, sizeof(settingIndexPage));
      }
      hasAlternativeIndex        = true;
    }
    else  hasAlternativeIndex    = false;
  }
  if (!hasAlternativeIndex && !DSMRfileExist("/DSMRindex.html", false) )
  {
    spiffsNotPopulated = true;
  }
  if (!hasAlternativeIndex)    //--- there's no alternative index.html
  {
    DSMRfileExist("/DSMRindex.js",    false);
    DSMRfileExist("/DSMRindex.css",   false);
    DSMRfileExist("/DSMRgraphics.js", false);
  }
  if (!DSMRfileExist("/FSexplorer.html", true))
  {
    spiffsNotPopulated = true;
  }
  if (!DSMRfileExist("/FSexplorer.css", true))
  {
    spiffsNotPopulated = true;
  }
//=============end SPIFFS =========================================
#ifdef USE_SYSLOGGER
  if (spiffsNotPopulated)
  {
    sysLog.write("SPIFFS is not correct populated (files are missing)");
  }
#endif
  
//=============now test if "convertPRD" file exists================

  if (SPIFFS.exists("/!PRDconvert") )
  {
    convertPRD2RING();
  }

//=================================================================

#if defined(USE_NTP_TIME)                                                           //USE_NTP
  time_t t = now(); // store the current time in time variable t                    //USE_NTP
  snprintf(cMsg, sizeof(cMsg), "%02d%02d%02d%02d%02d%02dW\0\0"                      //USE_NTP
                                               , (year(t) - 2000), month(t), day(t) //USE_NTP
                                               , hour(t), minute(t), second(t));    //USE_NTP
  pTimestamp = cMsg;                                                                //USE_NTP
  DebugTf("Time is set to [%s] from NTP\r\n", cMsg);                                //USE_NTP
#endif  // use_dsmr_30

  if (settingOledType > 0)
  {
    snprintf(cMsg, sizeof(cMsg), "DT: %02d%02d%02d%02d0101W", thisYear
                                                            , thisMonth, thisDay, thisHour);
    oled_Print_Msg(0, " <DSMRlogger-Next>", 0);
    oled_Print_Msg(3, cMsg, 1500);
  }

//================ Start MQTT  ======================================

#ifdef USE_MQTT                                                 //USE_MQTT
  connectMQTT();                                                //USE_MQTT
  if (settingOledType > 0)                                      //USE_MQTT
  {                                                             //USE_MQTT
    oled_Print_Msg(0, " <DSMRlogger-Next>", 0);                    //USE_MQTT
    oled_Print_Msg(3, "MQTT server set!", 1500);                //USE_MQTT
  }                                                             //USE_MQTT
#endif                                                          //USE_MQTT

//================ End of Start MQTT  ===============================


//================ Start HTTP Server ================================

  if (!spiffsNotPopulated) {
    DebugTln(F("SPIFFS correct populated -> normal operation!\r"));
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, " <DSMRlogger-Next>", 0); 
      oled_Print_Msg(1, "OK, SPIFFS correct", 0);
      oled_Print_Msg(2, "Verder met normale", 0);
      oled_Print_Msg(3, "Verwerking ;-)", 2500);
    }

      
    DebugTln(F("Starting HTTP server now..."));
    httpServer.begin();
    DebugTln( "HTTP server gestart." );

    if (hasAlternativeIndex)
    {
      DebugTln(F("has Alternative Index"));
      httpServer.serveStatic("/",                 SPIFFS, settingIndexPage);
      httpServer.serveStatic("/index",            SPIFFS, settingIndexPage);
      httpServer.serveStatic("/index.html",       SPIFFS, settingIndexPage);
      httpServer.serveStatic("/DSMRindex.html",   SPIFFS, settingIndexPage);
    }
    else
    {
      DebugTln(F("has Alternative Index"));
      httpServer.serveStatic("/",                 SPIFFS, "/DSMRindex.html");
      DebugTln(F("added serverStatic [/]"));
      httpServer.serveStatic("/DSMRindex.html",   SPIFFS, "/DSMRindex.html");
      DebugTln(F("added serverStatic [/DSMRindex.html]"));
      httpServer.serveStatic("/index",            SPIFFS, "/DSMRindex.html");
      DebugTln(F("added serverStatic [/index]"));
      httpServer.serveStatic("/index.html",       SPIFFS, "/DSMRindex.html");
      DebugTln(F("added serverStatic [/index.html]"));
      httpServer.serveStatic("/DSMRindex.css",    SPIFFS, "/DSMRindex.css");
      DebugTln(F("added serverStatic [/DSMRindex.css]"));
      httpServer.serveStatic("/DSMRindex.js",     SPIFFS, "/DSMRindex.js");
      DebugTln(F("added serverStatic [/DSMRindex.js]"));
      httpServer.serveStatic("/DSMRgraphics.js",  SPIFFS, "/DSMRgraphics.js");
      DebugTln(F("serverStatic [/DSMRgraphics.js]"));
    }
  } else {
    DebugTln(F("Oeps! not all files found on SPIFFS -> present FSexplorer!\r"));
    spiffsNotPopulated = true;
    if (settingOledType > 0)
    {
      oled_Print_Msg(0, "!OEPS! niet alle", 0);
      oled_Print_Msg(1, "files op SPIFFS", 0);
      oled_Print_Msg(2, "gevonden! (fout!)", 0);
      oled_Print_Msg(3, "Start FSexplorer", 2000);
    }
  }
  
  DebugTln(F("setupFSexplorer"));
  setupFSexplorer();
  httpServer.serveStatic("/FSexplorer.png",   SPIFFS, "/FSexplorer.png");

  DebugTln(F("setup RESTAPI interface"));
  httpServer.on("/api", HTTP_GET, processAPI);
  // all other api calls are catched in FSexplorer onNotFounD!


  if (settingOledType > 0)                                  //HAS_OLED
  {                                                         //HAS_OLED
    oled_Clear();                                           //HAS_OLED
    oled_Print_Msg(0, " <DSMRlogger-Next>", 0);                //HAS_OLED
    oled_Print_Msg(2, "HTTP server ..", 0);                 //HAS_OLED
    oled_Print_Msg(3, "gestart (poort 80)", 0);             //HAS_OLED
  }                                                         //HAS_OLED

  for (int i = 0; i< 10; i++) 
  {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(250);
  }
//================ End HTTP Server ================================

  //test(); monthTabel

//================ Start Mindergas =================================

#ifdef USE_MINDERGAS
    handleMindergas();
#endif

//================ End of Mindergas ================================
 

//================ Start InfluxDB  =================================

#ifdef USE_INFLUXDB
  initInfluxDB();
#endif

//================ End of InfluxDB ================================

//================ Start SolarEdge_Modbus =========================
//  Modbus_SolarEdge.client();
//================ End SolarEdge_Modbus ===========================

//================ Start Slimme Meter ===============================

  if (settingOledType > 0)
  {
    oled_Print_Msg(0, "<DSMRlogger-Next>", 0);
    oled_Print_Msg(1, "Startup complete", 0);
    oled_Print_Msg(2, "Wait for first", 0);
    oled_Print_Msg(3, "telegram .....", 500);
  }

  DebugTln(F("Start slimmeMeter..."));
  initSlimmermeter();
  slimmeMeter.enable(true);

//================ The final part of the Setup =====================

  DebugTf("Startup complete! actTimestamp[%s]\r\n", actTimestamp);  
  writeToSysLog("Startup complete! actTimestamp[%s]", actTimestamp);  

//================ Find out DST--------------- =====================
  isDST = isdsmrDST(String(actTimestamp).c_str());
  if (isDST)
  {
    DebugTln("Summertime");
  }
  else 
  {
    DebugTln("Wintertime");
  }
//================ End of Slimmer Meter ============================
} // setup()


//===[ no-blocking delay with running background tasks in ms ]============================
DECLARE_TIMER_MS(timer_delay_ms, 1);
void delayms(unsigned long delay_ms)
{
  CHANGE_INTERVAL_MS(timer_delay_ms, delay_ms);
  RESTART_TIMER(timer_delay_ms);
  while (!DUE(timer_delay_ms))
  {
    doSystemTasks();
  }
    
} // delayms()

//========================================================================================

//===[ blink the LED ]====================================================================
void blinkLED()
{
  // Blink once
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

//===[ If wifi is disconneted then blink 10 times ]=======================================

void doCheckWifiConnection()
{
  //when wifi is not connected, the blink fast
  if (WiFi.status() != WL_CONNECTED)
  {
    for(int b=0; b<10; b++) { blinkLED(); delay(75);}
  }
  
}

//===[ If wifi is not connected, then try to reconnect ]=================================

void doReconnectWifi()
{
  if (WiFi.status() == WL_CONNECTED) return;
  // if not connected, then try reconnect
  writeToSysLog("Restart wifi with [%s]...", settingHostname);
  startWiFi(settingHostname, 10);
  if (WiFi.status() != WL_CONNECTED)
  {
    writeToSysLog("%s", "Wifi still not connected! Waiting for next attempt.");
  }
  else 
  {
        snprintf(cMsg, sizeof(cMsg), "IP:[%s], Gateway:[%s]", WiFi.localIP().toString().c_str()
                                                            , WiFi.gatewayIP().toString().c_str());
        writeToSysLog("%s", cMsg);

        #ifdef USE_INFLUXDB
          //On reconnect wifi, also reconnect InfluxDB
          initInfluxDB();
        #endif
  }
}
//==[ Do Read Solaredge Modbus TCP ]==========================================================
// void doReadSolarEdge()
// {
//   uint16_t result = 0;
//   for (uint8_t i =0;  i < Modbus_SolarEdge_Reg_number; i++)
//   { 

//     if (Modbus_SolarEdge.isConnected(Modbus_SolarEdge_IP)) {                // Check if connection to Modbus Slave is established
//       Modbus_SolarEdge.readHreg(Modbus_SolarEdge_IP, Modbus_SolarEdge_Reg_offset, &result);     // Initiate Read Coil from Modbus Slave
//     } else {
//       Modbus_SolarEdge.connect(Modbus_SolarEdge_IP, Modbus_SolarEdge_port);           // Try to connect if no connection
//     }
//     Modbus_SolarEdge.task();                                   // Common local Modbus task
//     delay(100);                                                  // Pulling interval
//     Serial.printf("[reg][resp]=[%d][%d]\r\n", i, result);

//   }
// }

//==[ Do Telegram Processing ]===============================================================
void doTaskTelegram()
{
  //Trigger next telegram (or just generate data in case of no slimmemeter)
  if (Verbose1) DebugTln("doTaskTelegram");
  #if defined(HAS_NO_SLIMMEMETER)
    handleTestdata();  
  #else
    tiggerNextTelegram();
  #endif
  blinkLED();
}


//===[ Do System tasks ]=============================================================
void doSystemTasks()
{
  #ifndef HAS_NO_SLIMMEMETER
    //It's async serial device, so it can receive the next telegram, when done, trigger processing.
    //Do not use just slimmeMeter.loop(), it only "receives data", not process when done.
    handleSlimmemeter();  
  #endif
  #ifdef USE_MQTT
    MQTTclient.loop();
  #endif
  httpServer.handleClient();
#if defined(ESP8266)
  MDNS.update();
#endif
  handleKeyInput();
  if (settingOledType > 0)
  {
    checkFlashButton();
  }

  yield();

} // doSystemTasks()

  
void loop () 
{  
  //--- do the tasks that has to be done 
  //--- as often as possible
  doSystemTasks();

  loopCount++;

  //--- update upTime counter
  if DUE(updateSeconds)
  {
    upTimeSeconds++;
  }
    
  //--- verwerk volgend telegram
  if DUE(nextTelegram)
  {
    doTaskTelegram();
    //doReadSolarEdge();
  }

//--- if an OLED screen attached, display the status
  if (settingOledType > 0)
  {
    if DUE(updateDisplay)
    {
      displayStatus();
    }
  }

  //--- if mindergas then check
#ifdef USE_MINDERGAS
  if DUE(minderGasTimer) 
    handleMindergas();
#endif

  //--- if connection lost, try to reconnect to WiFi
  if DUE(reconnectWiFi) 
    doReconnectWifi();

//--- if NTP set, see if it needs synchronizing
#if defined(USE_NTP_TIME)                                           //USE_NTP
  if DUE(synchrNTP)                                                 //USE_NTP
  {
  //if (timeStatus() == timeNeedsSync || prevNtpHour != hour())     //USE_NTP
  //{
      //prevNtpHour = hour();                                         //USE_NTP
      setSyncProvider(getNtpTime);                                  //USE_NTP
      setSyncInterval(600);                                         //USE_NTP
  //}
  }
#endif                                                              //USE_NTP
  
  yield();
  
} // loop()


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
