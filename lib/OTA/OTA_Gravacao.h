#ifndef OTA_GRAVACAOH
#define OTA_GRAVACAO_H
#include "Arduino.h"

void OTA_init(void);
void OTA_handleUpload(void);

void OTA_debug(const String& message);
void OTA_CleanDebug();

uint8_t OTA_GetStateTest(void);

#endif