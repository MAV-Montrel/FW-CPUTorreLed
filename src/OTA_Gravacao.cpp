#include "OTA_Gravacao.h"


#include <WiFiClient.h>

#include <ArduinoOTA.h>

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
const char* host = "esp32";
WebServer server(80);

#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
const char* host = "esp8266";
ESP8266WebServer server(80);
#endif

const char* ssid = "GravadorHR";
const char* password = "hr654321";


 
/*
 * Login page
 */
const char* loginIndex =
  "<html><head>"
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "<style>"
  "body {"
  "  font-family: Arial, sans-serif;"
  "  background-color: #f4f4f4;"
  "  margin: 0;"
  "  display: flex;"
  "  justify-content: center;"
  "  align-items: center;"
  "  height: 100vh;"
  "}"
  ".login-box {"
  "  background-color: #fff;"
  "  border: 1px solid #ccc;"
  "  padding: 20px;"
  "  border-radius: 5px;"
  "  box-shadow: 0px 0px 10px rgba(0, 0, 0, 0.2);"
  "  text-align: center;"
  "}"
  "h1 {"
  "  font-size: 36px;"
  "}"
  "label {"
  "  font-size: 24px;"
  "}"
  "input[type='text'], input[type='password'] {"
  "  width: 100%;"
  "  font-size: 24px;"
  "  padding: 10px;"
  "  margin-bottom: 10px;"
  "  border: 1px solid #ccc;"
  "  border-radius: 5px;"
  "}"
  "button {"
  "  width: 100%;"
  "  font-size: 28px;"
  "  padding: 10px 20px;"
  "  background-color: #4CAF50;"
  "  color: white;"
  "  border: none;"
  "  border-radius: 5px;"
  "  cursor: pointer;"
  "}"
  "</style>"
  "</head>"
  "<body>"
  "<div class='login-box'>"
    "<h1>Login</h1>"
    "<br><br>"
    "<form name='loginForm'>"
      "<label for='userid'>Username:</label><br>"
      "<input type='text' id='userid' name='userid' required><br><br>"
      "<label for='pwd'>Password:</label><br>"
      "<input type='password' id='pwd' name='pwd' required><br><br>"
      "<button type='submit' onclick='check(this.form)'>Login</button>"
    "</form>"
  "</div>"
  "<script>"
    "function check(form) {"
    "  if (form.userid.value === 'admin' && form.pwd.value === 'admin') {"
    "    window.open('/serverIndex');"
    "  } else {"
    "    alert('Error Password or Username');"
    "  }"
    "}"
  "</script>"
  "</body></html>";

/*
  Server Index Page
*/
const char* serverIndex =
   "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "  <meta charset='UTF-8'>"
  "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>"
  "  <title>Firmware Update</title>"
  "  <style>"
  "    body {"
  "      text-align: center;"
  "      margin-top: 20px;"
  "      font-family: Arial, sans-serif;"
  "    }"
  "    h1 {"
  "      font-size: 36px;"
  "      line-height: 1.5;"
  "    }"
  "    label {"
  "      font-size: 24px;"
  "    }"
  "    input[type='file'] {"
  "      width: 100%;"
  "      font-size: 20px;"
  "      margin-top: 10px;"
  "    }"
  "    button[type='submit'] {"
  "      width: 100%;"
  "      font-size: 28px;"
  "      padding: 10px 20px;"
  "      background-color: #4CAF50;"
  "      color: white;"
  "      border: 2px solid #4CAF50;"
  "      border-radius: 5px;"
  "      margin-top: 20px;"
  "      cursor: pointer;"
  "    }"
  "    #prg {"
  "      font-size: 24px;"
  "      margin-top: 20px;"
  "    }"
  "  </style>"
  "</head>"
  "<body>"
  "  <h1>Firmware Update</h1>"
  "  <form method='POST' action='#' enctype='multipart/form-data' id='upload_form'>"
  "    <label for='update'>Select Firmware File:</label><br>"
  "    <input type='file' id='update' name='update' required><br>"
  "    <button type='submit'>Update</button>"
  "  </form>"
  "  <div id='prg'>progress: 0%</div>"
  "  <script src='https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js'></script>"
  "  <script>"
  "    $(document).ready(function() {"
  "      $('form').submit(function(e){"
  "        e.preventDefault();"
  "        var form = $('#upload_form')[0];"
  "        var data = new FormData(form);"
  "        $.ajax({"
  "          url: '/update',"
  "          type: 'POST',"
  "          data: data,"
  "          contentType: false,"
  "          processData: false,"
  "          xhr: function() {"
  "            var xhr = new window.XMLHttpRequest();"
  "            xhr.upload.addEventListener('progress', function(evt) {"
  "              if (evt.lengthComputable) {"
  "                var per = evt.loaded / evt.total;"
  "                $('#prg').html('progress: ' + Math.round(per*100) + '%');"
  "              }"
  "            }, false);"
  "            return xhr;"
  "          },"
  "          success: function(d, s) {"
  "            console.log('success!');"
  "          },"
  "          error: function (a, b, c) {"
  "            console.error('error:', a, b, c);"
  "          }"
  "        });"
  "      });"
  "    });"
  "  </script>"
  "</body>"
  "</html>";


void OTA_init(void)
{
   // Connect to WiFi network
  //WiFi.begin(ssid, password);
  Serial.begin(115200);
  Serial.print("Init OTA");
  delay(200);

 WiFi.softAP(ssid, password);
 Serial.println("modo ap");
 
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
 
  /*use mdns for host name resolution*/
  if (!MDNS.begin(host)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", loginIndex);
  });
  server.on("/serverIndex", HTTP_GET, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/html", serverIndex);
  });
  /*handling uploading firmware file */
  server.on("/update", HTTP_POST, []() {
    server.sendHeader("Connection", "close");
    server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK");
    ESP.restart();
  }, []() {
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      MDNS.addService("http", "tcp", 80);
      Serial.printf("Update: %s\n", upload.filename.c_str());
      if (!Update.begin(1024 * 1024)) {//if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      /* flashing firmware to ESP*/
      MDNS.addService("http", "tcp", 80);
      if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
        Update.printError(Serial);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (Update.end(true)) { //true to set the size to the current progress
        Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);
      } else {
        Update.printError(Serial);
      }
    }
  });
  server.begin();
  MDNS.addService("http", "tcp", 80);
}

void OTA_handleUpload(void)
{
  server.handleClient();
  delay(1);
}