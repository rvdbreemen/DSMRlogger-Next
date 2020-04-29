/* 
***************************************************************************  
**  Program  : ESP32ModUpdateServer.h
**  Version  : v0.0.1
**
**  Modified by 2020 Robert van den Breemen
**
**  TERMS OF USE: MIT License. See bottom of file.                                                            
**  
**  This is a hack based on sample code, the HTTP8266UpdateServer 
**  and Willem Aandewiel modfication of the orginal.
**
**  API is compatible with the Willem's modification.
** 
***************************************************************************      
*/

#ifndef ESP32_HTTP_UPDATE_SERVER_H
#define ESP32_HTTP_UPDATE_SERVER_H

/*
  B
*/

#include <WebServer.h>
#include <Update.h>

class ESP32HTTPUpdateServer
{
private:
  WebServer* _server;

  String _username;
  String _password;
  bool _serialDebugging;
  const char *_serverIndex;
  const char *_serverSuccess;

public:
  ESP32HTTPUpdateServer(bool serialDebugging = false)
  {
    _server = NULL;
    _username = "";
    _password = "";
  }

  void setIndexPage(const char *indexPage)
  {
	_serverIndex = indexPage;
  }

  void setSuccessPage(const char *successPage)
  {
	_serverSuccess = successPage;
  }  

  void setup(WebServer* server, const char* path = "/update", const char* username = "", const char* password = "")
  {
    _server = server;
    _username = username;
    _password = password;

    // Get of the index handling
    _server->on(path, HTTP_GET, [&]() {
      // Force authentication if a user and password are defined
      if (_username.length() > 0 && _password.length() > 0 && !_server->authenticate(_username.c_str(), _password.c_str()))
        return _server->requestAuthentication();

      _server->sendHeader("Connection", "close");
      _server->send(200, "text/html", _serverIndex);
    });

    // Post of the file handling
    _server->on(path, HTTP_POST, [&]() {
      _server->client().setNoDelay(true);
      _server->send_P(200, "text/html", (Update.hasError()) ? "FAIL" : _serverSuccess);
      delay(100);
      _server->client().stop();
      ESP.restart();
    }, [&]() {
      HTTPUpload& upload = _server->upload();

      if (upload.status == UPLOAD_FILE_START) 
      {
        // Check if we are authenticated
        if (!(_username.length() == 0 || _password.length() == 0 || _server->authenticate(_username.c_str(), _password.c_str())))
        {
          if (_serialDebugging)
            Serial.printf("Unauthenticated Update\n");

          return;
        }

        // Debugging message for upload start
        if (_serialDebugging) 
        {
          Serial.setDebugOutput(true);
          Serial.printf("Update: %s\n", upload.filename.c_str());
        }

        // Starting update
        bool error = Update.begin(UPDATE_SIZE_UNKNOWN);
        if (_serialDebugging && error)
          Update.printError(Serial);
      }
      else if (upload.status == UPLOAD_FILE_WRITE) 
      {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize && _serialDebugging) 
          Update.printError(Serial);
      }
      else if (upload.status == UPLOAD_FILE_END) 
      {
        if (Update.end(true) && _serialDebugging)
          Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
        else if(_serialDebugging)
          Update.printError(Serial);

        if(_serialDebugging)
          Serial.setDebugOutput(false);
      }
      else if(_serialDebugging)
        Serial.printf("Update Failed Unexpectedly (likely broken connection): status=%d\n", upload.status);
    });

    _server->begin();
  }
};

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
