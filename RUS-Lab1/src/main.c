/**
 * @file main.c
 * @brief Glavni program za Lab1 - višestruki prekidi.
 */

#include "interrupts.h"
#include "timer.h"
#include "sensor.h"
#include "uart.h"
#include "buttons.h"

int main() {
    // inicijalizacija sustava
    initInterrupts();
    initTimer();
    initSensor();
    initUART();
    initButtons();

    while (1) {
        // glavna petlja
        // ovdje se obrađuju flagovi iz ISR-ova
    }
}