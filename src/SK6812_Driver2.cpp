#include "SK6812_Driver2.h"
#include <Arduino.h>

uint8_t _pin = 16;

SK6812::SK6812(uint16_t num_leds)
{
    _count_led = num_leds;
    _pixels = (RGBW *)malloc(_count_led * sizeof(RGBW));
    _pin = 16; // Defina o pino padrão como 0 (você pode alterar isso na função set_output)
}

SK6812::~SK6812()
{
    free(_pixels);
}

void SK6812::set_output(uint8_t pin)
{
    _pin = pin;
    pinMode(_pin, OUTPUT);
}

RGBW SK6812::get_rgbw(uint16_t index)
{
    RGBW px_value;

    if (index < _count_led) {
        px_value.r = _pixels[index].r;
        px_value.g = _pixels[index].g;
        px_value.b = _pixels[index].b;
        px_value.w = _pixels[index].w;
    }

    return px_value;
}

uint8_t SK6812::set_rgbw(uint16_t index, RGBW px_value)
{
    if (index < _count_led) {
        _pixels[index].r = px_value.r;
        _pixels[index].g = px_value.g;
        _pixels[index].b = px_value.b;
        _pixels[index].w = px_value.w;

        return 0;
    }

    return 1;
}

void SK6812::sync()
{
    // Calcula o ponteiro para o registrador de saída do pino
    volatile uint32_t *portRegister = portOutputRegister(digitalPinToPort(_pin));

    // Calcula o ponteiro para o registrador de modo do pino
    volatile uint32_t *portModeRegister = portModeRegister(digitalPinToPort(_pin));

    sendarray_mask((uint8_t *)_pixels, _count_led * sizeof(RGBW), 1 << _pin, (uint8_t *)portRegister, (uint8_t *)portModeRegister);
}

void SK6812::sendarray_mask(uint8_t *array, uint16_t length, uint8_t pinmask, uint8_t *port, uint8_t *portreg) {
        // Implementação da função sendarray_mask
    // Certifique-se de que você está usando os parâmetros pinmask, port e portreg conforme necessário
    // Você deve configurar o pino e o registrador corretos aqui para enviar os dados para os LEDs
    // A implementação exata depende do hardware e da biblioteca que você está usando para controlar os LEDs SK6812.
    // Certifique-se de consultar a documentação da biblioteca que você está usando ou verificar a especificação do hardware.

    // Exemplo de implementação para ESP8266:
    // Nota: Isso é apenas um exemplo, e o código real pode variar dependendo da sua configuração.
    noInterrupts(); // Desabilita interrupções
    uint8_t *end = array + length;
    while (array < end) {
        uint8_t b = *array++;
        for (uint8_t i = 0x80; i; i >>= 1) {
            *portreg = pinmask;
            if (b & i) {
                delayMicroseconds(4);
                *portreg = 0;
                delayMicroseconds(9);
            } else {
                delayMicroseconds(1);
                *portreg = 0;
                delayMicroseconds(10);
            }
        }
    }
    interrupts(); // Reabilita interrupções (se necessário)
}
