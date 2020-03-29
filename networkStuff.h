/*
***************************************************************************  
**  Program  : networkStuff.h, part of DSMRloggerAPI
**  Version  : v1.1.0
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/


#if defined(ESP8266)
    // ESP8266 specific code here

    #include <ESP8266WiFi.h>        //ESP8266 Core WiFi Library         
    #include <ESP8266WebServer.h>   // Version 1.0.0 - part of ESP8266 Core https://github.com/esp8266/Arduino
    #include <ESP8266mDNS.h>        // part of ESP8266 Core https://github.com/esp8266/Arduino
    #include <WiFiUdp.h>            // part of ESP8266 Core https://github.com/esp8266/Arduino

    #ifdef USE_UPDATE_SERVER
      //#include "ESP8266HTTPUpdateServer.h"
      
      #include "ModUpdateServer.h"  // https://github.com/mrWheel/ModUpdateServer
      #include "UpdateServerHtml.h"
    #endif
    
    #include <ESP_WiFiManager.h>        // version 0.14.0 - https://github.com/tzapu/WiFiManager
    #include <FS.h>                 // part of ESP8266 Core https://github.com/esp8266/Arduino

    ESP8266WebServer        httpServer (80);
    #ifdef USE_UPDATE_SERVER
      ESP8266HTTPUpdateServer httpUpdater(true);
    #endif

#elif defined(ESP32) || defined(ARDUINO_ARCH_ESP32)
    
    //#include <esp_wifi.h>
    #include <WiFi.h>
    #include <WiFiClient.h>
    #include <WiFi.h>      //ESP32 Core WiFi Library    
    #include <WebServer.h> // part of ESP32 Core 
    #include <ESPmDNS.h>   // part of ESP32 Core 

    #include <WiFiUdp.h>            // part of ESP32 Core
    #ifdef USE_UPDATE_SERVER
//      #include "ESP8266HTTPUpdateServer.h"
      #include <ESP32httpUpdate.h>
//      #include "ESP32ModUpdateServer.h"  // <<modified version of ESP32ModUpdateServer.h by Robert>>
      #include "UpdateServerHtml.h"   
    #endif
    
    #include <ESP_WiFiManager.h>    // https://github.com/khoih-prog/ESP_WiFiManager   
    #include <FS.h>                 // part of ESP32 Core
    
    WebServer        httpServer (80);
    #ifdef USE_UPDATE_SERVER
      ESP32HTTPUpdate httpUpdater();
    #endif
#else
      #error unexpected / unsupported architecture, make sure to compile for ESP32 or ESP8266
#endif

static      FSInfo SPIFFSinfo;
bool        SPIFFSmounted = false ; 
bool        isConnected = false;

//gets called when WiFiManager enters configuration mode
//===========================================================================================
//void configModeCallback (ESP_WiFiManager *myWiFiManager) 
//{
//  DebugTln(F("Entered config mode\r"));
//  DebugTln(WiFi.softAPIP().toString());
//  //if you used auto generated SSID, print it
//  DebugTln(myWiFiManager->getConfigPortalSSID());
//  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
//      oled_Clear();
//      oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
//      oled_Print_Msg(1, "AP mode active", 0);
//      oled_Print_Msg(2, "Connect to:", 0);
//      oled_Print_Msg(3, myWiFiManager->getConfigPortalSSID(), 0);
//  #endif
//
//} // configModeCallback()


//===========================================================================================
void startWiFi(const char* hostname, int timeOut) 
{
  uint32_t lTime = millis();
  String thisAP = String(hostname) + "-" + WiFi.macAddress();
  String Router_SSID;
  String Router_Pass;

  DebugT("Start Wifi Autoconnect...");

    // Use this to default DHCP hostname to ESP8266-XXXXXX or ESP32-XXXXXX
  //ESP_WiFiManager ESP_wifiManager;
  // Use this to personalize DHCP hostname (RFC952 conformed)
  ESP_WiFiManager manageWiFi(thisAP.c_str());
  //ESP_WiFiManager manageWiFi("AutoConnectAP");
  
  manageWiFi.setDebugOutput(true);

  //set custom ip for portal
  manageWiFi.setAPStaticIPConfig(IPAddress(192, 168, 100, 1), IPAddress(192, 168, 100, 1), IPAddress(255, 255, 255, 0));

  manageWiFi.setMinimumSignalQuality(-1);
  // Set static IP, Gateway, Subnetmask, DNS1 and DNS2. New in v1.0.5+
  manageWiFi.setSTAStaticIPConfig(IPAddress(192, 168, 2, 114), IPAddress(192, 168, 2, 1), IPAddress(255, 255, 255, 0),
                                       IPAddress(192, 168, 2, 1), IPAddress(8, 8, 8, 8));


  // We can't use WiFi.SSID() in ESP32 as it's only valid after connected.
  // SSID and Password stored in ESP32 wifi_ap_record_t and wifi_config_t are also cleared in reboot
  // Have to create a new function to store in EEPROM/SPIFFS for this purpose
  Router_SSID = manageWiFi.WiFi_SSID();
  Router_Pass = manageWiFi.WiFi_Pass();

  //Remove this line if you do not want to see WiFi password printed
  //DebufTln("Stored: SSID = " + Router_SSID + ", Pass = " + Router_Pass);

  if (Router_SSID != "")
  {
    manageWiFi.setConfigPortalTimeout(timeOut); //If no access point name has been previously entered disable timeout.
    DebugTf("Got stored Credentials. Timeout %ss\n", timeOut);
  }
  else
  {
    DebugTln("No stored Credentials. No timeout");
  }
  
//   //--- set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
//  manageWiFi.setAPCallback(configModeCallback);
//
//  //--- sets timeout until configuration portal gets turned off
//  //--- useful to make it all retry or go to sleep in seconds
//  //manageWiFi.setTimeout(240);  // 4 minuten
//  manageWiFi.setTimeout(timeOut);  // in seconden ...
//  
//  String chipID = String(ESP_getChipId(), HEX);
//  chipID.toUpperCase();
//  
//  // SSID and PW for Config Portal
//  String AP_SSID = "ESP_" + chipID + "_AutoConnectAP";
//  String AP_PASS = "MyESP_" + chipID;

  // Get Router SSID and PASS from EEPROM, then open Config portal AP named "ESP_XXXXXX_AutoConnectAP" and PW "MyESP_XXXXXX"
  // 1) If got stored Credentials, Config portal timeout is 60s
  // 2) If no stored Credentials, stay in Config portal until get WiFi Credentials
  //ESP_wifiManager.autoConnect(AP_SSID.c_str(), AP_PASS.c_str());
  //or use this for Config portal AP named "ESP_XXXXXX" and NULL password
  //manageWiFi.autoConnect();
  
  //--- fetches ssid and pass and tries to connect
  //--- if it does not connect it starts an access point with the specified name
  //--- here  "DSMR-API-<MAC>"
  //--- and goes into a blocking loop awaiting configuration
  if (!manageWiFi.autoConnect(thisAP.c_str())) 
  {
    DebugTln(F("failed to connect and hit timeout"));
#if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Clear();
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "Failed to connect", 0);
    oled_Print_Msg(2, "and hit TimeOut", 0);
    oled_Print_Msg(3, "**** NO WIFI ****", 0);
#endif

    //reset and try again, or maybe put it to deep sleep
    //delay(3000);
    //ESP.reset();
    //delay(2000);
    DebugTf(" took [%d] seconds ==> ERROR!\r\n", (millis() - lTime) / 1000);
    return;
  }
  //if you get here you have connected to the WiFi

  DebugTf("Connected with IP-address [%s]\r\n\r\n", WiFi.localIP().toString().c_str());
  #if defined( HAS_OLED_SSD1306 ) || defined( HAS_OLED_SH1106 )
    oled_Clear();
  #endif



#ifdef USE_UPDATE_SERVER
  httpUpdater.setup(&httpServer);
  httpUpdater.setIndexPage(UpdateServerIndex);
  httpUpdater.setSuccessPage(UpdateServerSuccess);
#endif
  DebugTf(" took [%d] seconds => OK!\r\n", (millis() - lTime) / 1000);
  
} // startWiFi()


//===========================================================================================
void startTelnet() 
{
  TelnetStream.begin();
  DebugTln(F("\nTelnet server started .."));
  TelnetStream.flush();

} // startTelnet()


//=======================================================================
void startMDNS(const char *Hostname) 
{
  DebugTf("[1] mDNS setup as [%s.local]\r\n", Hostname);
  if (MDNS.begin(Hostname))               // Start the mDNS responder for Hostname.local
  {
    DebugTf("[2] mDNS responder started as [%s.local]\r\n", Hostname);
  } 
  else 
  {
    DebugTln(F("[3] Error setting up MDNS responder!\r\n"));
  }
  MDNS.addService("http", "tcp", 80);
  
} // startMDNS()

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
