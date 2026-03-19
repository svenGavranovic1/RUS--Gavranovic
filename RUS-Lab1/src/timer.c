/**
 * @file timer.c
 * @brief Implementacija hardverskog timera i njegovog prekida.
 */

#include "timer.h"

hw_timer_t *timer0 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile bool flag_timer = false;
unsigned long timer_hold_until = 0;

void IRAM_ATTR onTimer() {
    portENTER_CRITICAL_ISR(&timerMux);
    flag_timer = true;
    portEXIT_CRITICAL_ISR(&timerMux);
    timer_hold_until = millis() + 100;
}

void setupTimer() {
    timer0 = timerBegin(0, 80, true);
    timerAttachInterrupt(timer0, &onTimer, true);
    timerAlarmWrite(timer0, 500000, true);
    timerAlarmEnable(timer0);
}

void timer_handle() {
    if (flag_timer) {
        portENTER_CRITICAL(&timerMux);
        flag_timer = false;
        portEXIT_CRITICAL(&timerMux);
        digitalWrite(2, !digitalRead(2));
    }
}
