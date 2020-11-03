/**************************************************************************
**  Program  : espHelper.h
**  Version  : v2.3.0-rc5
**
**  Supporting the ESP 8266 port to ESP 32 and vise versa 
**
**  Copyright (c) 2020 Robert van den Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************/


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
//=======================================================================        

void esp_reboot()
{ 
  //wait 3 seconds, and then reboot.
  DebugT("Reboot in 3 seconds");
  delay(1000);Debug(".");
  delay(1000);Debug(".");
  delay(1000);Debugln(".");
  //WiFi.forceSleepBegin(); //stop wifi 
  ESP.restart();          //soft reset
  //wdt_reset(); ESP.restart(); while(1)wdt_reset();
  //ESP.reset(); //hard reset
  //restart() doesn't always end execution
  while (1) {
    yield();
  }
}

void esp_reset()
{
    #if defined(ESP8266) 
        ESP.reset();
    #elif defined(ESP32)
        ESP.restart();
    #endif 
}

uint32_t esp_get_chipid()
{
#if defined(ESP8266)
     return ESP.getChipId();
#elif defined(ESP32) 
    return ((uint32_t)ESP.getEfuseMac()); //The chipID is essentially its MAC address (length: 6 bytes) 
#endif
}

uint32_t esp_get_free_block()
{
#if defined(ESP8266)
    return ESP.getMaxFreeBlockSize();
#elif defined(ESP32)
  return ESP.getMaxAllocHeap();
#endif
}
