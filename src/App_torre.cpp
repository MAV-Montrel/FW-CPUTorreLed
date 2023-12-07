#include "App_torre.h"
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include <EEPROM.h>



#define NUM_LEDS 54     // Número total de LEDs na torre (9 LEDs por lado x 6 lados)
#define NUM_SIDES 6     // Número de lados da torre
#define LEDS_PER_SIDE 9 // Número de LEDs por lado
#define LEDS_PER_PART 3 // Número de LEDs por parte

#define timePulso 500

const char IN1 = 12;
const char IN2 = 13;
const char IN3 = 14;
const char IN4 = 5;
const char IN5 = 16;

const char Buzzer = 15;

#define DATA_PIN 2

CRGB leds[NUM_LEDS];

// Definição dos índices dos LEDs para cada parte da torre
const int parte1_leds[] = {7, 8, 9, 12, 11, 10, 25, 26, 27, 30, 29, 28, 43, 44, 45, 48, 47, 46};
const int parte2_leds[] = {4, 5, 6, 15, 14, 13, 22, 23, 24, 33, 32, 31, 40, 41, 42, 51, 50, 49};
const int parte3_leds[] = {1, 2, 3, 18, 17, 16, 19, 20, 21, 36, 35, 34, 37, 38, 39, 54, 53, 52};
int linhas_torre[9][6] = {
  {parte3_leds[0], parte3_leds[3], parte3_leds[6], parte3_leds[9], parte3_leds[12], parte3_leds[15]},  // linha 1
  {parte3_leds[1], parte3_leds[4], parte3_leds[7], parte3_leds[10], parte3_leds[13], parte3_leds[16]}, // linha 2
  {parte3_leds[2], parte3_leds[5], parte3_leds[8], parte3_leds[11], parte3_leds[14], parte3_leds[17]}, // linha 3
  {parte2_leds[0], parte2_leds[3], parte2_leds[6], parte2_leds[9], parte2_leds[12], parte2_leds[15]},  // linha 4
  {parte2_leds[1], parte2_leds[4], parte2_leds[7], parte2_leds[10], parte2_leds[13], parte2_leds[16]}, // linha 5 
  {parte2_leds[2], parte2_leds[5], parte2_leds[8], parte2_leds[11], parte2_leds[14], parte2_leds[17]}, // linha 6
  {parte1_leds[0], parte1_leds[3], parte1_leds[6], parte1_leds[9], parte1_leds[12], parte1_leds[15]},  // linha 7
  {parte1_leds[1], parte1_leds[4], parte1_leds[7], parte1_leds[10], parte1_leds[13], parte1_leds[16]}, // linha 8
  {parte1_leds[2], parte1_leds[5], parte1_leds[8], parte1_leds[11], parte1_leds[14], parte1_leds[17]}, // linha 9 
};

ESP8266WebServer serverAPP(80);

CRGB corParte1 = CRGB::Black;
CRGB corParte2 = CRGB::Black;
CRGB corParte3 = CRGB::Black;
CRGB corTorreToda = CRGB::Black; // Cor padrão para a torre toda
CRGB ultimaCorParte1 = CRGB::Black;
CRGB ultimaCorParte2 = CRGB::Black;
CRGB ultimaCorParte3 = CRGB::Black;
int brilhoParte1 = 255;
int brilhoParte2 = 255;
int brilhoParte3 = 255;
int brilhoTorreToda = 255;
bool flag_animar = false;
int animacaoAtual = 0;
bool TestandoTorre = false;
bool zerarTorre = false;
bool testeBuzzer = false;

bool configurarTorre = false;

EntradasDigitais_t EntradasDigitais;

Torre_HTML_t Torre_HTML;
App_torre AppTorre;

// Tamanho da estrutura App_torre em bytes
#define APP_TORRE_SIZE (sizeof(App_torre))
// Endereço inicial na EEPROM para armazenar a estrutura
#define EEPROM_ADDR 0

void salvarAppTorreNaEEPROM(const App_torre &dados)
{
  EEPROM.begin(APP_TORRE_SIZE);
  EEPROM.put(EEPROM_ADDR, dados);
  EEPROM.commit();
  EEPROM.end();
}

void lerAppTorreDaEEPROM(App_torre &dados)
{
  EEPROM.begin(APP_TORRE_SIZE);
  EEPROM.get(EEPROM_ADDR, dados);
  EEPROM.end();
}

void APP_TORRE_init(void){

  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // Configuração dos LEDs WS2812B
  FastLED.clear();  // Desliga todos os LEDs no início

  EEPROM.begin(APP_TORRE_SIZE);

  // Lê os dados da EEPROM para a estrutura App_torre
  lerAppTorreDaEEPROM(AppTorre);

  APP_ATUALIZA_DadosHTML();

  // // Configurações do Access Point
  // WiFi.mode(WIFI_AP);
  // WiFi.softAP("Torre Mav_Syrena", "mavsyrena");

  // serverAPP.on("/", handleRoot);
  // serverAPP.on("/configurar", handleConfigurar);
  // serverAPP.on("/animacao", handleAnimacao);

  // serverAPP.begin();

  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
  FastLED.show();

  pinMode(IN1,INPUT);
  pinMode(IN2,INPUT);
  pinMode(IN3,INPUT);
  pinMode(IN4,INPUT);
  pinMode(IN5,INPUT);
  pinMode(Buzzer,OUTPUT);
  digitalWrite(Buzzer,LOW);
  
}

void APP_TORRE_run(void){

  if(TestandoTorre){
    APP_TORRE_controllerTestTorre();
  }
  else{
    APP_TORRE_controllerEntradaDigital();
  }
 
  serverAPP.handleClient(); 
  

  if(digitalRead(4) == 0){

   while (digitalRead(4) == 0)
   {  
      static int contConfig = 0;
      contConfig++;
      if(contConfig > 1000){
        contConfig = 0;

        if(configurarTorre == false){
          configurarTorre = true;
          // Configurações do Access Point
          WiFi.mode(WIFI_AP);
          WiFi.softAP("Torre Mav_Syrena", "mavsyrena");
          serverAPP.on("/", handleRoot);
          serverAPP.on("/configurar", handleConfigurar);
          serverAPP.on("/animacao", handleAnimacao);   
          serverAPP.begin();

          while (digitalRead(4) == 0){ serverAPP.handleClient(); delay(1);}
        }
        else{
          configurarTorre = false;   
          WiFi.mode(WIFI_OFF);
          while (digitalRead(4) == 0){delay(1);}
        }     
      }
      delay(1);
    }
  }

  APP_TORRE_ReadEntradas();

}

void APP_TORRE_ReadEntradas(void){
  
  static uint32_t tick = 0;
  static uint32_t lastTick = 0;

  static uint16_t debounce[5] = {0,0,0,0,0};
  static uint16_t debounce2[5]= {0,0,0,0,0};
  static bool state[5] = {false,false,false,false,false};
  static int ultimaTeclaPressionada = -1;

  tick = millis();
  if ((tick - lastTick) > 1) {
    lastTick = tick; 

      if(digitalRead(IN1) == 0){
        debounce[0]++;
        if(debounce[0] > 50){
          debounce[0] = 0;
          state[0] = true;
          EntradasDigitais.Entrada1 = true; TestandoTorre = false;
        }
      }
      if(digitalRead(IN2) == 0){
        debounce[1]++;
        if(debounce[1] > 50){
          debounce[1] = 0;
          state[1] = true;
          EntradasDigitais.Entrada2 = true; TestandoTorre = false;
        }
      }
      if(digitalRead(IN3) == 0){
        debounce[2]++;
        if(debounce[2] > 50){
          debounce[2] = 0;
          state[2] = true;
          EntradasDigitais.Entrada3 = true; TestandoTorre = false;
        }
      }
      if(digitalRead(IN4) == 0){
        debounce[3]++;
        if(debounce[3] > 50){
          debounce[3] = 0;
          state[3] = true;
          EntradasDigitais.Entrada4 = true; TestandoTorre = false;
        }
      }
      if(digitalRead(IN5) == 0){
        debounce[4]++;
        if(debounce[4] > 50){
          debounce[4] = 0;
          state[4] = true;
          EntradasDigitais.Entrada5 = true; TestandoTorre = false;
        }
      }
      else{
        if(state[0] == false && state[1] == false && state[2] == false && state[3] == false && state[4] == false){
          zerarTorre = true;
        }
      }

        if(state[0] == true){

        }


      if(digitalRead(IN1) == 1 && state[0] == true){
        debounce2[0]++;
        if(debounce2[0] > 150){
          debounce[0] = 0;
          debounce2[0] = 0;
          state[0] = false;
          EntradasDigitais.Entrada1 = false; TestandoTorre = false;
          digitalWrite(Buzzer,LOW);
        }
      }
      if(digitalRead(IN2) == 1 && state[1] == true){
        debounce2[1]++;
        if(debounce2[1] > 150){
          debounce[1] = 0;
          debounce2[1] = 0;
          state[1] = false;
          EntradasDigitais.Entrada2 = false; TestandoTorre = false;
          digitalWrite(Buzzer,LOW);
        }
      }
      if(digitalRead(IN3) == 1 && state[2] == true){
        debounce2[2]++;
        if(debounce2[2] > 150){
          debounce[2] = 0;
          debounce2[2] = 0;
          state[2] = false;
          EntradasDigitais.Entrada3 = false; TestandoTorre = false;
          digitalWrite(Buzzer,LOW);
        }
      }
      if(digitalRead(IN4) == 1 && state[3] == true){
        debounce2[3]++;
        if(debounce2[3] > 150){
          debounce[3] = 0;
          debounce2[3] = 0;
          state[3] = false;
          EntradasDigitais.Entrada4 = false; TestandoTorre = false;
          digitalWrite(Buzzer,LOW);
        }
      }
      if(digitalRead(IN5) == 1 && state[4] == true){
        debounce2[4]++;
        if(debounce2[4] > 150){
          debounce[4] = 0;
          debounce2[4] = 0;
          state[4] = false;
          EntradasDigitais.Entrada5 = false; TestandoTorre = false;
          digitalWrite(Buzzer,LOW);
          }
      }   
   }
}

void APP_TORRE_controllerEntradaDigital(void) {
    static uint32_t tick = 0;
    static uint32_t lastTick = 0;  
    static uint8_t stateFalse[5];
    static uint8_t penultimaEntrada[5];
    static uint8_t index = 0;
    tick = millis();
    if ((tick - lastTick) > 1) {
        lastTick = tick;
        static  int ultimaEntrada = -1; // Variável para rastrear a última entrada verdadeira
       
        if (EntradasDigitais.Entrada1 && stateFalse[0] == 0) { 

            penultimaEntrada[0] =  ultimaEntrada;          
            ultimaEntrada = 1;
            stateFalse[0] = 1;
            index++;
            if(index > 4){index = 0;}
        }
        else if (EntradasDigitais.Entrada2 && stateFalse[1]  == 0) { 
            penultimaEntrada[1] =  ultimaEntrada;            
            ultimaEntrada = 2;
            stateFalse[1] = 1;
            index++;
            if(index > 4){index = 0;}
        }
        else if (EntradasDigitais.Entrada3 && stateFalse[2]  == 0) {
            penultimaEntrada[2] =  ultimaEntrada; 
            ultimaEntrada = 3;
            stateFalse[2] = 1;
            index++;
            if(index > 4){index = 0;}
        }
        else if (EntradasDigitais.Entrada4 && stateFalse[3]  == 0) {
            penultimaEntrada[3] =  ultimaEntrada; 
            ultimaEntrada = 4;
            stateFalse[3] = 1;
            index++;
            if(index > 4){index = 0;}
        }
        else if (EntradasDigitais.Entrada5 && stateFalse[4]  == 0) {
            penultimaEntrada[4] =  ultimaEntrada; 
            ultimaEntrada = 5;
            stateFalse[4] = 1;
            index++;
            if(index > 4){index = 0;}
        }
        
        if(EntradasDigitais.Entrada1 == false ){
          stateFalse[0] = 0;

          if(ultimaEntrada == 1){           
            ultimaEntrada = penultimaEntrada[0]; 
          }         
               
        }
        if(EntradasDigitais.Entrada2 == false ){
          stateFalse[1] = 0;

          if(ultimaEntrada == 2){           
            ultimaEntrada = penultimaEntrada[1]; 
          }

        }
        if(EntradasDigitais.Entrada3 == false ){
          stateFalse[2] = 0;

          if(ultimaEntrada == 3){           
            ultimaEntrada = penultimaEntrada[2]; 
          } 

        }
        if(EntradasDigitais.Entrada4 == false ){
          stateFalse[3] = 0;

          if(ultimaEntrada == 4){           
            ultimaEntrada = penultimaEntrada[3]; 
          }

        }
        if(EntradasDigitais.Entrada5 == false ){
          stateFalse[4] = 0;
          if(ultimaEntrada == 5){           
            ultimaEntrada = penultimaEntrada[4]; 
          }
        }

        if(EntradasDigitais.Entrada1 == false && EntradasDigitais.Entrada2 == false && EntradasDigitais.Entrada3 == false &&
           EntradasDigitais.Entrada4 == false && EntradasDigitais.Entrada5 == false){
            ultimaEntrada = -1;
            index = 0;
            penultimaEntrada[0] = 0;  penultimaEntrada[1] = 0;  penultimaEntrada[2] = 0; 
            penultimaEntrada[3] = 0;  penultimaEntrada[4] = 0; 
        }

        // Executa o tratamento apenas para a última entrada verdadeira
        switch (ultimaEntrada) {
            case 1:
                APP_TORRE_TratamentoEntrada1();
                break;
            case 2:
                APP_TORRE_TratamentoEntrada2();
                break;
            case 3:
                APP_TORRE_TratamentoEntrada3();
                break;
            case 4:
                APP_TORRE_TratamentoEntrada4();
                break;
            case 5:
                APP_TORRE_TratamentoEntrada5();
                break;
            default:
                if (zerarTorre) {
                    zerarTorre = false;
                    for (int i = 0; i < NUM_LEDS; i++) {
                        leds[i] = CRGB::Black;
                    }
                    FastLED.show();
                }
                break;
        }
    }
}


void playSiren() {
  // Estados da máquina de estado
  const int STATE_RISING = 0;
  const int STATE_HOLDING = 1;
  const int STATE_FALLING = 2;
  const int STATE_OFF = 3;

  static int currentFrequency = 10; // Frequência inicial
  static int currentState = STATE_HOLDING; // Estado atual
  static unsigned long stateChangeMillis = 0; // Tempo do último estado de mudança

  // Lógica da máquina de estado
  switch (currentState) {
    case STATE_RISING:
      // Aumente a frequência rapidamente a cada 50 milissegundos
      if (millis() - stateChangeMillis >= 10) {
        // Aumente a frequência
        currentFrequency += 10;
        if (currentFrequency >= 800) {
          currentFrequency = 800; // Fixe a frequência em 1000 Hz
          currentState = STATE_HOLDING;
        }
        tone(Buzzer, currentFrequency); // Ligue o novo som
        stateChangeMillis = millis();
      }
      break;

    case STATE_HOLDING:
      // Mantenha a frequência em 1000 Hz por 250 milissegundos
      digitalWrite(Buzzer,HIGH);
      if (millis() - stateChangeMillis >= 550) {
        currentState = STATE_OFF;
        stateChangeMillis = millis();  
      }
      break;

    case STATE_FALLING:
        // Aumente a frequência rapidamente a cada 50 milissegundos
      if (millis() - stateChangeMillis >= 10) {
        // Aumente a frequência
        currentFrequency -= 10;
        if (currentFrequency <= 400) {
          currentFrequency = 10; // Fixe a frequência em 1000 Hz
          currentState = STATE_OFF;
        }
        tone(Buzzer, currentFrequency); // Ligue o novo som
        stateChangeMillis = millis();
      }
      break;

    case STATE_OFF:
        // Desligue o som e espere por 250 milissegundos
        //noTone(Buzzer);
        digitalWrite(Buzzer,LOW);
        if (millis() - stateChangeMillis >= 550) {
          currentState = STATE_HOLDING;
          stateChangeMillis = millis();
        }
      break;
  }
}


void APP_TORRE_TratamentoEntrada1(void){
  static uint16_t contanima = 0;    
  // set brilho da entrada digital 1
  brilhoParte1 = AppTorre.BrilhoEntrada[0];
  brilhoParte2 = AppTorre.BrilhoEntrada[0];
  brilhoParte3 = AppTorre.BrilhoEntrada[0];
  brilhoTorreToda = AppTorre.BrilhoEntrada[0];
  // set cor da entrada digital 1
  if (AppTorre.CorEntrada[0] == 0) {corTorreToda = CRGB::Red;corParte1 = CRGB::Red;corParte2 = CRGB::Red;corParte3 = CRGB::Red;}
  else if (AppTorre.CorEntrada[0] == 1) {corTorreToda = CRGB::Green;corParte1 = CRGB::Green;corParte2 = CRGB::Green;corParte3 = CRGB::Green;}
  else if (AppTorre.CorEntrada[0] == 2) {corTorreToda = CRGB::Blue;corParte1 = CRGB::Blue;corParte2 = CRGB::Blue;corParte3 = CRGB::Blue;}
  else if (AppTorre.CorEntrada[0] == 3) {corTorreToda = CRGB::Yellow;corParte1 = CRGB::Yellow;corParte2 = CRGB::Yellow;corParte3 = CRGB::Yellow;}
  else if (AppTorre.CorEntrada[0] == 4) {corTorreToda = CRGB::White;corParte1 = CRGB::White;corParte2 = CRGB::White;corParte3 = CRGB::White;}
  else if (AppTorre.CorEntrada[0] == 5) {corTorreToda = CRGB::Black;corParte1 = CRGB::Black;corParte2 = CRGB::Black;corParte3 = CRGB::Black;}
  // roda o efeito da entrada digital 1
  if(AppTorre.AnimaEntrada[0]== 0){contanima++;if(contanima > 12){contanima = 0;giroflexTorre();}}
  else if(AppTorre.AnimaEntrada[0] == 1){contanima++;if(contanima > 12){contanima = 0;giroflexTorre2();}}
  else if(AppTorre.AnimaEntrada[0] == 2){contanima++;if(contanima > 50){contanima = 0;linhaTorre();}}
  else if(AppTorre.AnimaEntrada[0] == 3){contanima++;if(contanima > 5){contanima = 0;fadeColorAnimation();}}
  else if(AppTorre.AnimaEntrada[0] == 4){contanima++;if(contanima > 1){contanima = 0;fadeAnimation();}}
  else if(AppTorre.AnimaEntrada[0] == 5){contanima++;if(contanima > 150){contanima = 0;colorWipeAnimation();}}
  else if(AppTorre.AnimaEntrada[0] == 6){contanima++;if(contanima > 25){contanima = 0;enchendoTorre();}}
  else if(AppTorre.AnimaEntrada[0] == 7){contanima++;if(contanima > 250){contanima = 0;EncheColunaTorre();}}
  else if(AppTorre.AnimaEntrada[0] == 8){contanima++;if(contanima > 25){contanima = 0;LedaLedTorre();}}
  else if(AppTorre.AnimaEntrada[0] == 9){acenderPartes();FastLED.show();}
  // roda buzzer
  if(AppTorre.BuzzerEntrada[0] == 0){digitalWrite(Buzzer,LOW);}
  else{
    playSiren();
    // static int pulso = timePulso;
    // static bool flagPulso = false;
    // pulso++;
    // if(pulso > timePulso){
    //   pulso = 0;
    //   if(flagPulso == false){digitalWrite(Buzzer,HIGH);flagPulso = true;}
    //   else if(flagPulso == true){digitalWrite(Buzzer,LOW);flagPulso = false;}         
    // }    
  }
}

void APP_TORRE_TratamentoEntrada2(void){
    static uint16_t contanima = 0;    
  // set brilho da entrada digital 2
  brilhoParte1 = AppTorre.BrilhoEntrada[1];
  brilhoParte2 = AppTorre.BrilhoEntrada[1];
  brilhoParte3 = AppTorre.BrilhoEntrada[1];
  brilhoTorreToda = AppTorre.BrilhoEntrada[1];
  // set cor da entrada digital 2
  if (AppTorre.CorEntrada[1] == 0) {corTorreToda = CRGB::Red;corParte1 = CRGB::Red;corParte2 = CRGB::Red;corParte3 = CRGB::Red;}
  else if (AppTorre.CorEntrada[1] == 1) {corTorreToda = CRGB::Green;corParte1 = CRGB::Green;corParte2 = CRGB::Green;corParte3 = CRGB::Green;}
  else if (AppTorre.CorEntrada[1] == 2) {corTorreToda = CRGB::Blue;corParte1 = CRGB::Blue;corParte2 = CRGB::Blue;corParte3 = CRGB::Blue;}
  else if (AppTorre.CorEntrada[1] == 3) {corTorreToda = CRGB::Yellow;corParte1 = CRGB::Yellow;corParte2 = CRGB::Yellow;corParte3 = CRGB::Yellow;}
  else if (AppTorre.CorEntrada[1] == 4) {corTorreToda = CRGB::White;corParte1 = CRGB::White;corParte2 = CRGB::White;corParte3 = CRGB::White;}
  else if (AppTorre.CorEntrada[1] == 5) {corTorreToda = CRGB::Black;corParte1 = CRGB::Black;corParte2 = CRGB::Black;corParte3 = CRGB::Black;}
  // roda o efeito da entrada digital 2
  if(AppTorre.AnimaEntrada[1]== 0){contanima++;if(contanima > 12){contanima = 0;giroflexTorre();}}
  else if(AppTorre.AnimaEntrada[1] == 1){contanima++;if(contanima > 12){contanima = 0;giroflexTorre2();}}
  else if(AppTorre.AnimaEntrada[1] == 2){contanima++;if(contanima > 50){contanima = 0;linhaTorre();}}
  else if(AppTorre.AnimaEntrada[1] == 3){contanima++;if(contanima > 5){contanima = 0;fadeColorAnimation();}}
  else if(AppTorre.AnimaEntrada[1] == 4){contanima++;if(contanima > 1){contanima = 0;fadeAnimation();}}
  else if(AppTorre.AnimaEntrada[1] == 5){contanima++;if(contanima > 150){contanima = 0;colorWipeAnimation();}}
  else if(AppTorre.AnimaEntrada[1] == 6){contanima++;if(contanima > 25){contanima = 0;enchendoTorre();}}
  else if(AppTorre.AnimaEntrada[1] == 7){contanima++;if(contanima > 250){contanima = 0;EncheColunaTorre();}}
  else if(AppTorre.AnimaEntrada[1] == 8){contanima++;if(contanima > 25){contanima = 0;LedaLedTorre();}}
  else if(AppTorre.AnimaEntrada[1] == 9){acenderPartes();FastLED.show();}
  // roda buzzer 
  if(AppTorre.BuzzerEntrada[1] == 0){digitalWrite(Buzzer,LOW);}
  else{
    playSiren();
    // static int pulso = timePulso;
    // static bool flagPulso = false;
    // pulso++;
    // if(pulso > timePulso){
    //   pulso = 0;  
    //   if(flagPulso == false){digitalWrite(Buzzer,HIGH);flagPulso = true;}
    //   else if(flagPulso == true){digitalWrite(Buzzer,LOW);flagPulso = false;}       
    // }
    
  }
}

void APP_TORRE_TratamentoEntrada3(void){
    static uint16_t contanima = 0;    
  // set brilho da entrada digital 3
  brilhoParte1 = AppTorre.BrilhoEntrada[2];
  brilhoParte2 = AppTorre.BrilhoEntrada[2];
  brilhoParte3 = AppTorre.BrilhoEntrada[2];
  brilhoTorreToda = AppTorre.BrilhoEntrada[2];
  // set cor da entrada digital 3
  if (AppTorre.CorEntrada[2] == 0) {corTorreToda = CRGB::Red;corParte1 = CRGB::Red;corParte2 = CRGB::Red;corParte3 = CRGB::Red;}
  else if (AppTorre.CorEntrada[2] == 1) {corTorreToda = CRGB::Green;corParte1 = CRGB::Green;corParte2 = CRGB::Green;corParte3 = CRGB::Green;}
  else if (AppTorre.CorEntrada[2] == 2) {corTorreToda = CRGB::Blue;corParte1 = CRGB::Blue;corParte2 = CRGB::Blue;corParte3 = CRGB::Blue;}
  else if (AppTorre.CorEntrada[2] == 3) {corTorreToda = CRGB::Yellow;corParte1 = CRGB::Yellow;corParte2 = CRGB::Yellow;corParte3 = CRGB::Yellow;}
  else if (AppTorre.CorEntrada[2] == 4) {corTorreToda = CRGB::White;corParte1 = CRGB::White;corParte2 = CRGB::White;corParte3 = CRGB::White;}
  else if (AppTorre.CorEntrada[2] == 5) {corTorreToda = CRGB::Black;corParte1 = CRGB::Black;corParte2 = CRGB::Black;corParte3 = CRGB::Black;}
  // roda o efeito da entrada digital 3
  if(AppTorre.AnimaEntrada[2]== 0){contanima++;if(contanima > 12){contanima = 0;giroflexTorre();}}
  else if(AppTorre.AnimaEntrada[2] == 1){contanima++;if(contanima > 12){contanima = 0;giroflexTorre2();}}
  else if(AppTorre.AnimaEntrada[2] == 2){contanima++;if(contanima > 50){contanima = 0;linhaTorre();}}
  else if(AppTorre.AnimaEntrada[2] == 3){contanima++;if(contanima > 5){contanima = 0;fadeColorAnimation();}}
  else if(AppTorre.AnimaEntrada[2] == 4){contanima++;if(contanima > 1){contanima = 0;fadeAnimation();}}
  else if(AppTorre.AnimaEntrada[2] == 5){contanima++;if(contanima > 150){contanima = 0;colorWipeAnimation();}}
  else if(AppTorre.AnimaEntrada[2] == 6){contanima++;if(contanima > 25){contanima = 0;enchendoTorre();}}
  else if(AppTorre.AnimaEntrada[2] == 7){contanima++;if(contanima > 250){contanima = 0;EncheColunaTorre();}}
  else if(AppTorre.AnimaEntrada[2] == 8){contanima++;if(contanima > 25){contanima = 0;LedaLedTorre();}}
  else if(AppTorre.AnimaEntrada[2] == 9){acenderPartes();FastLED.show();}
  // roda buzzer 
  if(AppTorre.BuzzerEntrada[2] == 0){digitalWrite(Buzzer,LOW);}
  else{
    playSiren();
    // static int pulso = timePulso;
    // static bool flagPulso = false;
    // pulso++;
    // if(pulso > timePulso){
    //   pulso = 0;
    //   if(flagPulso == false){digitalWrite(Buzzer,HIGH);flagPulso = true;}
    //   else if(flagPulso == true){digitalWrite(Buzzer,LOW);flagPulso = false;}        
    // }
    
  }
}

void APP_TORRE_TratamentoEntrada4(void){
    static uint16_t contanima = 0;    
  // set brilho da entrada digital 4
  brilhoParte1 = AppTorre.BrilhoEntrada[3];
  brilhoParte2 = AppTorre.BrilhoEntrada[3];
  brilhoParte3 = AppTorre.BrilhoEntrada[3];
  brilhoTorreToda = AppTorre.BrilhoEntrada[3];
  // set cor da entrada digital 4
  if (AppTorre.CorEntrada[3] == 0) {corTorreToda = CRGB::Red;corParte1 = CRGB::Red;corParte2 = CRGB::Red;corParte3 = CRGB::Red;}
  else if (AppTorre.CorEntrada[3] == 1) {corTorreToda = CRGB::Green;corParte1 = CRGB::Green;corParte2 = CRGB::Green;corParte3 = CRGB::Green;}
  else if (AppTorre.CorEntrada[3] == 2) {corTorreToda = CRGB::Blue;corParte1 = CRGB::Blue;corParte2 = CRGB::Blue;corParte3 = CRGB::Blue;}
  else if (AppTorre.CorEntrada[3] == 3) {corTorreToda = CRGB::Yellow;corParte1 = CRGB::Yellow;corParte2 = CRGB::Yellow;corParte3 = CRGB::Yellow;}
  else if (AppTorre.CorEntrada[3] == 4) {corTorreToda = CRGB::White;corParte1 = CRGB::White;corParte2 = CRGB::White;corParte3 = CRGB::White;}
  else if (AppTorre.CorEntrada[3] == 5) {corTorreToda = CRGB::Black;corParte1 = CRGB::Black;corParte2 = CRGB::Black;corParte3 = CRGB::Black;}
  // roda o efeito da entrada digital 4
  if(AppTorre.AnimaEntrada[3]== 0){contanima++;if(contanima > 12){contanima = 0;giroflexTorre();}}
  else if(AppTorre.AnimaEntrada[3] == 1){contanima++;if(contanima > 12){contanima = 0;giroflexTorre2();}}
  else if(AppTorre.AnimaEntrada[3] == 2){contanima++;if(contanima > 50){contanima = 0;linhaTorre();}}
  else if(AppTorre.AnimaEntrada[3] == 3){contanima++;if(contanima > 5){contanima = 0;fadeColorAnimation();}}
  else if(AppTorre.AnimaEntrada[3] == 4){contanima++;if(contanima > 1){contanima = 0;fadeAnimation();}}
  else if(AppTorre.AnimaEntrada[3] == 5){contanima++;if(contanima > 150){contanima = 0;colorWipeAnimation();}}
  else if(AppTorre.AnimaEntrada[3] == 6){contanima++;if(contanima > 25){contanima = 0;enchendoTorre();}}
  else if(AppTorre.AnimaEntrada[3] == 7){contanima++;if(contanima > 250){contanima = 0;EncheColunaTorre();}}
  else if(AppTorre.AnimaEntrada[3] == 8){contanima++;if(contanima > 25){contanima = 0;LedaLedTorre();}}
  else if(AppTorre.AnimaEntrada[3] == 9){acenderPartes();FastLED.show();}
  // roda buzzer 
  if(AppTorre.BuzzerEntrada[3] == 0){digitalWrite(Buzzer,LOW);}
  else{
    playSiren();
    // static int pulso = timePulso;
    // static bool flagPulso = false;
    // pulso++;
    // if(pulso > timePulso){
    //   pulso = 0;  
    //   if(flagPulso == false){digitalWrite(Buzzer,HIGH);flagPulso = true;}
    //   else if(flagPulso == true){digitalWrite(Buzzer,LOW);flagPulso = false;}       
    // }
    
  }
}

void APP_TORRE_TratamentoEntrada5(void){
   static uint16_t contanima = 0;    
  // set brilho da entrada digital 5
  brilhoParte1 = AppTorre.BrilhoEntrada[4];
  brilhoParte2 = AppTorre.BrilhoEntrada[4];
  brilhoParte3 = AppTorre.BrilhoEntrada[4];
  brilhoTorreToda = AppTorre.BrilhoEntrada[4];
  // set cor da entrada digital 5
  if (AppTorre.CorEntrada[4] == 0) {corTorreToda = CRGB::Red;corParte1 = CRGB::Red;corParte2 = CRGB::Red;corParte3 = CRGB::Red;}
  else if (AppTorre.CorEntrada[4] == 1) {corTorreToda = CRGB::Green;corParte1 = CRGB::Green;corParte2 = CRGB::Green;corParte3 = CRGB::Green;}
  else if (AppTorre.CorEntrada[4] == 2) {corTorreToda = CRGB::Blue;corParte1 = CRGB::Blue;corParte2 = CRGB::Blue;corParte3 = CRGB::Blue;}
  else if (AppTorre.CorEntrada[4] == 3) {corTorreToda = CRGB::Yellow;corParte1 = CRGB::Yellow;corParte2 = CRGB::Yellow;corParte3 = CRGB::Yellow;}
  else if (AppTorre.CorEntrada[4] == 4) {corTorreToda = CRGB::White;corParte1 = CRGB::White;corParte2 = CRGB::White;corParte3 = CRGB::White;}
  else if (AppTorre.CorEntrada[4] == 5) {corTorreToda = CRGB::Black;corParte1 = CRGB::Black;corParte2 = CRGB::Black;corParte3 = CRGB::Black;}
  // roda o efeito da entrada digital 5
  if(AppTorre.AnimaEntrada[4]== 0){contanima++;if(contanima > 12){contanima = 0;giroflexTorre();}}
  else if(AppTorre.AnimaEntrada[4] == 1){contanima++;if(contanima > 12){contanima = 0;giroflexTorre2();}}
  else if(AppTorre.AnimaEntrada[4] == 2){contanima++;if(contanima > 50){contanima = 0;linhaTorre();}}
  else if(AppTorre.AnimaEntrada[4] == 3){contanima++;if(contanima > 5){contanima = 0;fadeColorAnimation();}}
  else if(AppTorre.AnimaEntrada[4] == 4){contanima++;if(contanima > 1){contanima = 0;fadeAnimation();}}
  else if(AppTorre.AnimaEntrada[4] == 5){contanima++;if(contanima > 150){contanima = 0;colorWipeAnimation();}}
  else if(AppTorre.AnimaEntrada[4] == 6){contanima++;if(contanima > 25){contanima = 0;enchendoTorre();}}
  else if(AppTorre.AnimaEntrada[4] == 7){contanima++;if(contanima > 250){contanima = 0;EncheColunaTorre();}}
  else if(AppTorre.AnimaEntrada[4] == 8){contanima++;if(contanima > 25){contanima = 0;LedaLedTorre();}}
  else if(AppTorre.AnimaEntrada[4] == 9){acenderPartes();FastLED.show();}
  // roda buzzer 
  if(AppTorre.BuzzerEntrada[4] == 0){digitalWrite(Buzzer,LOW);}
  else{
    playSiren();
    // static int pulso = timePulso;
    // static bool flagPulso = false;
    // pulso++;
    // if(pulso > timePulso){
    //   pulso = 0; 
    //   if(flagPulso == false){digitalWrite(Buzzer,HIGH);flagPulso = true;}
    //   else if(flagPulso == true){digitalWrite(Buzzer,LOW);flagPulso = false;}     
    // }
    
  }
}

void APP_TORRE_controllerTestTorre(void){

  static uint32_t tick = 0;
  static uint32_t lastTick = 0;
  static uint16_t contanima = 0;
  static bool flagRestauraBrilho = false;
  tick = millis();

  if ((tick - lastTick) > 1) {
    lastTick = tick;    

    if((animacaoAtual != 5 || flag_animar == false) && flagRestauraBrilho == true){
      flagRestauraBrilho = false;
      brilhoParte1 = 255;
      brilhoParte2 = 255;
      brilhoParte3 = 255;
      brilhoTorreToda = 255;
    }

    if(testeBuzzer){
      playSiren();
    }
    else{
      digitalWrite(Buzzer,LOW);
    }
           
    if(flag_animar){
      switch (animacaoAtual) {
        case 1:
            contanima++;
            if(contanima > 12){
              contanima = 0;
              giroflexTorre();
            }
          break;
        case 2:
            contanima++;
            if(contanima > 12){
              contanima = 0;
              giroflexTorre2();
            }
          break;
        case 3:
            contanima++;
            if(contanima > 50){
              contanima = 0;
             linhaTorre();
            }
          break;
        case 4:
          contanima++;
          if(contanima > 5){
            contanima = 0;
            fadeColorAnimation();
            flagRestauraBrilho = true;
          }
        break;

        case 5:
         contanima++;
          if(contanima > 1){
            contanima = 0;
            fadeAnimation(); 
            flagRestauraBrilho = true;
          }
        break;
        case 6:
          contanima++;
          if(contanima > 150){
            contanima = 0;
            colorWipeAnimation();
          }

        break;
           case 7:
          contanima++;
          if(contanima > 25){
            contanima = 0;
            enchendoTorre();
          }
        break;
        case 8:
          contanima++;
          if(contanima > 250){
            contanima = 0;
            EncheColunaTorre();
          }
        break;
        case 9:
          contanima++;
          if(contanima > 25){
            contanima = 0;
            LedaLedTorre();
          }
        break;
        

      }         
    }
    else{
      acenderPartes(); // Aplica as configurações atualizadas na torre
      FastLED.show(); // Atualiza os LEDs com as novas cores e brilhos
    }
  }


}

void APP_ATUALIZA_DadosHTML(void){

  for (int i = 0; i < 5; i++)
  {     
    //============================================================================
    // set da cores selecionadas entrada 
    if(AppTorre.CorEntrada[i] == 0){Torre_HTML.CorEntrada[i] = "red";}
    else if(AppTorre.CorEntrada[i] == 1){Torre_HTML.CorEntrada[i] = "green";}
    else if(AppTorre.CorEntrada[i] == 2){Torre_HTML.CorEntrada[i] = "blue";}
    else if(AppTorre.CorEntrada[i] == 3){Torre_HTML.CorEntrada[i] = "yellow";}
    else if(AppTorre.CorEntrada[i] == 4){Torre_HTML.CorEntrada[i] = "white";}
    else if(AppTorre.CorEntrada[i] == 5){Torre_HTML.CorEntrada[i] = "off";}
    // set do brilho selecionado entrda  
    if(AppTorre.BrilhoEntrada[i] == 0){Torre_HTML.BrilhoEntrada[i] = "0";}
    else if(AppTorre.BrilhoEntrada[i] == 63){Torre_HTML.BrilhoEntrada[i]= "25";}
    else if(AppTorre.BrilhoEntrada[i] == 127){Torre_HTML.BrilhoEntrada[i] = "50";}
    else if(AppTorre.BrilhoEntrada[i] == 191){Torre_HTML.BrilhoEntrada[i] = "75";}
    else if(AppTorre.BrilhoEntrada[i] == 255){Torre_HTML.BrilhoEntrada[i] = "100";}
    // set efeito selecionado
    if(AppTorre.AnimaEntrada[i]== 0){Torre_HTML.AnimaEntrada[i] = "Giroflex1";}
    else if(AppTorre.AnimaEntrada[i] == 1){Torre_HTML.AnimaEntrada[i] = "Giroflex2";}
    else if(AppTorre.AnimaEntrada[i] == 2){Torre_HTML.AnimaEntrada[i] = "Linha";}
    else if(AppTorre.AnimaEntrada[i] == 3){Torre_HTML.AnimaEntrada[i] = "FadeColor";}
    else if(AppTorre.AnimaEntrada[i] == 4){Torre_HTML.AnimaEntrada[i] = "Fade";}
    else if(AppTorre.AnimaEntrada[i] == 5){Torre_HTML.AnimaEntrada[i] = "Passando";}
    else if(AppTorre.AnimaEntrada[i] == 6){Torre_HTML.AnimaEntrada[i] = "Enchendo";}
    else if(AppTorre.AnimaEntrada[i] == 7){Torre_HTML.AnimaEntrada[i] = "EncheColuna";}
    else if(AppTorre.AnimaEntrada[i] == 8){Torre_HTML.AnimaEntrada[i] = "LedALed";}
    else if(AppTorre.AnimaEntrada[i] == 9){Torre_HTML.AnimaEntrada[i] = "Solido";}
    // set estado buzzer selecionado
    if(AppTorre.BuzzerEntrada[i] == 0){Torre_HTML.BuzzerEntrada[i] = "Desligado";}
    else if(AppTorre.BuzzerEntrada[i] == 1){Torre_HTML.BuzzerEntrada[i] = "Ligado";}
    //============================================================================
  }

}

//==============================================================================
//---------- ANIMAÇÕES ---------------------------------------------------------
//==============================================================================
 void acenderPartes() {
  // Acende a parte 1 com a cor e o brilho configurados
  for (int i = 0; i < sizeof(parte1_leds) / sizeof(parte1_leds[0]); i++) {
    int ledIndex = parte1_leds[i] - 1;
    leds[ledIndex] = corParte1;
    leds[ledIndex].fadeToBlackBy(255 - brilhoParte1);
  }

  // Acende a parte 2 com a cor e o brilho configurados
  for (int i = 0; i < sizeof(parte2_leds) / sizeof(parte2_leds[0]); i++) {
    int ledIndex = parte2_leds[i] - 1;
    leds[ledIndex] = corParte2;
    leds[ledIndex].fadeToBlackBy(255 - brilhoParte2);
  }

  // Acende a parte 3 com a cor e o brilho configurados
  for (int i = 0; i < sizeof(parte3_leds) / sizeof(parte3_leds[0]); i++) {
    int ledIndex = parte3_leds[i] - 1;
    leds[ledIndex] = corParte3;
    leds[ledIndex].fadeToBlackBy(255 - brilhoParte3);
  }
}

void LedaLedTorre() {

   static int linha = 0;
   static int coluna = 0; 
   static int linhaLigada = 0;
   CRGB cor = CRGB::Black;

  int ledIndex;
  static uint8_t state = 0;
  switch (state)
  {
    case 0:   
        ledIndex = linhas_torre[linha][linhaLigada] - 1; 
        if(linha < 3){
          leds[ledIndex] = corParte3;
        }
        else if( linha < 6){
          leds[ledIndex] = corParte2;
        }
        else{
          leds[ledIndex] = corParte1;
        }   
       linhaLigada ++;
       if(linhaLigada > 5 ){
        linhaLigada = 0;
        linha++;
       }      
      if(linha > 8 ){
        linha = 0;
        state = 1;
      }
      FastLED.show(); 
    break;
     case 1:
       ledIndex = linhas_torre[linha][linhaLigada] - 1;         
       leds[ledIndex] = CRGB::Black;        
       linhaLigada ++;
       if(linhaLigada > 5 ){
        linhaLigada = 0;
        linha++;
       }      
      if(linha > 8 ){
        linha = 0;
        state = 0;
      }
      FastLED.show(); 
    break;


  }
}

void EncheColunaTorre() {

   static int linha = 0;
   static int coluna = 6; 
   static int linhaLigada = 0;
   CRGB cor = CRGB::Black;

  int ledIndex;
  static uint8_t state = 0;
  switch (state)
  {
    case 0:
      // for (int i = 0; i < NUM_LEDS; i++) {
      // leds[i] = CRGB::Black;
      // }     
      for (int i = 0; i < 9; i++) { // acende linha

        
        ledIndex = linhas_torre[i][coluna] - 1; 
        if(i < 3){
          leds[ledIndex] = corParte3;
        }
        else if( i < 6){
          leds[ledIndex] = corParte2;
        }
        else{
          leds[ledIndex] = corParte1;
        }          
        
      }  

      coluna--;
      if(coluna < 0 ){
        coluna = 5;
        state = 1;
      }
      FastLED.show(); 
    break;
     case 1:
      // for (int i = 0; i < NUM_LEDS; i++) {
      // leds[i] = CRGB::Black;
      // }     
      for (int i = 0; i < 9; i++) { // acende linha
        ledIndex = linhas_torre[i][coluna] - 1;           
        leds[ledIndex] = CRGB::Black;
      }  
      coluna--;
      if(coluna < 0 ){
        coluna = 5;
        state = 0;
      }
      FastLED.show(); 
    break;
  }
}

void enchendoTorre() {

   static int linha = 8;
   static int coluna = 0; 
   static int linhaLigada = 0;
   CRGB cor = CRGB::Black;

  int ledIndex;
  static uint8_t state = 0;
  switch (state)
  {
    case 0:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }     
      for (int i = 0; i < 6; i++) { // acende linha
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte3;
      }  
      linha--;
      if(linha < 0 ){
        linha = 8;
        state = 1;
      }
      FastLED.show(); 
    break;

    case 1:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      } 
      for (int i = 0; i < 6; i++) { // acende linha 1 
        ledIndex = linhas_torre[0][i] - 1;           
        leds[ledIndex] = corParte3;
      } 
      for (int i = 0; i < 6; i++) { 
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte3;
      } 
      linha--;
      if(linha < 1 ){
        linha = 8;
        state = 2;
      }
      FastLED.show(); 
    break;

     case 2:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }  
      for (int y = 0; y < 2; y++)
      {
        for (int i = 0; i < 6; i++) {
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = corParte3;
        } 
      } 
      for (int i = 0; i < 6; i++) {
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte3;
      }  
      linha--;
      if(linha < 2 ){
        linha = 8;
        state = 3;
      }
      FastLED.show(); 
    break;

    case 3:  // parte 2
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }       
      for (int y = 0; y < 3; y++)
      {
        for (int i = 0; i < 6; i++) { // acende linha 1 2 3
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = corParte3;
        } 
      } 

      for (int i = 0; i < 6; i++) {
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte2;
      }  
      linha--;
      if(linha < 3 ){
        linha = 8;
        state = 4;
      }
      FastLED.show(); 
    break;

    case 4:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }       
      for (int y = 0; y < 4; y++)
      {
        for (int i = 0; i < 6; i++) { // acende linha
           if(y < 3){ cor = corParte3;}
          else if(y < 6){ cor = corParte2;}
          else{cor = corParte1;}
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = cor;
        } 
      } 
      for (int i = 0; i < 6; i++) { // acende linha
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte2;
      } 
      linha--;
      if(linha < 4 ){
        linha = 8;
        state = 5;
      }
      FastLED.show(); 
    break;

    case 5:
      for (int i = 0; i < NUM_LEDS; i++) {
       leds[i] = CRGB::Black;
      }
      for (int y = 0; y < 5; y++)
      {
        for (int i = 0; i < 6; i++) { // acende linha
            if(y < 3){ cor = corParte3;}
          else if(y <6){ cor = corParte2;}
          else{cor = corParte1;}
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = cor;
        } 
      }

      for (int i = 0; i < 6; i++) { // acende linha
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte2;
      }
         
      linha--;
      if(linha < 5 ){
        linha = 8;
        state = 6;
      }
      FastLED.show(); 
    break;

    case 6: // parte 3
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      } 
      for (int y = 0; y < 6; y++)
      {
         if(y < 3){ cor = corParte3;}
         else if(y < 6){ cor = corParte2;}
         else{cor = corParte1;}

        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = cor;
        } 
      }
      for (int i = 0; i < 6; i++) { // acende linha        
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte1;
      } 
        
      linha--;
      if(linha < 6 ){
        linha = 8;
        state = 7;
      }
      FastLED.show(); 
    break;

     case 7:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      } 
      for (int y = 0; y < 7; y++)
      {
         if(y < 3){ cor = corParte3;}
          else if(y < 6){ cor = corParte2;}
          else{cor = corParte1;}
        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = cor;
        } 
      }       
      for (int i = 0; i < 6; i++) { // acende linha         
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte1;
      }  
      
      linha--;
      if(linha < 7 ){
        linha = 8;
        state = 8;
      }
      FastLED.show(); 
    break;

    case 8:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }    
      for (int y = 0; y < 8; y++)
      {
          if(y < 3){ cor = corParte3;}
          else if(y < 6){ cor = corParte2;}
          else{cor = corParte1;}
        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = cor;
        } 
      } 
      for (int i = 0; i < 6; i++) { // acende linha    
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte1;
      }  
     
      linha--;
      if(linha < 8 ){
        linha = 8;
        state = 9;
      }
      FastLED.show(); 
    break;

    case 9:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }  
      for (int y = 0; y < 9; y++)
      {
          if(y < 3){ cor = corParte3;}
          else if(y < 6){ cor = corParte2;}
          else{cor = corParte1;}
        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = cor;
        } 
      } 
      for (int i = 0; i < 6; i++) { // acende linha        
        ledIndex = linhas_torre[linha][i] - 1;           
        leds[ledIndex] = corParte1;
      }        
      linha--;
      if(linha < 9 ){
        linha = 8;
        state = 0;
      }
      FastLED.show(); 
    break;
  }
}

void colorWipeAnimation() {
  static int linha = 0;
  static int coluna = 0;   
  int ledIndex;
  static uint8_t state = 0;  

  switch (state)
  {
    case 0:
     for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }  
     for (int y = 0; y < 3; y++)
     {
        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = corParte3;
        }  
     }   
      FastLED.show();
     state = 1;     
    break;
    case 1:
      for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }  
     for (int y = 3; y < 6; y++)
     {
        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = corParte2;
        }  
     }   
      FastLED.show();
     state = 2;  
    break;
    case 2:
     for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }  
     for (int y = 6; y < 9; y++)
     {
        for (int i = 0; i < 6; i++) { // acende linha
          ledIndex = linhas_torre[y][i] - 1;           
          leds[ledIndex] = corParte1;
        }  
     }   
      FastLED.show();
     state = 0;  
    break;  
    default:
     for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
      }
    break;
  } 
}

void fadeColorAnimation() {

  static uint8_t state = 0;
  static int i = 0;
  static int bri = 0;

  switch (state)
  {
    // cor red
    case 0:  
      bri = i;    
      i++;
      if(i >= 255){       
        state = 1;
      }
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Red;
        leds[i].fadeToBlackBy(255 - bri);
      }    
      FastLED.show();
    break;

    case 1:
     bri = i;  
    i--;
    if(i <= 0 ){
      state = 2;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Red;
      leds[i].fadeToBlackBy(255 - bri);
    } 
    FastLED.show();
    break;
    // cor green
    case 2:  
      bri = i;      
      i++;
      if(i >= 255){       
        state = 3;
      }
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Green;
        leds[i].fadeToBlackBy(255 - bri);
      }    
      FastLED.show();
    break;

    case 3:
     bri = i;
    i--;
    if(i <= 0 ){
      state = 4;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Green;
      leds[i].fadeToBlackBy(255 - bri);
    } 
    FastLED.show();
    break;
     // cor Blue
    case 4:  
      bri = i;     
      i++;
      if(i >= 255){       
        state = 5;
      }
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Blue;
        leds[i].fadeToBlackBy(255 - bri);
      }    
      FastLED.show();
    break;

    case 5:
     bri = i;
    i--;
    if(i <= 0 ){
      state = 6;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Blue;
      leds[i].fadeToBlackBy(255 - bri);
    } 
    FastLED.show();
    break;
    // cor yellow
    case 6:  
      bri = i;
      i++;
      if(i >= 255){       
        state = 7;
      }
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::Yellow;
        leds[i].fadeToBlackBy(255 - bri);
      }    
      FastLED.show();
    break;

    case 7:
     bri = i;
    i--;
    if(i <= 0 ){
      state = 8;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Yellow;
      leds[i].fadeToBlackBy(255 - bri);
    } 
    FastLED.show();
    break;
    // cor Write
    case 8:  
      bri = i;
      i++;
      if(i >= 255){       
        state = 9;
      }
      for (int i = 0; i < NUM_LEDS; i++) {
        leds[i] = CRGB::White;
        leds[i].fadeToBlackBy(255 - bri);
      }    
      FastLED.show();
    break;

    case 9:
    bri = i;
    i--;
    if(i <= 0 ){
      state = 0;
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::White;
      leds[i].fadeToBlackBy(255 - bri);
    } 
    FastLED.show();
    break;

    
  }

}

void fadeAnimation() {

  static uint8_t state = 0;
  static int i = 0;

  switch (state)
  {
    case 0:     

      brilhoParte1 = i;
      brilhoParte2 = i;
      brilhoParte3 = i;  
      i++;
      if(i >= 255){       
        state = 1;
      }
      acenderPartes();
      FastLED.show();

    break;

    case 1:
     brilhoParte1 = i;
      brilhoParte2 = i;
      brilhoParte3 = i; 

    i--;
    if(i <= 0 ){
      state = 0;
    }
    acenderPartes();
    FastLED.show();
    break;
    
  }

}

void giroflexTorre() {
  static int posicao = 0;
  static int posicao2 = 0;
  static int contador = 0;
  int ledIndex;
  static uint8_t state = 0;

  switch (state)
  {
    case 0:
       for (int i = 0; i < 3; i++) {
          ledIndex = parte1_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte1;
        }  
         for (int i = 9; i < 12; i++) {
          ledIndex = parte1_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte1;
        }  

        for (int i = 0; i < 3; i++) {
          ledIndex = parte2_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte2;
        }  
         for (int i = 9; i < 12; i++) {
          ledIndex = parte2_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte2;
        } 

        for (int i = 0; i < 3; i++) {
          ledIndex = parte3_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte3;
        }  
         for (int i = 9; i < 12; i++) {
          ledIndex = parte3_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte3;
        } 

        FastLED.show();
        state = 1;
      break;

    case 1:
      // Desliga os LEDs da sequência atual
      for (int i = 0; i < 3; i++) {
        ledIndex = parte1_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }
       for (int i = 9; i < 12; i++) {
        ledIndex = parte1_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }

      for (int i = 0; i < 3; i++) {
        ledIndex = parte2_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }
       for (int i = 9; i < 12; i++) {
        ledIndex = parte2_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }

      for (int i = 0; i < 3; i++) {
        ledIndex = parte3_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }
       for (int i = 9; i < 12; i++) {
        ledIndex = parte3_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }


      posicao = posicao + 3;
      if(posicao > 6){
        posicao = 0;
      }


      for (int i = 0; i < 3; i++) {
        ledIndex = parte1_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte1;
      }  
      for (int i = 9; i < 12; i++) {
        ledIndex = parte1_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte1;
      } 

       for (int i = 0; i < 3; i++) {
        ledIndex = parte2_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte2;
      }  
      for (int i = 9; i < 12; i++) {
        ledIndex = parte2_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte2;
      }


       for (int i = 0; i < 3; i++) {
        ledIndex = parte3_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte3;
      }  
      for (int i = 9; i < 12; i++) {
        ledIndex = parte3_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte3;
      }



      FastLED.show();     
      state = 0;
    break; 
  }

}

void giroflexTorre2() {
  static int posicao = 0;
  static int posicao2 = 0;
  static int contador = 0;
  int ledIndex;
  static uint8_t state = 0;

  switch (state)
  {
    case 0:
       for (int i = 0; i < 3; i++) {
          ledIndex = parte1_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte1;
        } 
        for (int i = 0; i < 3; i++) {
          ledIndex = parte2_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte2;
        }
        for (int i = 0; i < 3; i++) {
          ledIndex = parte3_leds[(posicao + i)] - 1;          
          leds[ledIndex] = corParte3;
        }           
        FastLED.show();
        state = 1;
      break;

    case 1:
      // Desliga os LEDs da sequência atual
      for (int i = 0; i < 3; i++) {
        ledIndex = parte1_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }       
      for (int i = 0; i < 3; i++) {
        ledIndex = parte2_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      } 
      for (int i = 0; i < 3; i++) {
        ledIndex = parte3_leds[posicao + i] - 1 ;      
        leds[ledIndex] = CRGB::Black;
      }       

      posicao = posicao + 3;
      if(posicao > 15){
        posicao = 0;
      }

      for (int i = 0; i < 3; i++) {
        ledIndex = parte1_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte1;
      } 
      for (int i = 0; i < 3; i++) {
        ledIndex = parte2_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte2;
      }       
      for (int i = 0; i < 3; i++) {
        ledIndex = parte3_leds[(posicao + i)] - 1;        
        leds[ledIndex] = corParte3;
      } 
      FastLED.show();     
      state = 0;
    break; 
  }

}

void linhaTorre() {
  int numRows = 3; // Número de linhas
  int numCols = 6; // Número de colunas
  static int currentStep = 0;
  static int currentPart = 3; // Parte atual
  static int currentRow = 0; // Linha atual

  // Define a matriz de LEDs correspondente à parte atual
  const int* currentLEDs;
  int numLEDs;

  // Determina a matriz de LEDs e a quantidade de LEDs para a parte atual
  if (currentPart == 1) {
    currentLEDs = parte1_leds;
    numLEDs = sizeof(parte1_leds) / sizeof(int);
  } else if (currentPart == 2) {
    currentLEDs = parte2_leds;
    numLEDs = sizeof(parte2_leds) / sizeof(int);
  } else if (currentPart == 3) {
    currentLEDs = parte3_leds;
    numLEDs = sizeof(parte3_leds) / sizeof(int);
  }

    // Apaga todos os LEDs de todas as partes
  for (int part = 1; part <= 3; part++) {
    const int* ledsPart;
    int numLEDsPart;
    
    if (part == 1) {
      ledsPart = parte1_leds;
      numLEDsPart = sizeof(parte1_leds) / sizeof(int);
    } else if (part == 2) {
      ledsPart = parte2_leds;
      numLEDsPart = sizeof(parte2_leds) / sizeof(int);
    } else if (part == 3) {
      ledsPart = parte3_leds;
      numLEDsPart = sizeof(parte3_leds) / sizeof(int);
    }
    
    for (int i = 0; i < numLEDsPart; i++) {
      int ledIndexPart = ledsPart[i] - 1; // Índice do LED na parte atual
      leds[ledIndexPart] = CRGB::Black; // Apaga o LED
    }
  }

  // Verifica se é necessário apagar a linha 3 da parte atual
  if (currentRow == 0 && currentPart != 3) {
    int ledIndex = (numCols - 1) * numRows + 2; // Índice do LED da linha 3 na parte atual
    int ledIndexPart = currentLEDs[ledIndex] - 1; // Índice do LED na parte atual    
  }

  // Acende a linha atual
  for (int col = 0; col < numCols; col++) {
    int ledIndex = currentRow + col * numRows; // Índice do LED na matriz
    int ledIndexPart = currentLEDs[ledIndex] - 1; // Índice do LED na parte atual
    leds[ledIndexPart] = (currentPart == 1) ? corParte1 : (currentPart == 2) ? corParte2 : corParte3;
  }

  FastLED.show(); // Atualiza os LEDs com as mudanças realizadas

  // Incrementa a linha atual e verifica se precisa mudar para a próxima parte ou reiniciar a sequência
  currentRow++;
  if (currentRow >= numRows) {
    currentRow = 0;
    currentPart--;
    if (currentPart == 0) {
      currentPart = 3;
    }
  }

}

//==============================================================================
//------ Pagina HTML -----------------------------------------------------------
//==============================================================================
void handleRoot() {
  String html = "<html><head>";
  html += "<meta charset=\"UTF-8\" name='viewport' content='width=device-width, initial-scale=1, user-scalable=no'>";
  html +="<style>"
          "body {"
          "  padding: 8px;"
          "  font-size: 16px;"
          "  line-height: 1.2;"
          "  background-color: #f2f2f2; /* Cor de fundo da página */"
          "}"
          "h1, h2 {"
          "  margin: 2px 0;"
          "  margin-bottom: 10px; /* Maior espaçamento inferior para títulos */"
          "}"
          ".button-container {"
          "  display: flex;"
          "  flex-wrap: wrap;"
          "}"
          ".config-section {"
          "  display: inline-block;"
          "  margin-right: 20px;"
          "  margin-bottom: 10px; /* Espaçamento inferior para as seções */"
          "}"
          ".color-button {"
          "  width: 120px; /* Tamanho maior para botões de cor */"
          "  height: 40px;"
          "  border: 2px solid #000000;"
          "  border-radius: 5px;"
          "  font-size: 16px; /* Tamanho de fonte maior */"
          "  color: #fff;"
          "  background-color: #404040;"
          "  text-align: center;"
          "  cursor: pointer; /* Mostrar cursor ao passar o mouse */"
          "}"
          ".color-button:hover {"
          "  background-color: #555555; /* Cor de destaque ao passar o mouse */"
          "}"
          ".red { background-color: #ff0000; color: #000;}"
          ".green { background-color: #00ff00; color: #000;}"
          ".blue { background-color: #0000ff; color: #000;}"
          ".yellow { background-color: #ffff00; color: #000;}"
          ".white { background-color: #ffffff; color: #000; }"
          ".black { background-color: #000000; color: #fff; }"
          "select {"
          "  width: 150px; /* Tamanho maior para elementos de seleção */"
          "  padding: 5px;"
          "  font-size: 16px;"
          "  border: 2px solid #000;"
          "  border-radius: 5px;"
          "}"
          "button {"
          "  width: 120px; /* Tamanho maior para o botão Update */"
          "  height: 40px;"
          "  border: none;"
          "  border-radius: 5px;"
          "  font-size: 16px;"
          "  color: #fff;"
          "  background-color: #0073e6; /* Cor de fundo azul para o botão */"
          "  cursor: pointer;"
          "}"
          "button:hover {"
          "  background-color: #0059b3; /* Cor de destaque ao passar o mouse */"
          "}"
          ".config-table {"
          "  border-collapse: collapse;" // Mescla as bordas das células da tabela
          "  width: 100%;" // Define a largura da tabela para preencher 100% do contêiner pai
          "}"
           ".config-table th, .config-table td {"
          "  padding: 8px;" // Adiciona espaço interno às células para espaçamento
          "  text-align: center;" // Centraliza o conteúdo das células
          "  border: 2px solid #000;" // Adiciona uma borda de 1px às células internas
          "}"
          ".config-table th {"
          "  background-color: #f2f2f2;" // Cor de fundo para cabeçalho da tabela
          "  position: relative;" // Permite posicionamento relativo para adicionar sombra
          "}"
          ".config-table th::after {"
          "  content: '';" // Adiciona um elemento de pseudo conteúdo ::after
          "  position: absolute;" // Posiciona o elemento de sombra absolutamente
          "  top: 0; right: 0; bottom: 0; left: 0;" // Estende a sombra sobre toda a célula
          "  box-shadow: 0 0 10px rgba(0, 0, 0, 0.3);" // Adiciona uma sombra 3D
          "}"
          "</style>";
  html += "<script>"
          "function openFullscreen() {"
          "  var element = document.documentElement;"
          "  if (element.requestFullscreen) {"
          "    element.requestFullscreen();"
          "  } else if (element.mozRequestFullScreen) {"
          "    element.mozRequestFullScreen();"
          "  } else if (element.webkitRequestFullscreen) {"
          "    element.webkitRequestFullscreen();"
          "  } else if (element.msRequestFullscreen) {"
          "    element.msRequestFullscreen();"
          "  }"
          "}"
          "openFullscreen();"
          "</script>";
  html += "</head><body>";
 // html += "<h2 style='font-size: 20px;'>Torre Led</h2>";;


// Seção 1: Configurações da Parte 1
html += "<h2 style='font-size: 16px;'>Configurar Entradas</h2>";
html += "<form action='/configurar' method='POST'>";
html += "<table class='config-table'>";
// Cabeçalho da tabela
html += "<tr>";
html += "<th>Entrada</th>";
html += "<th>Cor</th>";
html += "<th>Brilho</th>";
html += "<th>Efeito</th>";
html += "<th>Buzzer</th>";
html += "</tr>";

for (int entrada = 0; entrada < 5; entrada++) {
    html += "<tr>";
    html += "<td>Entrada " + String(entrada + 1) + "</td>";

    // Opções de Cor
    html += "<td><select name='Torre_HTML.CorEntrada[" + String(entrada) + "]'>";
    html += "<option value='red'";
    html += (Torre_HTML.CorEntrada[entrada] == "red" ? " selected" : "");
    html += ">Vermelho</option>";
    html += "<option value='green'";
    html += (Torre_HTML.CorEntrada[entrada] == "green" ? " selected" : "");
    html += ">Verde</option>";
    html += "<option value='blue'";
    html += (Torre_HTML.CorEntrada[entrada] == "blue" ? " selected" : "");
    html += ">Azul</option>";
    html += "<option value='yellow'";
    html += (Torre_HTML.CorEntrada[entrada] == "yellow" ? " selected" : "");
    html += ">Amarelo</option>";
    html += "<option value='white'";
    html += (Torre_HTML.CorEntrada[entrada] == "white" ? " selected" : "");
    html += ">Branco</option>";
    html += "<option value='off'";
    html += (Torre_HTML.CorEntrada[entrada] == "off" ? " selected" : "");
    html += ">Apagado</option>";
    html += "</select></td>";

    // Opções de Brilho
    html += "<td><select name='Torre_HTML.BrilhoEntrada[" + String(entrada) + "]'>";
    html += "<option value='0'";
    html += (Torre_HTML.BrilhoEntrada[entrada] == "0" ? " selected" : "");
    html += ">0%</option>";
    html += "<option value='25'";
    html += (Torre_HTML.BrilhoEntrada[entrada] == "25" ? " selected" : "");
    html += ">25%</option>";
    html += "<option value='50'";
    html += (Torre_HTML.BrilhoEntrada[entrada] == "50" ? " selected" : "");
    html += ">50%</option>";
    html += "<option value='75'";
    html += (Torre_HTML.BrilhoEntrada[entrada] == "75" ? " selected" : "");
    html += ">75%</option>";
    html += "<option value='100'";
    html += (Torre_HTML.BrilhoEntrada[entrada] == "100" ? " selected" : "");
    html += ">100%</option>";
    html += "</select></td>";

    // Opções de Efeito
    html += "<td><select name='Torre_HTML.AnimaEntrada[" + String(entrada) + "]'>";
    html += "<option value='Solido'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Solido" ? " selected" : "");
    html += ">Solido</option>";
    html += "<option value='Giroflex1'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Giroflex1" ? " selected" : "");
    html += ">Giroflex 1</option>";
    html += "<option value='Giroflex2'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Giroflex2" ? " selected" : "");
    html += ">Giroflex 2</option>";
    html += "<option value='Linha'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Linha" ? " selected" : "");
    html += ">Linha</option>";   
    html += ">FadeColor</option>";
    html += "<option value='Fade'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Fade" ? " selected" : "");
    html += ">Fade</option>";
    html += "<option value='Passando'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Passando" ? " selected" : "");
    html += ">Passando</option>";
    html += "<option value='Enchendo'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "Enchendo" ? " selected" : "");
    html += ">Enchendo</option>";
    html += "<option value='EncheColuna'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "EncheColuna" ? " selected" : "");
    html += ">Enche Coluna</option>";
    html += "<option value='LedALed'";
    html += (Torre_HTML.AnimaEntrada[entrada] == "LedALed" ? " selected" : "");
    html += ">Led a Led</option>";
    html += ">Enche Coluna</option>";    
    html += ">Led a Led</option>";
    html += "</select></td>";

    // Opções de Buzzer
    html += "<td><select name='Torre_HTML.BuzzerEntrada[" + String(entrada) + "]'>";
    html += "<option value='Ligado'";
    html += (Torre_HTML.BuzzerEntrada[entrada] == "Ligado" ? " selected" : "");
    html += ">Ligado</option>";
    html += "<option value='Desligado'";
    html += (Torre_HTML.BuzzerEntrada[entrada] == "Desligado" ? " selected" : "");
    html += ">Desligado</option>";
    html += "</select></td>";

    html += "</tr>";
}

// Feche a tabela
html += "</table>";

// Botão "Update" (fora da tabela)
html += "<button type='submit' name='ConfigIN1' value='true'>Update</button>";
html += "</form>";
  
  // Seção 4: Configurações da Torre Toda
  html += "</br>";
  html += "<h2 style='font-size: 16px;'>Teste da Torre</h2>";
  html += "<form action='/configurar' method='POST'>";
  html += "<div class='button-container'>";
  html += "<div class='button-group'>";
  html += "<button type='submit' name='cor_torre_toda' value='red' class='color-button red'>Vermelho</button>";
  html += "<button type='submit' name='cor_torre_toda' value='green' class='color-button green'>Verde</button>";
  html += "<button type='submit' name='cor_torre_toda' value='blue' class='color-button blue'>Azul</button>";
  html += "</div>";
  html += "<div class='button-group'>";
  html += "<button type='submit' name='cor_torre_toda' value='yellow' class='color-button yellow'>Amarelo</button>";
  html += "<button type='submit' name='cor_torre_toda' value='white' class='color-button white'>Branco</button>";
  html += "<button type='submit' name='cor_torre_toda' value='off' class='color-button black'>Apagado</button>"; 
  html += "</div>";
  html += "<div class='button-group'>";
  html += "<button type='submit' name='cor_torre_toda' value='BuzzerOn' class='color-button gray'>Buzzer ON</button>";
  html += "<button type='submit' name='cor_torre_toda' value='BuzzerOff' class='color-button gray'>Buzzer OFF</button>";
  html += "</div>";
  html += "<div>";
  html += "<label for='brilho_torre_toda'>Brilho:</label>";
  html += "<input type='range' name='brilho_torre_toda' min='0' max='255' value='" + String(brilhoTorreToda) + "'>";
  html += "</div>";
  html += "</form>";

// Seção 5: Botões de animações
 html += "<h2 style='font-size: 16px;'>Efeitos :</h2>";
html += "<form action='/animacao' method='POST'>";
html += "<div class='button-container'>";
html += "<div class='button-group'>";
html += "<button type='submit' name='animacao_1' value='an1' class='color-button'>Giroflex 1</button>";
html += "<button type='submit' name='animacao_2' value='an2' class='color-button'>Giroflex 2</button>";
html += "<button type='submit' name='animacao_3' value='an3' class='color-button'>Linha</button>";
html += "</div>";
html += "<div class='button-group'>";
html += "<button type='submit' name='animacao_4' value='an4' class='color-button'>FadeColor</button>";
html += "<button type='submit' name='animacao_5' value='an5' class='color-button'>Fade</button>";
html += "<button type='submit' name='animacao_6' value='an6' class='color-button'>Passando</button>";
html += "</div>";
html += "<div class='button-group'>";
html += "<button type='submit' name='animacao_7' value='an7' class='color-button'>Enchendo</button>";
html += "<button type='submit' name='animacao_8' value='an8' class='color-button'>Enche Coluna</button>";
html += "<button type='submit' name='animacao_9' value='an9' class='color-button'>Led a Led</button>";
html += "</div>";
html += "</div>";
html += "</form>";

  html += "</body></html>";

  serverAPP.send(200, "text/html", html);
}

void handleConfigurar() {

  if (serverAPP.method() == HTTP_POST) 
  {

    flag_animar = false;

    if (serverAPP.hasArg("ConfigIN1")) {
       // Atualize os valores com base nos dados recebidos do cliente
      for (int entrada = 0; entrada < 5; entrada++) {
        String paramName = "Torre_HTML.CorEntrada[" + String(entrada) + "]";
        if (serverAPP.hasArg(paramName)) {
          // Atribua o valor do parâmetro de entrada para a cor correspondente em Torre_HTML
          Torre_HTML.CorEntrada[entrada] = serverAPP.arg(paramName);
        }
        String paramName2 = "Torre_HTML.AnimaEntrada[" + String(entrada) + "]";
        if (serverAPP.hasArg(paramName2)) {
          // Atribua o valor do parâmetro de entrada para a cor correspondente em Torre_HTML
          Torre_HTML.AnimaEntrada[entrada] = serverAPP.arg(paramName2);
        }
        String paramName3 = "Torre_HTML.BuzzerEntrada[" + String(entrada) + "]";
        if (serverAPP.hasArg(paramName3)) {
          // Atribua o valor do parâmetro de entrada para a cor correspondente em Torre_HTML
          Torre_HTML.BuzzerEntrada[entrada] = serverAPP.arg(paramName3);
        }
        String paramName4 = "Torre_HTML.BrilhoEntrada[" + String(entrada) + "]";
        if (serverAPP.hasArg(paramName4)) {
          // Atribua o valor do parâmetro de entrada para a cor correspondente em Torre_HTML
          Torre_HTML.BrilhoEntrada[entrada] = serverAPP.arg(paramName4);
        }
      }

      //====================
      for (int i = 0; i < 5; i++) {     
        //============================================================================
        // Configuração das cores selecionadas de entrada 
        if (Torre_HTML.CorEntrada[i] == "red") { AppTorre.CorEntrada[i] = 0; }
        else if (Torre_HTML.CorEntrada[i] == "green") { AppTorre.CorEntrada[i] = 1; }
        else if (Torre_HTML.CorEntrada[i] == "blue") { AppTorre.CorEntrada[i] = 2; }
        else if (Torre_HTML.CorEntrada[i] == "yellow") { AppTorre.CorEntrada[i] = 3; }
        else if (Torre_HTML.CorEntrada[i] == "white") { AppTorre.CorEntrada[i] = 4; }
        else if (Torre_HTML.CorEntrada[i] == "off") { AppTorre.CorEntrada[i] = 5; }

        // Configuração do brilho selecionado de entrada  
        if (Torre_HTML.BrilhoEntrada[i] == "0") { AppTorre.BrilhoEntrada[i] = 0; }
        else if (Torre_HTML.BrilhoEntrada[i] == "25") { AppTorre.BrilhoEntrada[i] = 63; }
        else if (Torre_HTML.BrilhoEntrada[i] == "50") { AppTorre.BrilhoEntrada[i] = 127; }
        else if (Torre_HTML.BrilhoEntrada[i] == "75") { AppTorre.BrilhoEntrada[i] = 191; }
        else if (Torre_HTML.BrilhoEntrada[i] == "100") { AppTorre.BrilhoEntrada[i] = 255; }

        // Configuração do efeito selecionado
        if (Torre_HTML.AnimaEntrada[i] == "Giroflex1") { AppTorre.AnimaEntrada[i] = 0; }
        else if (Torre_HTML.AnimaEntrada[i] == "Giroflex2") { AppTorre.AnimaEntrada[i] = 1; }
        else if (Torre_HTML.AnimaEntrada[i] == "Linha") { AppTorre.AnimaEntrada[i] = 2; }
        else if (Torre_HTML.AnimaEntrada[i] == "FadeColor") { AppTorre.AnimaEntrada[i] = 3; }
        else if (Torre_HTML.AnimaEntrada[i] == "Fade") { AppTorre.AnimaEntrada[i] = 4; }
        else if (Torre_HTML.AnimaEntrada[i] == "Passando") { AppTorre.AnimaEntrada[i] = 5; }
        else if (Torre_HTML.AnimaEntrada[i] == "Enchendo") { AppTorre.AnimaEntrada[i] = 6; }
        else if (Torre_HTML.AnimaEntrada[i] == "EncheColuna") { AppTorre.AnimaEntrada[i] = 7; }
        else if (Torre_HTML.AnimaEntrada[i] == "LedALed") { AppTorre.AnimaEntrada[i] = 8; }
        else if (Torre_HTML.AnimaEntrada[i] == "Solido") { AppTorre.AnimaEntrada[i] = 9; }

        // Configuração do estado do buzzer selecionado
        if (Torre_HTML.BuzzerEntrada[i] == "Desligado") { AppTorre.BuzzerEntrada[i] = 0; }
        else if (Torre_HTML.BuzzerEntrada[i] == "Ligado") { AppTorre.BuzzerEntrada[i] = 1; }
        //============================================================================
      }

        // Salve a estrutura na EEPROM
        salvarAppTorreNaEEPROM(AppTorre);

      Serial.println("Set das variaveis : ");
      Serial.println(Torre_HTML.CorEntrada[0]);
      Serial.println(Torre_HTML.AnimaEntrada[0]);
      Serial.println(Torre_HTML.BuzzerEntrada[0]);
      Serial.println(Torre_HTML.BrilhoEntrada[0]);
                          
      // Faça o que você precisa com esses valores (por exemplo, armazenar em variáveis)
    }

    
    // teste da torre toda 
    if (serverAPP.hasArg("cor_torre_toda")) {
      TestandoTorre = true;
      String corTorreTodaValue = serverAPP.arg("cor_torre_toda");
      if (corTorreTodaValue == "red") {
        corTorreToda = CRGB::Red;
        corParte1 = CRGB::Red;
        corParte2 = CRGB::Red;
        corParte3 = CRGB::Red;
      } else if (corTorreTodaValue == "green") {
        corTorreToda = CRGB::Green;
        corParte1 = CRGB::Green;
        corParte2 = CRGB::Green;
        corParte3 = CRGB::Green;
      } else if (corTorreTodaValue == "blue") {
        corTorreToda = CRGB::Blue;
        corParte1 = CRGB::Blue;
        corParte2 = CRGB::Blue;
        corParte3 = CRGB::Blue;
      } else if (corTorreTodaValue == "yellow") {
        corTorreToda = CRGB::Yellow;
        corParte1 = CRGB::Yellow;
        corParte2 = CRGB::Yellow;
        corParte3 = CRGB::Yellow;
      } else if (corTorreTodaValue == "white") {
        corTorreToda = CRGB::White;
        corParte1 = CRGB::White;
        corParte2 = CRGB::White;
        corParte3 = CRGB::White;
      } else if (corTorreTodaValue == "off") {
        corTorreToda = CRGB::Black;
        corParte1 = CRGB::Black;
        corParte2 = CRGB::Black;
        corParte3 = CRGB::Black;
      }
      else if(corTorreTodaValue == "BuzzerOn"){
        testeBuzzer = true;
      }
      else if(corTorreTodaValue == "BuzzerOff"){
        testeBuzzer = false;
      }
    } 
    if (serverAPP.hasArg("brilho_torre_toda")) {
      brilhoTorreToda = serverAPP.arg("brilho_torre_toda").toInt();
      brilhoParte1 = brilhoTorreToda;
      brilhoParte2 = brilhoTorreToda;
      brilhoParte3 = brilhoTorreToda;
    }

    // acenderPartes(); // Aplica as configurações atualizadas na torre
    // FastLED.show(); // Atualiza os LEDs com as novas cores e brilhos
   
    // delay(100); // Aguarda 100 milissegundos antes de redirecionar
  }

  serverAPP.sendHeader("Location", "/", true); // Redireciona para a página inicial após enviar as configurações
  serverAPP.send(302, "text/plain", ""); // Envia uma resposta vazia com o código de redirecionamento (302)  
}

void handleAnimacao() {
  if (serverAPP.method() == HTTP_POST) 
  {
    // Desliga todos os LEDs
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i] = CRGB::Black;
    }

    flag_animar = true;
    if (serverAPP.hasArg("animacao_1")) {
      animacaoAtual = 1;
    } else if (serverAPP.hasArg("animacao_2")) {
      animacaoAtual = 2;
    } else if (serverAPP.hasArg("animacao_3")) {
      animacaoAtual = 3;
    } else if (serverAPP.hasArg("animacao_4")) {
      animacaoAtual = 4;
    } else if (serverAPP.hasArg("animacao_5")) {
      animacaoAtual = 5;
    } else if (serverAPP.hasArg("animacao_6")) {
      animacaoAtual = 6;
    } else if (serverAPP.hasArg("animacao_7")) {
      animacaoAtual = 7;
    } else if (serverAPP.hasArg("animacao_8")) {
      animacaoAtual = 8;
    } else if (serverAPP.hasArg("animacao_9")) {
      animacaoAtual = 9;
    }
  }

  serverAPP.sendHeader("Location", String("/"), true);
  serverAPP.send(302, "text/plain", "");
  
}






