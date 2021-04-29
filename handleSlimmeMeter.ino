/*
***************************************************************************  
**  Program  : handleSlimmeMeter - part of DSMRloggerAPI
**  Version  : v3.0.0
**
**  Copyright (c) 2021 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/  

#if !defined(HAS_NO_SLIMMEMETER)
//==================================================================================
void handleSlimmemeter()
{
  //DebugTf("showRaw (%s)\r\n", showRaw ?"true":"false");
  if (showRaw) {
    //-- process telegrams in raw mode
    processSlimmemeterRaw();
  } 
  else
  {
    processSlimmemeter();
  } 

} // handleSlimmemeter()


//==================================================================================
void processSlimmemeterRaw()
{
  char    tlgrm[1200]   = "";
  char    checkSum[10]  = "";
   
  DebugTf("handleSlimmerMeter RawCount=[%4d]\r\n", showRawCount);
  showRawCount++;
  showRaw = (showRawCount <= 20);
  if (!showRaw)
  {
    showRawCount  = 0;
    return;
  }
  
  if (settingOledType > 0)
  {
    oled_Print_Msg(0, "<DSMRloggerAPI>", 0);
    oled_Print_Msg(1, "-------------------------",0);
    oled_Print_Msg(2, "Raw Format",0);
    snprintf(cMsg, sizeof(cMsg), "Raw Count %4d", showRawCount);
    oled_Print_Msg(3, cMsg, 0);
  }

  slimmeMeter.enable(true);
  Serial.setTimeout(5000);  // 5 seconds must be enough ..
  memset(tlgrm, 0, sizeof(tlgrm));
  int l = 0, lc = 0;
  // The terminator character is discarded from the serial buffer.
  l = Serial.readBytesUntil('/', tlgrm, sizeof(tlgrm));
  // now read from '/' to '!'
  // The terminator character is discarded from the serial buffer.
  l = Serial.readBytesUntil('!', tlgrm, sizeof(tlgrm));
  Serial.setTimeout(1000);  // seems to be the default ..
//  DebugTf("read [%d] bytes\r\n", l);
  if (l == 0) 
  {
    DebugTln(F("RawMode: Timeout - no telegram received within 5 seconds"));
    return;
  }

  tlgrm[l++] = '!';
  // next 6 bytes are "<CRC>\r\n"
  lc = Serial.readBytesUntil('\n', checkSum, sizeof(checkSum));
  for (int i=0; i<lc; i++)
  {
    tlgrm[l++] = checkSum[i];
    if (checkSum[i] == '\r')      DebugTf("added '\\r' at pos[%d]\r\n", l);
    else if (checkSum[i] == '\n') DebugTf("added '\\n' at pos[%d]\r\n", l);
    else  DebugTf("added [%c] at pos[%d]\r\n", checkSum[i], l);
  }
  tlgrm[l++]    = '\n';
  tlgrm[(l +1)] = '\0';
  // shift telegram 1 char to the right (make room at pos [0] for '/')
  for (int i=strlen(tlgrm); i>=0; i--) { tlgrm[i+1] = tlgrm[i]; yield(); }
  tlgrm[0] = '/'; 
  //Post result to Debug 
  Debugf("Telegram (%d chars):\r\n/%s\r\n", strlen(tlgrm), tlgrm); 
  return;
  
} // processSlimmemeterRaw()


//==================================================================================
void processSlimmemeter()
{
  bool doParseChecksum = true;
  
  if (settingPreDSMR40 == 1) doParseChecksum = false;

  slimmeMeter.loop();

  if (slimmeMeter.available() ) 
  {
    DebugTf("telegramCount=[%d] telegramErrors=[%d]\r\n", telegramCount, telegramErrors);
    Debugln(F("\r\n[Time----][FreeHeap/mBlck][Function----(line):\r"));
    // Voorbeeld: [21:00:11][   9880/  8960] loop        ( 997): read telegram [28] => [140307210001S]
    telegramCount++;
        
    DSMRdata = {};
    String    DSMRerror;

    // Parse succesful, print result
    //if (slimmeMeter.parse(&DSMRdata, &DSMRerror, doParseChecksum) )
    if (slimmeMeter.parse(&DSMRdata, &DSMRerror) )
    {
      if (telegramCount > (UINT32_MAX - 10)) 
      {
        delay(1000);
        ESP.reset();
        delay(1000);
      }
      digitalWrite(LED_BUILTIN, LED_OFF);
      if (DSMRdata.identification_present)
      {
        //--- this is a hack! The identification can have a backslash in it
        //--- that will ruin javascript processing :-(
        for(int i=0; i<DSMRdata.identification.length(); i++)
        {
          if (DSMRdata.identification[i] == '\\') DSMRdata.identification[i] = '=';
          yield();
        }
      }
      
      if (DSMRdata.p1_version_be_present)
      {
        DSMRdata.p1_version = DSMRdata.p1_version_be;
        DSMRdata.p1_version_be_present  = false;
        DSMRdata.p1_version_present     = true;
      }
      
      if (!settingSmHasFaseInfo)
      {
        if (DSMRdata.power_delivered_present && !DSMRdata.power_delivered_l1_present)
        {
          DSMRdata.power_delivered_l1 = DSMRdata.power_delivered;
          DSMRdata.power_delivered_l1_present = true;
          DSMRdata.power_delivered_l2_present = true;
          DSMRdata.power_delivered_l3_present = true;
        }
        if (DSMRdata.power_returned_present && !DSMRdata.power_returned_l1_present)
        {
          DSMRdata.power_returned_l1 = DSMRdata.power_returned;
          DSMRdata.power_returned_l1_present = true;
          DSMRdata.power_returned_l2_present = true;
          DSMRdata.power_returned_l3_present = true;
        }
      } // No Fase Info

      if (!DSMRdata.timestamp_present)                        //ezTime
      {                                                       //ezTime
        sprintf(cMsg, "%02d%02d%02d%02d%02d%02dW\0\0"         //ezTime
                        , (year() - 2000), month(), day()     //ezTime
                        , hour(), minute(), second());        //ezTime
        DSMRdata.timestamp         = cMsg;                    //ezTime
        DSMRdata.timestamp_present = true;                    //ezTime
      }                                                       //ezTime

      //-- handle mbus delivered values
      DebugTf("settingMbusNrGas [%d] => ", settingMbusNrGas);
      switch (settingMbusNrGas)
      {
        case 2:   if (DSMRdata.mbus2_device_type == 3)
                  {
                    gasDelivered =  (float)DSMRdata.mbus2_delivered
                                  + (float)DSMRdata.mbus2_delivered_ntc
                                  + (float)DSMRdata.mbus2_delivered_dbl;
                    DSMRdata.mbus2_delivered_present     = true;
                    DSMRdata.mbus2_delivered_ntc_present = false;
                    DSMRdata.mbus2_delivered_dbl_present = false;
                  }
                  break;
        case 3:   if (DSMRdata.mbus3_device_type == 3)
                  {
                    gasDelivered =  (float)DSMRdata.mbus3_delivered
                                  + (float)DSMRdata.mbus3_delivered_ntc
                                  + (float)DSMRdata.mbus3_delivered_dbl;
                    DSMRdata.mbus3_delivered_present     = true;
                    DSMRdata.mbus3_delivered_ntc_present = false;
                    DSMRdata.mbus3_delivered_dbl_present = false;
                  }
                  break;
        case 4:   if (DSMRdata.mbus4_device_type == 3)
                  {
                    gasDelivered =  (float)DSMRdata.mbus4_delivered
                                  + (float)DSMRdata.mbus4_delivered_ntc
                                  + (float)DSMRdata.mbus4_delivered_dbl;
                    DSMRdata.mbus4_delivered_present     = true;
                    DSMRdata.mbus4_delivered_ntc_present = false;
                    DSMRdata.mbus4_delivered_dbl_present = false;
                  }
                  break;
        default:  if (DSMRdata.mbus1_device_type == 3)
                  {
                    Debug("MBus1 devType == 3 =>");
                    gasDelivered =  (float)DSMRdata.mbus1_delivered
                                  + (float)DSMRdata.mbus1_delivered_ntc
                                  + (float)DSMRdata.mbus1_delivered_dbl;
                    DSMRdata.mbus1_delivered_present     = true;
                    DSMRdata.mbus1_delivered_ntc_present = false;
                    DSMRdata.mbus1_delivered_dbl_present = false;
                  }
                  break;
 
      } // switch(settingMbusNrGas)
      Debugf("gasDelivered [%f]\r\n", gasDelivered);
      
      processTelegram();
      if (Verbose2) 
      {
        DebugTln("Show Values:");
        DSMRdata.applyEach(showValues());
      }
          
    } 
    else                  // Parser error, print error
    {
      telegramErrors++;
      #ifdef USE_SYSLOGGER
        sysLog.writef("Parse error\r\n%s\r\n\r\n", DSMRerror.c_str());
      #endif
      DebugTf("Parse error\r\n%s\r\n\r\n", DSMRerror.c_str());
      //--- set DTR to get a new telegram as soon as possible
      slimmeMeter.enable(true);
      slimmeMeter.loop();
    }

    if ( (telegramCount > 25) && (telegramCount % (2100 / (settingTelegramInterval + 1)) == 0) )
    {
      DebugTf("Processed [%d] telegrams ([%d] errors)\r\n", telegramCount, telegramErrors);
      writeToSysLog("Processed [%d] telegrams ([%d] errors)", telegramCount, telegramErrors);
    }
        
  } // if (slimmeMeter.available()) 

} // handleSlimmeMeter()

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
