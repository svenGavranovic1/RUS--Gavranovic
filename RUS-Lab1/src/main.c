/**
 * @file main.c
 * @brief Glavni program za RUS_Lab1_prekidi (verzija za predaju profesoru).
 *
 * Ovaj modul povezuje sve ostale module:
 * - prekidi tipkala (buttons)
 * - hardverski timer (timer)
 * - ultrazvučni senzor HC-SR04 (hcsr04)
 * - Serial Plotter vizualizacija (plotter)
 * - centralna inicijalizacija prekida (interrupts)
 *
 * Napomena:
 *  - U Arduino okruženju stvarni main() generira core,
 *    a korisnik definira samo setup() i loop().
 *  - Ovaj main.c služi kao čista, modularna i dokumentirana
 *    prezentacija projekta za potrebe kolegija.
 */

#include <Arduino.h>
#include "buttons.h"
#include "timer.h"
#include "hcsr04.h"
#include "plotter.h"
#include "interrupts.h"

// PINOVI (isti kao u Wokwi projektu)
#define BTN_INT0_PIN    15
#define BTN_INT1_PIN    4
#define BTN_INT2_PIN    5

#define LED_INT0_PIN    39
#define LED_INT1_PIN    26
#define LED_INT2_PIN    38
#define LED_TIMER_PIN   2
#define LED_ALERT_PIN   33

#define HCSR04_TRIG_PIN 12
#define HCSR04_ECHO_PIN 14

/**
 * @brief Inicijalizacija svih periferija i modula.
 *
 * Ova funkcija se u Arduino okruženju poziva jednom pri pokretanju
 * programa. Postavlja pinove, serijsku komunikaciju i registrira
 * sve prekide preko interrupts_init().
 */
void setup(void)
{
    Serial.begin(115200);

    pinMode(LED_INT0_PIN, OUTPUT);
    pinMode(LED_INT1_PIN, OUTPUT);
    pinMode(LED_INT2_PIN, OUTPUT);
    pinMode(LED_TIMER_PIN, OUTPUT);
    pinMode(LED_ALERT_PIN, OUTPUT);

    pinMode(BTN_INT0_PIN, INPUT_PULLUP);
    pinMode(BTN_INT1_PIN, INPUT_PULLUP);
    pinMode(BTN_INT2_PIN, INPUT_PULLUP);

    pinMode(HCSR04_TRIG_PIN, OUTPUT);
    pinMode(HCSR04_ECHO_PIN, INPUT);

    interrupts_init();

    Serial.println("RUS_Lab1_prekidi - start");
}

/**
 * @brief Glavna petlja programa.
 *
 * U Arduino okruženju ova funkcija se poziva u beskonačnoj petlji.
 * Ovdje se:
 *  - periodički trigera HC-SR04 senzor
 *  - obrađuju događaji od senzora, tipkala i timera
 *  - periodički šalju podaci u Serial Plotter
 */
void loop(void)
{
    static unsigned long lastTrig = 0;
    static unsigned long lastPlot = 0;

    unsigned long now = millis();

    // Periodički trig za HC-SR04 (svakih ~200 ms)
    if (now - lastTrig > 200U) {
        lastTrig = now;
        hcsr04_trigger();
    }

    // Obrada događaja po prioritetima:
    // 1) udaljenost (HC-SR04)
    // 2) tipkala (INT0, INT1, INT2)
    // 3) timer
    hcsr04_handle();
    buttons_handle();
    timer_handle();

    // Ispis podataka za Serial Plotter (svakih ~50 ms)
    if (now - lastPlot > 50U) {
        lastPlot = now;
        plotData();
    }
}
