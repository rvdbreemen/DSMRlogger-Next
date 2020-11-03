/* 
***************************************************************************  
**  Program  : MQTTstuff, part of DSMRlogger-Next
**  Version  : v2.3.0-rc5
**
**  Copyright (c) 2020 Robert van den Breemen
**   Based on (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
**  RvdB  changed MQTT stuff to FSM 
**  AaW   simplified and restructured the FSM 
**  RvdB  added automatic discovery home assistant style (and domoticz?)
*/

// Declare some variables within global scope

  static IPAddress  MQTTbrokerIP;
  static char       MQTTbrokerIPchar[20] {0};

#ifdef USE_MQTT
//  #include <PubSubClient.h>           // MQTT client publish and subscribe functionality
  
//  static PubSubClient MQTTclient(wifiClient);
  int8_t              reconnectAttempts = 0;
  char                lastMQTTtimestamp[15] = "-";
  char                mqttBuff[100] {0};


  enum states_of_MQTT { MQTT_STATE_INIT, MQTT_STATE_TRY_TO_CONNECT, MQTT_STATE_IS_CONNECTED, MQTT_STATE_ERROR };
  enum states_of_MQTT stateMQTT = MQTT_STATE_INIT;

  char                MQTTclientId[80] {0}; //hostname + mac 

#endif

//===========================================================================================
void connectMQTT() 
{
#ifdef USE_MQTT
  
  if (Verbose2) DebugTf("MQTTclient.connected(%d), mqttIsConnected[%d], stateMQTT [%d]\r\n"
                                              , MQTTclient.connected()
                                              , mqttIsConnected, stateMQTT);

  if (settingMQTTinterval == 0) {
    mqttIsConnected = false;
    return;
  }

  if (!MQTTclient.connected() || stateMQTT != MQTT_STATE_IS_CONNECTED)
  {
    mqttIsConnected = false;
    stateMQTT = MQTT_STATE_INIT;
  }

  mqttIsConnected = connectMQTT_FSM();
  
  if (Verbose1) DebugTf("connected()[%d], mqttIsConnected[%d], stateMQTT [%d]\r\n"
                                              , MQTTclient.connected()
                                              , mqttIsConnected, stateMQTT);

  CHANGE_INTERVAL_MIN(reconnectMQTTtimer, 5);

#endif
}


//===========================================================================================
bool connectMQTT_FSM() 
{
#ifdef USE_MQTT
  
  switch(stateMQTT) 
  {
    case MQTT_STATE_INIT:  
          DebugTln(F("MQTT State: MQTT Initializing"));
          WiFi.hostByName(settingMQTTbroker, MQTTbrokerIP);  // lookup the MQTTbroker convert to IP
          snprintf(MQTTbrokerIPchar, sizeof(MQTTbrokerIPchar), "%d.%d.%d.%d", MQTTbrokerIP[0]
                                                                            , MQTTbrokerIP[1]
                                                                            , MQTTbrokerIP[2]
                                                                            , MQTTbrokerIP[3]);
          if (!isValidIP(MQTTbrokerIP))  
          {
            DebugTf("ERROR: [%s] => is not a valid URL\r\n", settingMQTTbroker);
            settingMQTTinterval = 0;
            DebugTln(F("Next State: MQTT_STATE_ERROR"));
            stateMQTT = MQTT_STATE_ERROR;
            return false;
          }
          
          //MQTTclient.disconnect();
          //DebugTf("disconnect -> MQTT status, rc=%d \r\n", MQTTclient.state());
          DebugTf("[%s] => setServer(%s, %d) \r\n", settingMQTTbroker, MQTTbrokerIPchar, settingMQTTbrokerPort);
          MQTTclient.setServer(MQTTbrokerIPchar, settingMQTTbrokerPort);
          DebugTf("setServer  -> MQTT status, rc=%d \r\n", MQTTclient.state());
          strlcpy(MQTTclientId, settingHostname, sizeof(MQTTclientId));
          strlcat(MQTTclientId, "-", sizeof(MQTTclientId));
          strlcat(MQTTclientId, WiFi.macAddress().c_str(), sizeof(MQTTclientId));
          //MQTTclientId  = String(settingHostname) + "-" + WiFi.macAddress();
          stateMQTT = MQTT_STATE_TRY_TO_CONNECT;
          DebugTln(F("Next State: MQTT_STATE_TRY_TO_CONNECT"));
          reconnectAttempts = 0;

    case MQTT_STATE_TRY_TO_CONNECT:
          DebugTln(F("MQTT State: MQTT try to connect"));
          DebugTf("MQTT server is [%s], IP[%s]\r\n", settingMQTTbroker, MQTTbrokerIPchar);
      
          DebugTf("Attempting MQTT connection as [%s] .. \r\n", MQTTclientId);
          reconnectAttempts++;

          //--- If no username, then anonymous connection to broker, otherwise assume username/password.
          if (strlen(settingMQTTuser) == 0) 
          {
            DebugT(F("without a Username/Password "));
            MQTTclient.connect(MQTTclientId);
          } 
          else 
          {
            DebugTf("with Username [%s] and password ", settingMQTTuser);
            MQTTclient.connect(MQTTclientId, settingMQTTuser, settingMQTTpasswd);
          }
          //--- If connection was made succesful, move on to next state...
          if (MQTTclient.connected())
          {
            reconnectAttempts = 0;  
            Debugf(" .. connected -> MQTT status, rc=%d\r\n", MQTTclient.state());
            MQTTclient.loop();
            doAutoConfigure(); //HA Auto-Discovery 
            stateMQTT = MQTT_STATE_IS_CONNECTED;
            return true;
          }
          Debugf(" -> MQTT status, rc=%d \r\n", MQTTclient.state());
      
          //--- After 3 attempts... go wait for a while.
          if (reconnectAttempts >= 3)
          {
            DebugTln(F("3 attempts have failed. Retry wait for next reconnect in 10 minutes\r"));
            stateMQTT = MQTT_STATE_ERROR;  // if the re-connect did not work, then return to wait for reconnect
            DebugTln(F("Next State: MQTT_STATE_ERROR"));
          }   
          break;
          
    case MQTT_STATE_IS_CONNECTED:
          MQTTclient.loop();
          return true;

    case MQTT_STATE_ERROR:
          DebugTln(F("MQTT State: MQTT ERROR, wait for 10 minutes, before trying again"));
          //--- next retry in 10 minutes.
          CHANGE_INTERVAL_MIN(reconnectMQTTtimer, 10);
          break;

    default:
          DebugTln(F("MQTT State: default, this should NEVER happen!"));
          //--- do nothing, this state should not happen
          stateMQTT = MQTT_STATE_INIT;
          CHANGE_INTERVAL_MIN(reconnectMQTTtimer, 10);
          DebugTln(F("Next State: MQTT_STATE_INIT"));
          break;
  }
#endif

  return false;  

} // connectMQTT_FSM()


//=======================================================================
struct buildJsonMQTT {
#ifdef USE_MQTT

    char topicId[100];

    template<typename Item>
    void apply(Item &i) {
      if (i.present()) 
      {
        String Name = Item::name;
        //-- for dsmr30 -----------------------------------------------
  #if defined( USE_PRE40_PROTOCOL )
        if (Name.indexOf("gas_delivered2") == 0) Name = "gas_delivered";
  #endif
        String Unit = Item::unit();

        if (settingMQTTtopTopic[strlen(settingMQTTtopTopic)-1] == '/')
              snprintf(topicId, sizeof(topicId), "%s",  settingMQTTtopTopic);
        else  snprintf(topicId, sizeof(topicId), "%s/", settingMQTTtopTopic);
        strlcat(topicId, Name.c_str(), sizeof(topicId));
        if (Verbose2) DebugTf("topicId[%s]\r\n", topicId);
        
        if (Unit.length() > 0)
        {
          createMQTTjsonMessage(mqttBuff, Name.c_str(), i.val(), Unit.c_str());
        }
        else
        {
          createMQTTjsonMessage(mqttBuff, Name.c_str(), i.val());
        }
        
        //snprintf(cMsg, sizeof(cMsg), "%s", jsonString.c_str());
        //DebugTf("topicId[%s] -> [%s]\r\n", topicId, mqttBuff);
        if (!MQTTclient.publish(topicId, mqttBuff, true))
        {
          DebugTf("Error publish(%s) [%s] [%d bytes]\r\n", topicId, mqttBuff, (strlen(topicId) + strlen(mqttBuff)));
        }
      }
  }
#endif

};  // struct buildJsonMQTT


//===========================================================================================
void sendMQTTData() 
{
#ifdef USE_MQTT
  String dateTime, topicId, json;

  if (settingMQTTinterval == 0) return;   // 0 == turned off

  if (ESP.getFreeHeap() < 7000) // to prevent firmware from crashing!
  {
    DebugTf("==> Bailout due to low heap (%d bytes)\r\n",   ESP.getFreeHeap() );
    writeToSysLog("==> Bailout low heap (%d bytes)", ESP.getFreeHeap() );
    esp_reboot(); //due to low memory, let's restart.
    return;
  }

  //Only send data when there is a new telegram
  static uint32_t lastTelegram = 0;
  if (telegramCount != lastTelegram)
  {
      //New telegram received, let's forward that to influxDB
      lastTelegram = telegramCount;
  } else return;

  if (!MQTTclient.connected() || ! mqttIsConnected)
  {
    DebugTf("MQTTclient.connected(%d), mqttIsConnected[%d], stateMQTT [%d]\r\n"
                                              , MQTTclient.connected()
                                              , mqttIsConnected, stateMQTT);
  }
  if (!MQTTclient.connected())  
  {
    if ( DUE( reconnectMQTTtimer) || mqttIsConnected)
    {
      mqttIsConnected = false;
      connectMQTT();
    }
    else
    {
      DebugTf("trying to reconnect in less than %d minutes\r\n", (TIME_LEFT_MIN(reconnectMQTTtimer) +1) );
    }
    if ( !mqttIsConnected ) 
    {
      DebugTln("no connection with a MQTT broker ..");
      return;
    }
  }

  DebugTf("Sending data to MQTT server [%s]:[%d]\r\n", settingMQTTbroker, settingMQTTbrokerPort);
  
  DSMRdata.applyEach(buildJsonMQTT());

#endif

} // sendMQTTData()

//===========================================================================================
void sendMQTTData(const String item, const String json)
{
  sendMQTTData(item.c_str(), json.c_str());
} 

void sendMQTTData(const char* item, const char *json) 
{
/*  
* The maximum message size, including header, is 128 bytes by default. 
* This is configurable via MQTT_MAX_PACKET_SIZE in PubSubClient.h.
* Als de json string te lang wordt zal de string niet naar de MQTT server
* worden gestuurd. Vandaar de korte namen als ED en PDl1.
* Mocht je langere, meer zinvolle namen willen gebruiken dan moet je de
* MQTT_MAX_PACKET_SIZE dus aanpassen!!!
*/
//===========================================================================================

  if (!MQTTclient.connected() || !isValidIP(MQTTbrokerIP)) return;
  // DebugTf("Sending data to MQTT server [%s]:[%d]\r\n", settingMQTTbroker.c_str(), settingMQTTbrokerPort);
  char topic[100];
  snprintf(topic, sizeof(topic), "%s/", settingMQTTtopTopic);
  strlcat(topic, item, sizeof(topic));
  DebugTf("Sending MQTT: TopicId [%s] Message [%s]\r\n", topic, json);
  if (!MQTTclient.publish(topic, json, true)) DebugTln("MQTT publish failed.");
  delay(0);
} // sendMQTTData()

//===========================================================================================
void sendMQTT(const char* topic, const char *json, const int8_t len) 
{
  if (!MQTTclient.connected() || !isValidIP(MQTTbrokerIP)) return;
  // DebugTf("Sending data to MQTT server [%s]:[%d] ", settingMQTTbroker.c_str(), settingMQTTbrokerPort);  
  DebugTf("Sending MQTT: TopicId [%s] Message [%s]\r\n", topic, json);
  if (MQTTclient.getBufferSize() < len) MQTTclient.setBufferSize(len); //resize buffer when needed
  if (!MQTTclient.publish(topic, json, true)) DebugTln("MQTT publish failed."); 
  delay(0);
} // sendMQTTData()

//===========================================================================================
bool splitString(String sIn, char del, String& cKey, String& cVal)
{
  sIn.trim();                                 //trim spaces
  cKey=""; cVal="";
  if (sIn.indexOf("//")==0) return false;     //comment, skip split
  if (sIn.length()<=3) return false;          //not enough buffer, skip split
  int pos = sIn.indexOf(del);                 //determine split point
  if ((pos==0) || (pos==(sIn.length()-1))) return false; // no key or no value
  cKey = sIn.substring(0,pos-1); cKey.trim(); //before, and trim spaces
  cVal = sIn.substring(pos+1); cVal.trim();   //after,and trim spaces
  return true;
}

//===========================================================================================
/*  
* The maximum message size, including header, is 128 bytes by default. 
* This is configurable via MQTT_MAX_PACKET_SIZE in PubSubClient.h.
* Als de json string te lang wordt zal de string niet naar de MQTT server
* worden gestuurd. 
* Check de langste auto discovery string die verzonden gaat worden, 
* indien meer dan MQTT_MAX_PACKET_SIZE !!!
* 
* Advies (3 november 2020): Stel het voor de zekerheid op 400 bytes in. 
*
*/
void doAutoConfigure()
{

  const char* cfgFilename = "/mqttha.cfg";
  String sTopic="";
  String sMsg="";
  File fh; //filehandle
  //Let's open the MQTT autoconfig file
  SPIFFS.begin();
  if (SPIFFS.exists(cfgFilename))
  {
    fh = SPIFFS.open(cfgFilename, "r");
    if (fh) {
      //Lets go read the config and send it out to MQTT line by line
      while(fh.available()) 
      {  //read file line by line, split and send to MQTT (topic, msg)
          delay(0);
          String sLine = fh.readStringUntil('\n');
          // DebugTf("sline[%s]\r\n", sLine.c_str());
          if (splitString( sLine, ',', sTopic, sMsg))
          {
            DebugTf("sTopic[%s], sMsg[%s]\r\n", sTopic.c_str(), sMsg.c_str());
            sendMQTT(sTopic.c_str(), sMsg.c_str(), (sTopic.length() + sMsg.length()+2));
          } else DebugTf("Either comment or invalid config line: [%s]\r\n", sLine.c_str());
      } // while available()
      fh.close();  
    } 
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
****************************************************************************
*/
