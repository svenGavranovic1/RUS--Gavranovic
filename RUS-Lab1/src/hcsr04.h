/**
 * @file hcsr04.h
 * @brief Ultrazvučni senzor HC-SR04 i pripadajući prekid.
 */

#ifndef HCSR04_H
#define HCSR04_H

#include <Arduino.h>

extern volatile bool flag_distance;
extern volatile float last_distance_cm;

/**
 * @brief ISR za echo pin HC-SR04 senzora.
 */
void IRAM_ATTR echo_isr();

/**
 * @brief Generira trigger impuls za HC-SR04.
 */
void hcsr04_trigger();

/**
 * @brief Obrada udaljenosti nakon prekida.
 */
void hcsr04_handle();

#endif
