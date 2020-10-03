/*
***************************************************************************  
**  Program  : handleInfluxDB - part of DSMRlogger-Next
**  Version  : v2.1.2-rc2
**
**  Copyright (c) 2020 Robert van den Breemen
**
**  Description: Handles sending all data directly to an influxdb instance
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*      Created by Robert van den Breemen (26 june 2020)
*/  
#ifdef USE_INFLUXDB
// InfluxDB  server url. Don't use localhost, always server name or ip address.
// For InfluxDB 2 e.g. http://192.168.1.48:9999 (Use: InfluxDB UI -> Load Data -> Client Libraries), 
// For InfluxDB 1 e.g. http://192.168.1.48:8086
#define INFLUXDB_URL "http://192.168.88.254:8086"
//#define INFLUXDB_URL "http://127.0.0.1:8086"
// InfluxDB 2 server or cloud API authentication token (Use: InfluxDB UI -> Load Data -> Tokens -> <select token>)
//#define INFLUXDB_TOKEN "toked-id"
// InfluxDB 2 organization id (Use: InfluxDB UI -> Settings -> Profile -> <name under tile> )
//#define INFLUXDB_ORG "org"
// InfluxDB 2 bucket name (Use: InfluxDB UI -> Load Data -> Buckets)
//#define INFLUXDB_BUCKET "bucket"
// InfluxDB v1 database name 
#define INFLUXDB_DB_NAME "dsmr-api"
#include <InfluxDbClient.h>

// InfluxDB client instance
//InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN);
// InfluxDB client instance for InfluxDB 1
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_DB_NAME);

// Set timezone string according to https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html
// Examples:
//  Pacific Time:   "PST8PDT"
//  Eastern:        "EST5EDT"
//  Japanesse:      "JST-9"
//  Central Europe: "CET-1CEST,M3.5.0,M10.5.0/3"
#define TZ_INFO "CET-1CEST,M3.5.0,M10.5.0/3"
#define WRITE_PRECISION WritePrecision::S
#define MAX_BATCH_SIZE 16
#define WRITE_BUFFER_SIZE 32

time_t thisEpoch;

void initInfluxDB()
{
  if ( sizeof(settingInfluxDBhostname) < 8 )  return; 

 // Set InfluxDB 1 authentication params
  // Only needed when query's are done?
  //client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_DB_NAME, INFLUXDB_USER, INFLUXDB_PASSWORD);   
  snprintf(cMsg, sizeof(cMsg), "http://%s:%d", settingInfluxDBhostname , settingInfluxDBport);
  DebugTf("InfluxDB Connection Setup: [%s] - database: [%s]\r\n", cMsg , settingInfluxDBdatabasename);
  client.setConnectionParamsV1(cMsg, settingInfluxDBdatabasename);
  Debugln("InfluxDB client setConnectionsParamsV1 done!");
  // Connect to influxdb server connection

  if (client.validateConnection()) {
    DebugT("Connected to InfluxDB: ");
    Debugln(client.getServerUrl());
  } else {
    DebugT("InfluxDB connection failed: ");
    Debugln(client.getLastErrorMessage());
  }
    
  //Enable messages batching and retry buffer
  client.setWriteOptions(WRITE_PRECISION, MAX_BATCH_SIZE, WRITE_BUFFER_SIZE);
//  client.setWriteOptions(WRITE_PRECISION);
}
struct writeInfluxDataPoints {
  template<typename Item>
  void apply(Item &i) {
    
    if (i.present()) 
    {
      if (strlen(Item::unit()) != 0) 
      {
        //when there is a unit, then it is a measurement
        Point pointItem(Item::unit());
        //pointItem.setTime(thisEpoch);
        pointItem.addTag("instance",Item::name);     
        pointItem.addField("value", i.val());
//        pointItem.addField((String)(Item::name), i.val());
        if (Verbose1) {
          DebugT("Writing to influxdb:");
          Debugln(pointItem.toLineProtocol());          
        }
        if (!client.writePoint(pointItem)) {
          DebugT("InfluxDB write failed: ");
          Debugln(client.getLastErrorMessage());
        }
      }//writing to influxdb      
    }
  }
};

void handleInfluxDB()
{
  static uint32_t lastTelegram = 0;
  if (!client.validateConnection()) return; // only write if there is a valid connection to InfluxDB
  if ((telegramCount - lastTelegram)> 0)
  {
    //New telegram received, let's forward that to influxDB
    lastTelegram = telegramCount;
    //Setup the timestamp for this telegram, so all points for this batch are the same.
    //thisEpoch = UTC.now();  
    //DebugTf("Writing telegram to influxdb - Epoc = %d (this) %d (NL) %d (UTC) \r\n", (int)thisEpoch, (int)localTZ.now(), (int)UTC.now());
    uint32_t timeThis = millis();
    DSMRdata.applyEach(writeInfluxDataPoints());
    // Check whether buffer in not empty
    if (!client.isBufferEmpty()) {
      // Write all remaining points to db
      client.flushBuffer();
    }
    DebugTf("Influxdb write took [%d] ms\r\n", (int)(millis()-timeThis));
  }
  
}
#endif

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
