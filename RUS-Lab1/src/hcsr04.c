/**
 * @file hcsr04.c
 * @brief Implementacija HC-SR04 senzora i echo prekida.
 */

#include "hcsr04.h"

volatile bool flag_distance = false;
volatile float last_distance_cm = 0.0f;

volatile unsigned long echo_rising_time = 0;
volatile unsigned long echo_falling_time = 0;
volatile bool echo_high = false;

portMUX_TYPE echoMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR echo_isr() {
    unsigned long now = micros();
    bool level = digitalRead(14);

    if (level) {
        echo_rising_time = now;
        echo_high = true;
    } else {
        if (echo_high) {
            echo_falling_time = now;
            echo_high = false;

            unsigned long pulse = echo_falling_time - echo_rising_time;
            last_distance_cm = (float)pulse / 58.0f;

            if (last_distance_cm < 100.0f) {
                portENTER_CRITICAL_ISR(&echoMux);
                flag_distance = true;
                portEXIT_CRITICAL_ISR(&echoMux);
            }
        }
    }
}

void hcsr04_trigger() {
    digitalWrite(12, LOW);
    delayMicroseconds(2);
    digitalWrite(12, HIGH);
    delayMicroseconds(10);
    digitalWrite(12, LOW);
}

void hcsr04_handle() {
    if (flag_distance) {
        portENTER_CRITICAL(&echoMux);
        flag_distance = false;
        portEXIT_CRITICAL(&echoMux);

        digitalWrite(33, HIGH);
        delay(50);
        digitalWrite(33, LOW);
    }
}
