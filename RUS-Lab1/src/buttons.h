/**
 * @file buttons.h
 * @brief Rukovanje prekidima tipkala (INT0, INT1, INT2).
 *
 * Ovaj modul sadrži ISR rutine za tipkala te funkciju za obradu
 * događaja nakon prekida. Također implementira "pulse hold" mehanizam
 * za vizualizaciju prekida u Serial Plotteru.
 */

#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

extern volatile bool flag_int0;
extern volatile bool flag_int1;
extern volatile bool flag_int2;

extern unsigned long int0_hold_until;
extern unsigned long int1_hold_until;
extern unsigned long int2_hold_until;

/**
 * @brief ISR za INT0 tipkalo.
 */
void IRAM_ATTR isr_int0();

/**
 * @brief ISR za INT1 tipkalo.
 */
void IRAM_ATTR isr_int1();

/**
 * @brief ISR za INT2 tipkalo.
 */
void IRAM_ATTR isr_int2();

/**
 * @brief Obrada događaja tipkala nakon prekida.
 */
void buttons_handle();

#endif
