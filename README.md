# DSMRloggerAPI
Firmware for the DSMR-logger using only API call's

[Here](https://willem.aandewiel.nl/index.php/2019/04/09/dsmr-logger-v4-slimme-meter-uitlezer/) and
[here](https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/)
you can find information about this project.

Documentation can be found [here](https://mrwheel-docs.gitbook.io/dsmrloggerapi/) (in progress)!

<table>
<tr><th>Versie</th><th>Opmerking</th></tr>
<tr>
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
