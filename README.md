# DSMRlogger-Next
The Next firmware for the DSMR-logger 

Find out more about the Next firmware [in this blogpost](https://willem.aandewiel.nl/index.php/2020/07/22/dsmr-logger-schrijft-nu-rechtstreeks-in-influxdb-grafana/)

**This is the FORK of the original DSMRloggerAPI from Willem Aandewiel.** 

The intent of this for is to start implementing and extend with new features, fixing bugs and trying to improve the codebase.

With the launch of the Final version of DSMRloggerAPI, version 2.0.1. This completed the last and final update of Willem and released it to the public DSMRlogger, as it complete the REST API.This fork is picking up where Willem finished the development of the REST finished, by fixing bugs, improvement to the codebas and starting to add new features to the firmware.

Willem contributed a large part of his time and effort in creating the open hardware design, making it easy ot get a PCB and components and the firmware in an Open Source way. I plan to extend building features I care about, try to fix bugs and improve code (if even possible). I thank Willem for his contribution to the community and I will keep posting the work as a fork from his original code base. 

The first release candidate of DSMRlogger-Next will contain:
- Direct writting to InfluxDB;
- Fixing DSMR handling so that DSMR spec 4.2 meter will work properly once more;
- Minor improvements.

If you have any idea's or suggestions, just ask for them through the issues of this project on GitHub. Did you want to join me in development, then just do a Pull Request this project and I will consider it.

The original code and release of DSMRloggerAPI can be found here:
[Here](https://willem.aandewiel.nl/index.php/2019/04/09/dsmr-logger-v4-slimme-meter-uitlezer/) and
[here](https://willem.aandewiel.nl/index.php/2020/02/28/restapis-zijn-hip-nieuwe-firmware-voor-de-dsmr-logger/)
you can find information about this project.

Documentation of DSMRloggerAPI can be found [here](https://mrwheel-docs.gitbook.io/dsmrloggerapi/) (work in progress)!

<table>
<tr><th>Versie</th><th>Opmerking</th></tr>
<tr>
   <td valign="top">2.1.0-rc0</td>
   <td>Release Candidate (rc0) of DSMRlogger-Next
   <br>Implementing direct InfluxDB support 
   <br>Bugfix reading DSMR 4.2 correctly (timing issues) 
   <br>Improvement: Send MQTT once per telegram (no repeats)
   </td>
</tr>
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
