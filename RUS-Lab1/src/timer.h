/**
 * @file timer.h
 * @brief Hardverski timer i pripadajući prekid.
 */

#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

extern volatile bool flag_timer;
extern unsigned long timer_hold_until;

/**
 * @brief Inicijalizira hardverski timer.
 */
void setupTimer();

/**
 * @brief ISR za hardverski timer.
 */
void IRAM_ATTR onTimer();

/**
 * @brief Obrada događaja timera nakon prekida.
 */
void timer_handle();

#endif
