/**
 * @file interrupts.h
 * @brief Centralizirana inicijalizacija svih prekida.
 */

#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include <Arduino.h>

/**
 * @brief Registrira sve prekide u sustavu.
 */
void interrupts_init(void);

#endif
