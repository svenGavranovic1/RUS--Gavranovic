/**
 * @file interrupts.c
 * @brief Implementacija centralne registracije prekida.
 */

#include "interrupts.h"
#include "buttons.h"
#include "timer.h"
#include "hcsr04.h"

void interrupts_init(void)
{
    attachInterrupt(15, isr_int0, FALLING);
    attachInterrupt(4,  isr_int1, FALLING);
    attachInterrupt(5,  isr_int2, FALLING);

    attachInterrupt(14, echo_isr, CHANGE);

    setupTimer();
}
