#ifndef APP_TORRE_H
#define APP_TORRE_H

#include <Arduino.h>

struct App_torre
{
 uint8_t CorEntrada[5];  
 int BrilhoEntrada[5]; 
 uint8_t AnimaEntrada[5];
 bool BuzzerEntrada[5];
};

struct Torre_HTML_t{
 String CorEntrada[5];
 String AnimaEntrada[5]; 
 String BuzzerEntrada[5];
 String BrilhoEntrada[5];
};

struct EntradasDigitais_t{
  bool Entrada1 = false;
  bool Entrada2 = false;
  bool Entrada3 = false;
  bool Entrada4 = false;
  bool Entrada5 = false;
};


void APP_TORRE_init(void);
void APP_TORRE_run(void);
void APP_TORRE_controllerTestTorre(void);
void APP_TORRE_controllerEntradaDigital(void);

void APP_TORRE_ReadEntradas(void);

void salvarAppTorreNaEEPROM(const App_torre& dados);
void lerAppTorreDaEEPROM(App_torre& dados);

void APP_TORRE_TratamentoEntrada1(void);
void APP_TORRE_TratamentoEntrada2(void);
void APP_TORRE_TratamentoEntrada3(void);
void APP_TORRE_TratamentoEntrada4(void);
void APP_TORRE_TratamentoEntrada5(void);

void APP_ATUALIZA_DadosHTML(void);

void handleRoot();
void handleConfigurar();
void handleAnimacao();

// ---------  animações ---------
void acenderPartes();
void LedaLedTorre();
void EncheColunaTorre();
void enchendoTorre();
void colorWipeAnimation();
void fadeColorAnimation();
void fadeAnimation();
void giroflexTorre();
void giroflexTorre2();
void linhaTorre();



#endif