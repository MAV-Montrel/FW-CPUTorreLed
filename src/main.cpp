#include <Arduino.h>
#include "OTA_Gravacao.h"
#include "SK6812Driver.h"
#include "SK6812_Driver2.h"

#define NUM_LEDS 1
#define DATA_PIN 16
int Tecla = 4;

SK6812Driver leds(NUM_LEDS, DATA_PIN);

// Crie uma instância da classe SK6812
SK6812 ledStrip(NUM_LEDS);

void setup() {
    Serial.begin(115200);
    pinMode(Tecla, INPUT_PULLUP);

   // ledStrip.set_output(6);  // Substitua 6 pelo pino que você está usando

    leds.begin();
}

void loop() {
 
   leds.setPixelColor(0,10,0,0,0);

   leds.show();

  delay(2000);  // Aguarde 1 segundo
}
