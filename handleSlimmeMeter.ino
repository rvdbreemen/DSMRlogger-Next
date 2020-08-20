/*
***************************************************************************  
**  Program  : handleSlimmeMeter - part of DSMRlogger-Next
**  Version  : v2.1.2-rc2
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/  

void initSlimmermeter()
{
#if defined( USE_REQUEST_PIN ) && !defined( HAS_NO_SLIMMEMETER )
  #if defined(ESP8266) 
    #ifdef USE_PRE40_PROTOCOL                                                         //PRE40
      SM_SERIAL.begin(9600, SERIAL_7E1);                                                 //PRE40
    #else   // not use_dsmr_30                                                        //PRE40
      SM_SERIAL.begin(115200, SERIAL_8N1);
    #endif  // use_dsmr_30
    DebugTf("Swapping serial port to Smart Meter, debug output will continue on telnet\r\n");
    DebugFlush();
    SM_SERIAL.swap();      // swap to SmartMeter
  #elif defined(ESP32)
    DebugTf("Serialport set to (RX,TX) (%d/%d)\r\n", RXD2, TXD2 );
    #ifdef USE_PRE40_PROTOCOL                                                         //PRE40
      SM_SERIAL.begin( 9600, SERIAL_7E1, RXD2, -1, true );                            //PRE40
    #else   // DSMR 4.x & 5.x
      SM_SERIAL.begin( 115200, SERIAL_8N1, RXD2, -1, true );
    #endif  // use_dsmr_30
  #endif

  #ifdef DTR_ENABLE
    pinMode(DTR_ENABLE, OUTPUT);
  #endif
#endif // USE_REQUEST_PIN && !HAS_NO_SLIMMEMETER 
}

#ifndef HAS_NO_SLIMMEMETER
//==================================================================================
void handleSlimmemeter()
{
  //DebugTf("showRaw (%s)\r\n", showRaw ?"true":"false");
  if (showRaw) {
    //-- process telegrams in raw in mode
    processSlimmemeterRaw();
  } 
  else
  {
    processSlimmemeter();
  } 

} // handleSlimmemeter()

//==================================================================================

void tiggerNextTelegram()
{
    if (Verbose1|| Verbose2) Debugln("Enable DTR, get that telegram");
    // //-- enable DTR to read a telegram from the Slimme Meter
    slimmeMeter.enable(true); 
} // tiggerNextTelegram()

//==================================================================================
void processSlimmemeterRaw()
{
  char    tlgrm[1200] = "";
   
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
    oled_Print_Msg(0, "<DSMRlogger-Next>", 0);
    oled_Print_Msg(1, "-------------------------",0);
    oled_Print_Msg(2, "Raw Format",0);
    snprintf(cMsg, sizeof(cMsg), "Raw Count %4d", showRawCount);
    oled_Print_Msg(3, cMsg, 0);
  }

  slimmeMeter.enable(true);
  SM_SERIAL.setTimeout(5000);  // 5 seconds must be enough ..
  memset(tlgrm, 0, sizeof(tlgrm));
  int l = 0;
  // The terminator character is discarded from the serial buffer.
  l = SM_SERIAL.readBytesUntil('/', tlgrm, sizeof(tlgrm));
  // now read from '/' to '!'
  // The terminator character is discarded from the serial buffer.
  l = SM_SERIAL.readBytesUntil('!', tlgrm, sizeof(tlgrm));
  SM_SERIAL.setTimeout(1000);  // seems to be the default ..
//  DebugTf("read [%d] bytes\r\n", l);
  if (l == 0) 
  {
    DebugTln(F("RawMode: Timerout - no telegram received within 5 seconds"));
    return;
  }

  tlgrm[l++] = '!';
#if !defined( USE_PRE40_PROTOCOL )
  // next 6 bytes are "<CRC>\r\n"
  for (int i=0; ( i<6 && (i<(sizeof(tlgrm)-7)) ); i++)
  {
    tlgrm[l++] = (char)SM_SERIAL.read();
  }
#else
  tlgrm[l++]    = '\r';
  tlgrm[l++]    = '\n';
#endif
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
  slimmeMeter.loop();
  if (slimmeMeter.available()) 
  {
    DebugTf("telegramCount=[%d] telegramErrors=[%d]\r\n", telegramCount, telegramErrors);
    Debugln(F("\r\n[Time----][FreeHea| Frags| mBlck] Function----(line):\r"));
    //  Voorbeeld: [21:00:11][   9880|     9|  8960] loop        ( 997): read telegram [28] => [140307210001S]
    // Debugln(F("\r\n[Time----][FreeHea| mBlck] Function----(line):\r"));
    //     Voorbeeld: [21:00:11][   9880|  8960] loop        ( 997): read telegram [28] => [140307210001S]
    telegramCount++;
        
    DSMRdata = {};
    String    DSMRerror;
        
    if (slimmeMeter.parse(&DSMRdata, &DSMRerror))   // Parse succesful, print result
    {
      if (telegramCount > (UINT32_MAX - 10)) 
      {
        delay(1000);
        ESP_RESET();
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

#ifdef USE_NTP_TIME
      if (!DSMRdata.timestamp_present)                        //USE_NTP
      {                                                       //USE_NTP
        sprintf(cMsg, "%02d%02d%02d%02d%02d%02dW\0\0"         //USE_NTP
                        , (year() - 2000), month(), day()     //USE_NTP
                        , hour(), minute(), second());        //USE_NTP
        DSMRdata.timestamp         = cMsg;                    //USE_NTP
        DSMRdata.timestamp_present = true;                    //USE_NTP
      }                                                       //USE_NTP
#endif

      processTelegram();
      if (Verbose2) 
      {
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
  
} // processSlimmeMeter()

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
