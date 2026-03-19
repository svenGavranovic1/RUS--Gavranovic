/**
 * @file buttons.c
 * @brief Implementacija prekida i obrade tipkala.
 */

#include "buttons.h"

volatile bool flag_int0 = false;
volatile bool flag_int1 = false;
volatile bool flag_int2 = false;

unsigned long int0_hold_until = 0;
unsigned long int1_hold_until = 0;
unsigned long int2_hold_until = 0;

portMUX_TYPE buttonsMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR isr_int0() {
    portENTER_CRITICAL_ISR(&buttonsMux);
    flag_int0 = true;
    portEXIT_CRITICAL_ISR(&buttonsMux);
    int0_hold_until = millis() + 100;
}

void IRAM_ATTR isr_int1() {
    portENTER_CRITICAL_ISR(&buttonsMux);
    flag_int1 = true;
    portEXIT_CRITICAL_ISR(&buttonsMux);
    int1_hold_until = millis() + 100;
}

void IRAM_ATTR isr_int2() {
    portENTER_CRITICAL_ISR(&buttonsMux);
    flag_int2 = true;
    portEXIT_CRITICAL_ISR(&buttonsMux);
    int2_hold_until = millis() + 100;
}

void buttons_handle() {
    if (flag_int0) {
        portENTER_CRITICAL(&buttonsMux);
        flag_int0 = false;
        portEXIT_CRITICAL(&buttonsMux);
        digitalWrite(39, !digitalRead(39));
    }

    if (flag_int1) {
        portENTER_CRITICAL(&buttonsMux);
        flag_int1 = false;
        portEXIT_CRITICAL(&buttonsMux);
        digitalWrite(26, !digitalRead(26));
    }

    if (flag_int2) {
        portENTER_CRITICAL(&buttonsMux);
        flag_int2 = false;
        portEXIT_CRITICAL(&buttonsMux);
        digitalWrite(38, !digitalRead(38));
    }
}
