#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdate.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Update.h>
#endif

 #include <DNSServer.h>
#include <WiFiClient.h>
#include "Index.h"
#include <ArduinoJson.h>

const char* host = "GravadorHR";
const char* ssid = "GravadorHR";
const char* password = "hr654321";

#ifdef ESP8266
ESP8266WebServer server(80);
#elif defined(ESP32)
WebServer server(80);
#endif


String debugMessages = "";
int uploadCounter = 0;
bool loginSuccessful = false;
bool stateLoggin = false;
String debugOutput = "";
bool stateDebug = false;
bool cleanDegub = false;
uint8_t OTA_stateTeste = 0;

void OTA_init(void)
{ 
  WiFi.softAP(ssid, password); 
  IPAddress IP = WiFi.softAPIP();

  delay(1000); // Aguarde 1 segundo
  /*use mdns for host name resolution*/
  if (!MDNS.begin("gavadorhr")) { //http://esp32.local
    while (1) {
      delay(1000);
    }
  }
  //Serial.println("mDNS responder started");
  /*return index page which is stored in serverIndex */
  server.on("/", HTTP_GET, []() {
    //server.sendHeader("Connection", "close");
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.send(200, "text/html", serverIndex);
  });

  server.on("/serverIndex", HTTP_GET, []() {  
        server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        String response = String(serverIndex);       
        server.send(200, "text/html", response);   
  }); 

    server.on("/getDebugOutput", HTTP_GET, []() {
      //if(stateDebug == true){
       // stateDebug = false;
        //Serial.print("otadebug:");
        //Serial.println(debugOutput);
        // Defina o tipo de conteúdo como texto/html
       // server.sendHeader("Content-Type", "text/html");
        server.sendHeader("Content-Type", "text/html");
        server.sendHeader("Cache-Control", "no-cache");
        server.sendHeader("Pragma", "no-cache");
        server.sendHeader("Expires", "-1");
        // Envie o conteúdo de debug (debugOutput) como resposta
        server.send(200, "text/plain", debugOutput);
    //  }
     // else{
       // server.send(200);
     // }
    });

    server.on("/clearDebugData", HTTP_GET, []() {
      String message = "";
      debugOutput = message;
      // Defina o tipo de conteúdo como texto/html
      server.sendHeader("Content-Type", "text/html");
      // Envie o conteúdo de debug (debugOutput) como resposta
      server.send(200, "text/html", debugOutput);
    });

      // Rota para lidar com solicitações PUT em '/testeHardware'
        server.on("/testeHardware", HTTP_PUT, []() {
          // Lê o valor JSON enviado no corpo da solicitação
        String jsonStr = server.arg("plain");

        // Crie um objeto JSON para analisar a string JSON
        StaticJsonDocument<64> jsonDoc; // Ajuste o tamanho conforme necessário
        DeserializationError error = deserializeJson(jsonDoc, jsonStr);

        // Verifique se houve um erro ao analisar o JSON
        if (error) {
         // Serial.print("Erro ao analisar JSON: ");
         // Serial.println(error.c_str());
          server.send(400, "text/plain", "Erro ao analisar JSON");
          return;
        }
        // Verifique se o JSON contém a chave "number"
        if (jsonDoc.containsKey("number")) {
          int intValue = jsonDoc["number"]; // Obtenha o valor "number" do JSON como int

          // Verificar se o valor cabe em um uint8_t (0 a 255)
          if (intValue >= 0 && intValue <= 255) {
            uint8_t uint8Value = (uint8_t)intValue;
            OTA_stateTeste = uint8Value;
           // Serial.print("Seta estado teste: ");
           // Serial.println(OTA_stateTeste);
          } else {
            //Serial.println("Valor fora do intervalo permitido (0-255).");
            server.send(400, "text/plain", "Valor fora do intervalo permitido (0-255)");
          }
        } else {
          //Serial.println("Chave 'number' ausente no JSON.");
          server.send(400, "text/plain", "Chave 'number' ausente no JSON");
        }
        // Envie uma resposta de sucesso
        server.send(200, "text/plain", "Valor recebido com sucesso");
    });
  
 //////==== ESP32 ==========
  // /*handling uploading firmware file */
  // server.on("/update", HTTP_POST, []() {    
  //   //server.sendHeader("Connection", "close");
  //   server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  //   server.send(200, "text/plain", (Update.hasError()) ? "FAIL" : "OK"); 
  //   server.sendContent("<script>limpaHTML();</script>");
  //   server.sendContent("<script>redirectToServerIndex();</script>");
  //   server.send(200);
  //   delay(1000);
  //   ESP.restart();
  // }, []() {
  //   HTTPUpload& upload = server.upload();
  //   if (upload.status == UPLOAD_FILE_START) {
  //     MDNS.addService("http", "tcp", 80);
  //     Serial.printf("Update: %s\n", upload.filename.c_str());
  //     // #ifdef ESP8266
  //     //   uint32_t flashSize = ESP.getFlashChipRealSize();
  //     //   Serial.print("Tamanho da esp:");
  //     //   Serial.println(flashSize);
  //     //   if (!Update.begin(flashSize)) { // Use o tamanho real da memória flash
  //     //     Update.printError(Serial);
  //     //   }
  //     // #elif defined(ESP32)
  //       if (!Update.begin(UPDATE_SIZE_UNKNOWN)) { //start with max available size
  //         Update.printError(Serial);
  //       }
  //     // #endif        
  //   } else if (upload.status == UPLOAD_FILE_WRITE) {
  //     /* flashing firmware to ESP*/   
  //     MDNS.addService("http", "tcp", 80);
  //     if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
  //       Update.printError(Serial);
  //     }   
  //   } else if (upload.status == UPLOAD_FILE_END) {
  //     if (Update.end(true)) { //true to set the size to the current progress
  //       Serial.printf("Update Success: %u\nRebooting...\n", upload.totalSize);         
  //     } else {
  //       Serial.println("Erro achado: ");
  //       Update.printError(Serial);
  //     }
  //   }
  // });

 //======  ESP8266 ================
  // Rota para a atualização do firmware
  server.on("/update", HTTP_POST, []() {  
  Serial.println("Iniciando atualização...");
  server.send(200, "text/plain", "Iniciando atualização...\n");
  }, []() {
  // Verifique se o arquivo de atualização está presente
      HTTPUpload& upload = server.upload();
      if (upload.status == UPLOAD_FILE_START) {
        MDNS.addService("http", "tcp", 80);
        Serial.println("Vai carregar nome do arquivo: ");
        String filename = upload.filename;
        //delay(1000);
        Serial.println("Nome do arquivo recebido: " + filename);
        if (!filename.endsWith(".bin")) {
          Serial.println("Apenas arquivos .bin são suportados!");
          server.send(500, "text/plain", "Apenas arquivos .bin são suportados!");
          return;
        }
        Serial.println("Iniciando gravação do arquivo...");
        if (Update.begin(ESP.getFreeSketchSpace())) {
          Update.runAsync(true);
        } else {
          Serial.println("Falha ao iniciar a atualização!");
          server.send(500, "text/plain", "Falha ao iniciar a atualização!");
        }
      } else if (upload.status == UPLOAD_FILE_WRITE) {
        Serial.println("Gravando arquivo...");       
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
          Serial.println("Falha na gravação do arquivo!");
          server.send(500, "text/plain", "Falha na gravação do arquivo!");
        }
      } else if (upload.status == UPLOAD_FILE_END) {
        Serial.println("Finalizando atualização...");
        if (Update.end(true)) {          
          Serial.println("Atualização concluída, reiniciando...");
          server.send(200, "text/plain", "Atualização concluída, reiniciando...");
          ESP.restart();
        } else {    
          Serial.println("Falha na atualização!");
          server.send(500, "text/plain", "Falha na atualização!");
        }
      }
    });


  server.begin();
  MDNS.addService("http", "tcp", 80);
  
  Serial.println();

}

void OTA_handleUpload(void)
{
  static uint32_t tick = 0;
  static uint32_t LastTick = 0;
  tick = millis();
  if((tick - LastTick) > 1){
    LastTick = tick;     
    server.handleClient(); 
  }
   
}

// Função para enviar mensagens de debug
void OTA_debug(const String& message) {  
  stateDebug = true;  
  if(cleanDegub){
    debugOutput = "";   
  }  
  debugOutput += message + "\n";
}

// Função para limpar a caixa de texto de debug
void OTA_CleanDebug() { 
  cleanDegub = true;
  stateDebug = false;
}

uint8_t OTA_GetStateTest(void){
  return OTA_stateTeste;
}