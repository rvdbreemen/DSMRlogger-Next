/*
***************************************************************************  
**  Program  : timeStuff, part of DSMRlogger-Next
**  Version  : v2.1.2-rc2
**
**  Copyright (c) 2020 Willem Aandewiel
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/


//===========================================================================================
String buildDateTimeString(const char* timeStamp, int len) 
{
  String tmpTS = String(timeStamp);
  String DateTime = "";
  if (len < 12) return String(timeStamp);
  DateTime   = "20" + tmpTS.substring(0, 2);    // YY
  DateTime  += "-"  + tmpTS.substring(2, 4);    // MM
  DateTime  += "-"  + tmpTS.substring(4, 6);    // DD
  DateTime  += " "  + tmpTS.substring(6, 8);    // HH
  DateTime  += ":"  + tmpTS.substring(8, 10);   // MM
  DateTime  += ":"  + tmpTS.substring(10, 12);  // SS
  return DateTime;
    
} // buildDateTimeString()

//===========================================================================================
void epochToTimestamp(time_t t, char *ts, int8_t len) 
{
  if (len < 12) {
    strncpy(ts, "Error", sizeof(ts));
    return;
  }
  //------------yy  mm  dd  hh  mm  ss
  sprintf(ts, "%02d%02d%02d%02d%02d%02d", year(t)-2000, month(t), day(t)
                                        , hour(t), minute(t), second(t));
                                               
  if (Verbose2) DebugTf("epochToTimestamp() => [%s]\r\n", ts);
  
} // epochToTimestamp()

//===========================================================================================
int8_t SecondFromTimestamp(const char *timeStamp) 
{
  char aSS[4] = "";
  // 0123456789ab
  // YYMMDDHHmmss SS = 4-5
  
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp);
  strncpy(aSS, timeStamp+10,  2);
  //strCopy(aSS, 4, timeStamp, 10, 11);
  if (Verbose2) Debugf("aSe[%s], aSe[%02d]\r\n", aSS, String(aSS).toInt()); 
  return String(aSS).toInt();
    
} // SecondFromTimestamp()

//===========================================================================================
int8_t MinuteFromTimestamp(const char *timeStamp) 
{
  char aMM[4] = "";
  // 0123456789ab
  // YYMMDDHHmmss MM = 8-9
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp);
  strncpy(aMM, timeStamp+8,  2);
  //strCopy(aMM, 4, timeStamp, 8, 9);
  if (Verbose2) Debugf("aMi[%s], nMi[%02d]\r\n", aMM, String(aMM).toInt()); 
  return String(aMM).toInt();
    
} // MinuteFromTimestamp()

//===========================================================================================
int8_t HourFromTimestamp(const char *timeStamp) 
{
  char aHH[4] = "";
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp); // YYMMDDHHmmss HH = 5-6
  strncpy(aHH, timeStamp+6,  2);
  //strCopy(aHH, 4, timeStamp, 6, 7);
  if (Verbose2) Debugf("aHo[%s], nHo[%02d]\r\n", aHH, String(aHH).toInt()); 
  return String(aHH).toInt();
    
} // HourFromTimestamp()

//===========================================================================================
int8_t DayFromTimestamp(const char *timeStamp) 
{
  char aDD[4] = "";
  // 0123456789ab
  // YYMMDDHHmmss DD = 4-5
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp);
  strncpy(aDD, timeStamp+4,  2);
  //strCopy(aDD, 4, timeStamp, 4, 5);
  if (Verbose2) Debugf("aDD[%s], nDD[%02d]\r\n", aDD, String(aDD).toInt()); 
  return String(aDD).toInt();
    
} // DayFromTimestamp()

//===========================================================================================
int8_t MonthFromTimestamp(const char *timeStamp) 
{
  char aMM[4] = "";
  // 0123456789ab
  // YYMMDDHHmmss MM = 2-3
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp);
  strncpy(aMM, timeStamp+2,  2);
  //strCopy(aMM, 4, timeStamp, 2, 3);
  if (Verbose2) Debugf("aMM[%s], nMM[%02d]\r\n", aMM, String(aMM).toInt()); 
  return String(aMM).toInt();
    
} // MonthFromTimestamp()

//===========================================================================================
int8_t YearFromTimestamp(const char *timeStamp) 
{
  char aYY[4] = "";
  // 0123456789ab
  // YYMMDDHHmmss YY = 0-1
  
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp);
  strncpy(aYY, timeStamp+0,  2);
  //strCopy(aYY, 4, timeStamp, 0, 1);
  if (Verbose2) Debugf("aYY[%s], nYY[%02d]\r\n", aYY, String(aYY).toInt()); 
  return String(aYY).toInt();
    
} // YearFromTimestamp()

//===========================================================================================
int32_t HoursKeyTimestamp(const char *timeStamp) 
{
  char aHK[10] = "";
  // 0123456789ab
  // YYMMDDHHmmss YY = 0-1
  
  if (Verbose2) DebugTf("timeStamp[%s] => ", timeStamp);
  strncpy(aHK, timeStamp+0,  8);
  //strCopy(aHK, 4, timeStamp, 0, 7);
  if (Verbose2) Debugf("aHK[%s], nHK[%02d]\r\n", aHK, String(aHK).toInt()); 
  //return timeStamp.substring(0, 8).toInt();
  return String(aHK).toInt();
    
} // HourFromTimestamp()

//===========================================================================================
// calculate epoch from timeStamp
// if syncTime is true, set system time to calculated epoch-time
time_t epoch(const char *timeStamp, int8_t len, bool syncTime) 
{
  char fullTimeStamp[16] = "";

  strlcat(fullTimeStamp, timeStamp, sizeof(fullTimeStamp));
  if (Verbose2) DebugTf("epoch(%s) strlen([%d])\r\n", fullTimeStamp, strlen(fullTimeStamp));  
  switch(strlen(fullTimeStamp)) {
    case  4:  //--- timeStamp is YYMM
              strlcat(fullTimeStamp, "01010101X", sizeof(fullTimeStamp));
              break;
    case  6:  //--- timeStamp is YYMMDD
              strlcat(fullTimeStamp, "010101X", sizeof(fullTimeStamp));
              break;
    case  8:  //--- timeStamp is YYMMDDHH
              strlcat(fullTimeStamp, "0101X", sizeof(fullTimeStamp));
              break;
    case  10:  //--- timeStamp is YYMMDDHHMM
              strlcat(fullTimeStamp, "01X", sizeof(fullTimeStamp));
              break;
    case  12:  //--- timeStamp is YYMMDDHHMMSS
              strlcat(fullTimeStamp,  "X", sizeof(fullTimeStamp));
              break;
    //default:  return now();
  }
  
  if (strlen(fullTimeStamp) < 13) return now();
  
  if (Verbose2) DebugTf("DateTime: [%02d]-[%02d]-[%02d] [%02d]:[%02d]:[%02d]\r\n"
                                                                 ,DayFromTimestamp(timeStamp)
                                                                 ,MonthFromTimestamp(timeStamp)
                                                                 ,YearFromTimestamp(timeStamp)
                                                                 ,HourFromTimestamp(timeStamp)
                                                                 ,MinuteFromTimestamp(timeStamp)
                                                                 ,0
                       );
   
 
  time_t nT;
  time_t savEpoch = now();
  
  setTime(HourFromTimestamp(fullTimeStamp)
         ,MinuteFromTimestamp(fullTimeStamp)
         ,SecondFromTimestamp(fullTimeStamp)
         ,DayFromTimestamp(fullTimeStamp)
         ,MonthFromTimestamp(fullTimeStamp)
         ,YearFromTimestamp(fullTimeStamp));

  
  nT = now();

  syncTime=false;  //do never sync time
  if (!syncTime)
  {
    setTime(savEpoch);
  }

  return nT;

} // epoch()


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
