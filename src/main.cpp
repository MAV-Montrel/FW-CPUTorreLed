#include <Arduino.h>
#include "../lib/OTA/OTA_Gravacao.h"
#include "App_torre.h"

int Tecla = 4;
void setup() {
    Serial.begin(115200);
    pinMode(Tecla, INPUT_PULLUP);
    delay(100);   
    while(digitalRead(Tecla) == 0){
      static int cont = 0;
      if(cont > 1000){
        cont = 0;
        Serial.println("Modo gravação");
        OTA_init();
        while(1){
          OTA_handleUpload(); 
          delay(1);
        }
      }
      cont++;      
      delay(1);
    }
   
  APP_TORRE_init();
}

void loop() {
  APP_TORRE_run();
}
