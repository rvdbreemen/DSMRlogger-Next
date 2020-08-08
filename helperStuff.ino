/* 
***************************************************************************  
**  Program  : helperStuff, part of DSMRlogger-Next
**  Version  : v2.1.1-rc1
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

//===========================================================================================
boolean isValidIP(IPAddress ip)
{
 /* Works as follows:
   *  example: 
   *  127.0.0.1 
   *   1 => 127||0||0||1 = 128>0 = true 
   *   2 => !(false || false) = true
   *   3 => !(false || false || false || false ) = true
   *   4 => !(true && true && true && true) = false
   *   5 => !(false) = true
   *   true && true & true && false && true = false ==> correct, this is an invalid addres
   *   
   *   0.0.0.0
   *   1 => 0||0||0||0 = 0>0 = false 
   *   2 => !(true || true) = false
   *   3 => !(false || false || false || false) = true
   *   4 => !(true && true && true && tfalse) = true
   *   5 => !(false) = true
   *   false && false && true && true && true = false ==> correct, this is an invalid addres
   *   
   *   192.168.0.1 
   *   1 => 192||168||0||1 =233>0 = true 
   *   2 => !(false || false) = true
   *   3 +> !(false || false || false || false) = true
   *   4 => !(false && false && true && true) = true
   *   5 => !(false) = true
   *   true & true & true && true && true = true ==> correct, this is a valid address
   *   
   *   255.255.255.255
   *   1 => 255||255||255||255 =255>0 = true 
   *   2 => !(false || false ) = true
   *   3 +> !(true || true || true || true) = false
   *   4 => !(false && false && false && false) = true
   *   5 => !(true) = false
   *   true && true && false && true && false = false ==> correct, this is an invalid address
   *   
   *   0.123.12.1       => true && false && true && true && true = false  ==> correct, this is an invalid address 
   *   10.0.0.0         => true && false && true && true && true = false  ==> correct, this is an invalid address 
   *   10.255.0.1       => true && true && false && true && true = false  ==> correct, this is an invalid address 
   *   150.150.255.150  => true && true && false && true && true = false  ==> correct, this is an invalid address 
   *   
   *   123.21.1.99      => true && true && true && true && true = true    ==> correct, this is annvalid address 
   *   1.1.1.1          => true && true && true && true && true = true    ==> correct, this is annvalid address 
   *   
   *   Some references on valid ip addresses: 
   *   - https://www.quora.com/How-do-you-identify-an-invalid-IP-address
   *   
   */
  boolean _isValidIP = false;
  _isValidIP = ((ip[0] || ip[1] || ip[2] || ip[3])>0);                             // if any bits are set, then it is not 0.0.0.0
  _isValidIP &= !((ip[0]==0) || (ip[3]==0));                                       // if either the first or last is a 0, then it is invalid
  _isValidIP &= !((ip[0]==255) || (ip[1]==255) || (ip[2]==255) || (ip[3]==255)) ;  // if any of the octets is 255, then it is invalid  
  _isValidIP &= !(ip[0]==127 && ip[1]==0 && ip[2]==0 && ip[3]==1);                 // if not 127.0.0.0 then it might be valid
  _isValidIP &= !(ip[0]>=224);                                                     // if ip[0] >=224 then reserved space  
  
  DebugTf( "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  if (_isValidIP) 
    Debugln(F(" = Valid IP")); 
  else 
    Debugln(F(" = Invalid IP!"));
    
  return _isValidIP;
  
} //  isValidIP()


//===========================================================================================
bool isNumericp(const char *timeStamp, int8_t len)
{
  for (int i=0; (i<len && i<12);i++)
  {
    if (timeStamp[i] < '0' || timeStamp[i] > '9')
    {
      return false;
    }
  }
  return true;
  
} // isNumericp()


//===========================================================================================

int8_t splitString(String inStrng, char delimiter, String wOut[], uint8_t maxWords) 
{
  int16_t inxS = 0, inxE = 0, wordCount = 0;
  
    inStrng.trim();
    while(inxE < inStrng.length() && wordCount < maxWords) 
    {
      inxE  = inStrng.indexOf(delimiter, inxS);         //finds location of first ,
      wOut[wordCount] = inStrng.substring(inxS, inxE);  //captures first data String
      wOut[wordCount].trim();
      //DebugTf("[%d] => [%c] @[%d] found[%s]\r\n", wordCount, delimiter, inxE, wOut[wordCount].c_str());
      inxS = inxE;
      inxS++;
      wordCount++;
    }
    // zero rest of the words
    for(int i=wordCount; i< maxWords; i++)
    {
      wOut[wordCount][0] = 0;
    }
    // if not whole string processed place rest in last word
    if (inxS < inStrng.length()) 
    {
      wOut[maxWords-1] = inStrng.substring(inxS, inStrng.length());  // store rest of String      
    }

    return wordCount;
    
} // splitString()


//===========================================================================================
String upTime() 
{
  char    calcUptime[20];

  snprintf(calcUptime, sizeof(calcUptime), "%d(d)-%02d:%02d(H:m)"
                                          , int((upTimeSeconds / (60 * 60 * 24)) % 365)
                                          , int((upTimeSeconds / (60 * 60)) % 24)
                                          , int((upTimeSeconds / (60)) % 60));

  return calcUptime;

} // upTime()


//===========================================================================================

//taken from influxdbclient.cpp == escapeJSONString
static String escapeJSONString(String &value) {
    String ret;
    int d = 0;
    int i,from = 0;
    while((i = value.indexOf('"',from)) > -1) {
        d++;
        if(i == value.length()-1) {
            break;
        }
        from = i+1;
    }
    ret.reserve(value.length()+d); //most probably we will escape just double quotes
    for (char c: value)
    {
        switch (c)
        {
            case '"': ret += "\\\""; break;
            case '\\': ret += "\\\\"; break;
            case '\b': ret += "\\b"; break;
            case '\f': ret += "\\f"; break;
            case '\n': ret += "\\n"; break;
            case '\r': ret += "\\r"; break;
            case '\t': ret += "\\t"; break;
            default:
                if ('\x00' <= c && c <= '\x1f') {
                    ret += "\\u";
                    char buf[3 + 8 * sizeof(unsigned int)];
                    sprintf(buf,  "\\u%04u", c);
                    ret += buf;
                } else {
                    ret += c;
                }
        }
    }
    return ret;
}

String getResetReason()
{
  String _reason="";

#if defined(ESP8266) 
  _reason = ESP.getResetReason();
#elif defined(ESP32)
  RESET_REASON _reset_reason = rtc_get_reset_reason(0);//CPU ID = 0
  switch ( _reset_reason )
  {
    case POWERON_RESET          : _reason = "0: Vbat power on reset";break;
    case SW_RESET               : _reason = "0: Software reset digital core";break;
    case OWDT_RESET             : _reason = "0: Legacy watch dog reset digital core";break;
    case DEEPSLEEP_RESET        : _reason = "0: Deep Sleep reset digital core";break;
    case SDIO_RESET             : _reason = "0: Reset by SLC module, reset digital core";break;
    case TG0WDT_SYS_RESET       : _reason = "0: Timer Group0 Watch dog reset digital core";break;
    case TG1WDT_SYS_RESET       : _reason = "0: Timer Group1 Watch dog reset digital core";break;
    case RTCWDT_SYS_RESET       : _reason = "0: RTC Watch dog Reset digital core";break;
    case INTRUSION_RESET        : _reason = "0: Instrusion tested to reset CPU";break;
    case TGWDT_CPU_RESET        : _reason = "0: Time Group reset CPU";break;
    case SW_CPU_RESET           : _reason = "0: Software reset CPU";break;
    case RTCWDT_CPU_RESET       : _reason = "0: RTC Watch dog Reset CPU";break;
    case EXT_CPU_RESET          : _reason = "0: for APP CPU, reseted by PRO CPU";break;
    case RTCWDT_BROWN_OUT_RESET : _reason = "0: Reset when the vdd voltage is not stable";break;
    case RTCWDT_RTC_RESET       : _reason = "0: RTC Watch dog reset digital core and rtc module";break;
    default                     : _reason = "0: NO_MEAN";
  }
  _reset_reason = rtc_get_reset_reason(1);//CPU ID = 1
  _reason += " - ";
  switch ( _reset_reason )
  {
    case POWERON_RESET          : _reason += "1: Vbat power on reset";break;
    case SW_RESET               : _reason += "1: Software reset digital core";break;
    case OWDT_RESET             : _reason += "1: Legacy watch dog reset digital core";break;
    case DEEPSLEEP_RESET        : _reason += "1: Deep Sleep reset digital core";break;
    case SDIO_RESET             : _reason += "1: Reset by SLC module, reset digital core";break;
    case TG0WDT_SYS_RESET       : _reason += "1: Timer Group0 Watch dog reset digital core";break;
    case TG1WDT_SYS_RESET       : _reason += "1: Timer Group1 Watch dog reset digital core";break;
    case RTCWDT_SYS_RESET       : _reason += "1: RTC Watch dog Reset digital core";break;
    case INTRUSION_RESET        : _reason += "1: Instrusion tested to reset CPU";break;
    case TGWDT_CPU_RESET        : _reason += "1: Time Group reset CPU";break;
    case SW_CPU_RESET           : _reason += "1: Software reset CPU";break;
    case RTCWDT_CPU_RESET       : _reason += "1: RTC Watch dog Reset CPU";break;
    case EXT_CPU_RESET          : _reason += "1: for APP CPU, reseted by PRO CPU";break;
    case RTCWDT_BROWN_OUT_RESET : _reason += "1: Reset when the vdd voltage is not stable";break;
    case RTCWDT_RTC_RESET       : _reason += "1: RTC Watch dog reset digital core and rtc module";break;
    default                     : _reason += "1: NO_MEAN";
  }

#else
  _reason = "Not an ESP8266 / ESP 32 - aka unknown architecture";
#endif

  return _reason;
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
