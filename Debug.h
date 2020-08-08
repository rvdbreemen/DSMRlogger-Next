/* 
***************************************************************************  
**  Program  : Debug.h, part of DSMRlogger-Next
**  Version  : v2.1.1-rc1
**
**  Copyright (c) 2020 Willem Aandewiel
**  Met dank aan Erik
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
***************************************************************************      
*/

#ifndef DEBUG_H
#define DEBUG_H 

// Setup the debug port during setup phase
#define DEBUG_PORT Serial

//DEBUG_ESP_PORT setup means the Core ESP Arduino can also output debug info
//To use this: Tools -> Core Debug Level --> <none, error, warning, info, debug, verbose>
#define DEBUG_ESP_PORT DEBUG_PORT

#ifdef DEBUG_ESP_PORT
  #define DEBUG_MSG(...) DEBUG_ESP_PORT.printf( __VA_ARGS__ )
  //#define DEBUG_MSG(...) Debugf( __VA_ARGS__ )
  #else
  #define DEBUG_MSG(...)
#endif

/*---- start macro's ------------------------------------------------------------------*/

#define Debug(...)      ({ Serial.print(__VA_ARGS__);         \
                           TelnetStream.print(__VA_ARGS__);   \
                        })                       
#define Debugln(...)    ({ Serial.println(__VA_ARGS__);       \
                           TelnetStream.println(__VA_ARGS__); \
                        })                        
#define Debugf(...)     ({ Serial.printf(__VA_ARGS__);        \
                           TelnetStream.printf(__VA_ARGS__);  \
                        })
#define DebugFlush()    ({ Serial.flush(); \
                           TelnetStream.flush(); \
                        })


#define DebugT(...)     ({ _debugBOL(__FUNCTION__, __LINE__);     \
                           Debug(__VA_ARGS__);                    \
                        })
#define DebugTln(...)   ({ _debugBOL(__FUNCTION__, __LINE__);     \
                           Debugln(__VA_ARGS__);                  \
                        })
#define DebugTf(...)    ({ _debugBOL(__FUNCTION__, __LINE__);     \
                           Debugf(__VA_ARGS__);                   \
                        })

/*---- einde macro's ------------------------------------------------------------------*/

// needs #include <TelnetStream.h>       // Version 0.0.1 - https://github.com/jandrassy/TelnetStream

char _bol[128];
void _debugBOL(const char *fn, int line)
{
   
  snprintf(_bol, sizeof(_bol), "[%02d:%02d:%02d][%7u|%6u] %-12.12s(%4d): ", \
                hour(), minute(), second(), \
                ESP.getFreeHeap(), ESP_GET_FREE_BLOCK(),\
                fn, line);
                 
  DEBUG_PORT.print (_bol);
  TelnetStream.print(_bol);
}

#endif // DEBUG_H 
