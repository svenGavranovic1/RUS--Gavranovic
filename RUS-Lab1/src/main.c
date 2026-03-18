/**
 * @file main.c
 * @brief Minimalni primjer za ESP32 prekid tipkala.
 */

#include <Arduino.h>

#define BUTTON_PIN 15
#define LED_PIN 2

volatile bool buttonPressed = false;

/**
 * @brief ISR za tipkalo (najviši prioritet).
 */
void IRAM_ATTR buttonISR() {
    buttonPressed = true;
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // attachInterrupt(pin, funkcija, edge)
    attachInterrupt(BUTTON_PIN, buttonISR, FALLING);
}

void loop() {
    if (buttonPressed) {
        buttonPressed = false;
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    }
}