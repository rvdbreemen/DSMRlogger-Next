
[![Join the chat at https://gitter.im/DSMRlogger-Next/community](https://badges.gitter.im/DSMRlogger-Next/community.svg)](https://gitter.im/DSMRlogger-Next/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

# DSMRlogger-Next
The Next firmware for the DSMR-logger for ESP32 and ESP8266

# Important RC5: Breaking change to MQTT Messages 
With the introduction of Auto Discovery and simplified MQTT messages this also breaks current integrations based on JSON messages over MQTT.  

A couple of things about that:
1. The JSON based MQTT message code is still in the codebase, and can be switch back on.
2. Simplified MQTT messages allow for easy integration with any platform.
3. Auto Discovery will update your Home Assistant configuration. You can remove any existing configuration templates in your setup.

Please be aware of these changes, moving over to RC5 is breaking some stuff. So update your Home Assistant by removing the configuration items. 

# Hardware support
This codebase support the original ESP8266 based DSMR API logger hardware. Starting with RC5 the code is also supporting the ESP32 hardware version that is in development currently. Just select the correct hardware board and compile, it should work fine. 
# Active Development
The active development branch is currently:  
https://github.com/rvdbreemen/DSMRlogger-Next/tree/esp32-merge-next-firmware**

Find out more about the Next firmware [in this blogpost](https://willem.aandewiel.nl/index.php/2020/07/22/dsmr-logger-schrijft-nu-rechtstreeks-in-influxdb-grafana/)

**This is the FORK of the original DSMRloggerAPI from Willem Aandewiel.** 

The intent of this for is to start implementing and extend with new features, fixing bugs and trying to improve the codebase.

With the launch of the Final version of DSMRloggerAPI, version 2.0.1. This completed the last and final update of Willem and released it to the public DSMRlogger, as it complete the REST API.This fork is picking up where Willem finished the development of the REST finished, by fixing bugs, improvement to the codebas and starting to add new features to the firmware.

Willem contributed a large part of his time and effort in creating the open hardware design, making it easy ot get a PCB and components and the firmware in an Open Source way. I plan to extend building features I care about, try to fix bugs and improve code (if even possible). I thank Willem for his contribution to the community and I will keep posting the work as a fork from his original code base. 

The release candidate of DSMRlogger-Next will contain:
- Direct writting to InfluxDB;
- Fixing DSMR handling so that DSMR spec 4.2 meter will work properly once more
- Improved c-style string implementation
- Home Assitant Auto-Discovery through MQTT integration
- Reboot instead of "Bailout" for 8266 (due to memory leak, means more reboots, but is stable)
- Support for WEMOS LOLIN32 (for Rob Roos)
- Minor improvements.

Bughunting help by: Rob Roos

If you have any idea's or suggestions, just ask for them through the issues of this project on GitHub. Did you want to join me in development, then just do a Pull Request this project and I will consider it.

The original code and release of DSMRloggerAPI can be found here:
[Here](https://willem.aandewiel.nl/index.php/2019/04/09/dsmr-logger-v4-slimme-meter-uitlezer/) and
[here](https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/)
you can find information about this project.

Documentation of DSMRloggerAPI can be found [here](https://mrwheel-docs.gitbook.io/dsmrloggerapi/) (work in progress)!

<table>
<tr><th>Versie</th><th>Opmerking</th></tr>
<tr>
   <td valign="top">2.3.0-rc5</td>
   <td>Release Candidate (rc5) of DSMRlogger-Next
   <br>Implemented auto-discovery for Homeassistant and Domoticz
   <br>Removed JSON on MQTT messages (backward compatible with HA integration)
</td>
</tr>
<tr>
   <td valign="top">2.1.4-rc4</td>
   <td>Release Candidate (rc4) of DSMRlogger-Next
   <br>upgraded to latest InfluxDB and modified API for writeOptions
   <br>cleaned up the debug code in ESP32 library
   <br>changed processTelegram so detection is now based on timestamp, not on internal clock
   <br>thanks to Rob Roos for help with testing, and validating 8266 compilation
   </td>
</tr>
<tr>
   <td valign="top">2.1.3-rc3</td>
   <td>Release Candidate (rc3) of DSMRlogger-Next
   <br>removed ezTime implementation due to fundamental problem with epoch
   <br>added isdsmrDST() to detect summer/winter time for correct influxDB setting
   <br>Implemented HomeAssistant Autodiscovery for MQTT
   </td>
</tr>
<tr>
   <td valign="top">2.1.2-rc2</td>
   <td>Release Candidate (rc2) of DSMRlogger-Next
   <br>Back to InfluxDB client library
   <br>ezTime implementation done right (removed it)
   <br>No more bailout with low heap, this broke MQTT and Mindergas
   <br>Fixes corrupt records in ring-files (Thanks to Rob Roos)
   </td>
</tr>
<tr>
   <td valign="top">2.1.1-rc1</td>
   <td>Release Candidate (rc1) of DSMRlogger-Next
   <br>Implementing my own InfluxDB writer logic 
   <br>Lots of String processing changed in to Cstyle char processing
   </td>
</tr>
<tr>
   <td valign="top">2.1.0-rc0</td>
   <td>Release Candidate (rc0) of DSMRlogger-Next
   <br>Implementing direct InfluxDB support 
   <br>Bugfix reading DSMR 4.2 correctly (timing issues) 
   <br>Improvement: Send MQTT once per telegram (no repeats)
   <td valign="top">3.0.Beta</td>
   <td>BETA release! Don't use for production!
   <br>Trying to make the firmware more "settings" driven.
   <br>In the settings tab you can now:
   <ul>
     <li>Choose the DSMR version to use (Pré 40, 4+ or Belgium 5+). No more need for 
         compiler switches and different firmware versions (one fits all).
     </li>
     <li>Choose on which MBus channel the Gas meter is connected and what type
         of format it uses to present meter values (Temp. Corrected or not, value
         on second record).  </li>
   </ul>
   Switching to LittleFS (SPIFFS is depreciated in the ESP8266 core).
   <br>New File System Manager (FSmanager) to replace FSexplorer.
   </td>
</tr>
<tr>
   <td valign="top">2.0.1</td>
   <td>First Final Release
   <br>Implementing DSMRloggerWS actual api (for backwards compatibility)
   <br>More robust way to write hourly data to the RING-files
   <br>Bugfix PRE40 gasmeter reading
   <br>Remove validity check on meterstanden editor
   <br>Better FieldName translation
   <br>Bugfix mindergas processing
   </td>
</tr>
<tr>
   <td valign="top">1.2.1</td>
   <td>Third Official Release
      <br>Instelling SM_HAS_NO_FASE_INFO nu via settings
      <br>Selectie OLED scherm via settings
      <br>Mogelijkheid om het oled scherm 180* te flippen via settings
      <br>Check op volgordelijkheid Uren (in de GUI)
      <br>macaddress in /api/v1/dev/info (Phyxion)
      <br>Bailout some functions on low heap
      <br>Simplification and better tab restAPIs
      <br>Editer Maanden tabel verbetert  (maar nog steeds lastig te gebruiken)
      <br>Betere test of er op github nieuwe firmware beschikbaar is
      <br>bugfix prevent GUI firering multiple restAPI call's
      <br>The Leading Edge type GUI now completely from github. Set
      'index page' to "DSMRindexEDGE.html" to make use of
      the latest development (but be aware, there might be bugs)
   </td>
</tr>
<tr>
   <td valign="top">1.1.0</td>
   <td>Second Official Release
      <br>Introduction ESP_SysLogger
      <br>GUI is now even more a Single Page Application (SPA)
      <br>Better coping with WiFi losses (continue operation and try to reconnect)
      <br>Restructure slimmemeter.loop() etc.
      <br>You can find pré compiled binaries at <b>code</b> -> <b><i>releases</i></b>
   </td>
</tr>
<tr>
   <td valign="top">1.0.1</td>
   <td>First official release
      <br>You can find pré compiled binaries at <b>code</b> -> <b><i>releases</i></b>
   </td>
</tr>
<tr>
   <td valign="top">0.3.5 RC3</td>
   <td>Third Release Candidate
   </td>
</tr>
</table>
