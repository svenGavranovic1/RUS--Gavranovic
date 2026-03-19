/**
 * @file plotter.c
 * @brief Implementacija Serial Plotter vizualizacije.
 */

#include "plotter.h"
#include "buttons.h"
#include "timer.h"
#include "hcsr04.h"

void plotData() {
    unsigned long now = millis();

    Serial.print("dist:");
    Serial.print(last_distance_cm);

    Serial.print(" int0:");
    Serial.print(now < int0_hold_until ? 11 : 10);

    Serial.print(" int1:");
    Serial.print(now < int1_hold_until ? 21 : 20);

    Serial.print(" int2:");
    Serial.print(now < int2_hold_until ? 31 : 30);

    Serial.print(" timer:");
    Serial.println(now < timer_hold_until ? 41 : 40);
}
