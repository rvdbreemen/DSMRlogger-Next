/*
***************************************************************************  
**  Program  : handleInfluxDB - part of DSMRlogger-Next
**  Version  : v2.1.1-rc1
**
**  Copyright (c) 2020 Robert van den Breemen
**
**  Description: Handles sending all data directly to an influxdb instance
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      Created by Robert van den Breemen (26 june 2020)
*                 my influxdb implementation (30 juli 2020)
*/  

#include <ESP8266HTTPClient.h>

#define LINE_BUFFER 250
char buffer[LINE_BUFFER] {0};
char sAPIwrite[100];
char sTmp[20];


time_t thisEpoch;
WiFiClient  influxdbClient;   
HTTPClient  http;

void initInfluxDB()
{
  if ( sizeof(settingInfluxDBhostname) < 8 )  return; 
 
  snprintf(sAPIwrite, sizeof(sAPIwrite),"http://%s:%d/write?db=%s&precision=s", settingInfluxDBhostname , settingInfluxDBport, settingInfluxDBdatabasename);
  DebugTf("API influxdb: %s\r\n",sAPIwrite);
}

//=========================================================================
// function to add a value to the lineprotocol buffer for influxdb
//=========================================================================
void addvalueInfluxdb(char *buff, const char *cValue)
{
  strlcat(buff, cValue, LINE_BUFFER);
} // addvalueInfluxdb(*char, *char)
//---------------------------------------------------------------
void addvalueInfluxdb(char *buff, String sValue)
{
  strlcat(buff, sValue.c_str(), LINE_BUFFER);
} // addvalueInfluxdb(*char, String)
//---------------------------------------------------------------
void addvalueInfluxdb(char *buff, int32_t iValue)
{
  snprintf(sTmp, 10, "%d", iValue);
  strlcat(buff, sTmp, LINE_BUFFER);
  strlcat(buff, "i", LINE_BUFFER);  
} // addvalueInfluxdb(*char,  int)
//---------------------------------------------------------------
void addvalueInfluxdb(char *buff, uint32_t uValue)
{
  snprintf(sTmp, 10, "%u", uValue);
  strlcat(buff, sTmp, LINE_BUFFER);
  strlcat(buff, "i", LINE_BUFFER);  
} // addvalueInfluxdb(char *buff, *char, uint)
//---------------------------------------------------------------
void addvalueInfluxdb(char *buff, float fValue)
{
  snprintf(sTmp, 10, "%.3f", fValue);
  strlcat(buff, sTmp, LINE_BUFFER);
} // addvalueInfluxdb(*char, float)

//=========[ writeInfluxDataPoints ]==========================================================

struct writeInfluxDataPoints {

    template<typename Item>
    void apply(Item &i) {
      if (i.present()) 
      {
        if (strlen(Item::unit()) != 0) 
        {
          strlcpy(buffer, Item::unit(), sizeof(buffer));                    //measurement name
          strlcat(buffer, ",instance=", sizeof(buffer));                    //addtag "instance"
          strncat_P(buffer, (PGM_P)Item::name, sizeof(buffer));             //DSMR items name = tag value
          strlcat(buffer, " value=", sizeof(buffer));                       //adddatapoint use "value" as name
          addvalueInfluxdb(buffer, i.val());                                //add actual value (polymorphism addvalue)
          strlcat(buffer, " ", sizeof(buffer));                             //whitespace delimiter for epoch timestamp
          strlcat(buffer, itoa(thisEpoch, sTmp, 10), sizeof(buffer));       //add epoch timestamp
          
          if (Verbose1) DebugTf("Influxdb lineprotocol buffer: [%s]\r\n",buffer);
          //Now sent the data
          if(WiFi.status()== WL_CONNECTED){
            // HTTPClient http;
            // Your Domain name with URL path or IP address with path
            http.begin(influxdbClient, sAPIwrite);
            http.setReuse(true); //try server keep-alive to prevent rebuilding http connection
            http.setUserAgent(F("DSMRlogger/" _SEMVER_FULL));
            const char * headerKeys[] = {"Transfer-Encoding"} ;
            http.collectHeaders(headerKeys, 1);
            // Specify content-type header
            http.addHeader(F("Content-Type"), F("text/plain)"));
            //Now sent the data
            int httpResponseCode = http.POST((uint8_t*)buffer, strlen(buffer));
            yield();
            if (httpResponseCode==204)
            {
              if (Verbose1) DebugTf("HTTP Response code: (%d) %s\r\n", httpResponseCode, http.getString().c_str());
            }
            else
            {
              if (Verbose1) DebugTf("HTTP Error code: (%d) %s\r\n", httpResponseCode, http.errorToString(httpResponseCode).c_str());
            }
            yield();
            http.end();
          } //wifi.status==wl_connected


        } // if data has a unit
      } // if data is present
  } //apply
};  // struct writeInfluxDataPoints

//=========[ handleInfluxDB ]=================================================================

void handleInfluxDB()
{
  static uint32_t lastTelegram = 0;
  // if (!client.validateConnection()) return; // only write if there is a valid connection to InfluxDB
  if ((telegramCount - lastTelegram)> 0)
  {
    //New telegram received, let's forward that to influxDB
    lastTelegram = telegramCount;

    //Setup the timestamp for this telegram, so all points for this batch are the same.
    thisEpoch = UTC.now(); 
    //DebugTf("UTC epoch     : %d\r\n", (int)UTC.now());
    //DebugTf("local epoch   : %d\r\n", (int)localTZ.now());
    //DebugTf("default epoch : %d\r\n", (int)now());

    DebugTf("Writing telegram to influxdb - Epoc UTC = %d (now) \r\n", (int)thisEpoch);

    uint32_t timeThis = millis();

    DSMRdata.applyEach(writeInfluxDataPoints());
 
    DebugTf("Influxdb write took [%d] ms\r\n", (int)(millis()-timeThis));
  }
  
}

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
