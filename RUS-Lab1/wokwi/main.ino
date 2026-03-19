/**
   @file main.ino
   @brief RUS_Lab1_prekidi - Wokwi test verzija (ESP32 core 3.3.7)
*/
#include <Arduino.h>
// PINOVI
#define BTN_INT0_PIN   15
#define BTN_INT1_PIN   4
#define BTN_INT2_PIN   5

#define LED_INT0_PIN   39
#define LED_INT1_PIN   26
#define LED_INT2_PIN   38
#define LED_TIMER_PIN  2
#define LED_ALERT_PIN  33

#define HCSR04_TRIG_PIN 12
#define HCSR04_ECHO_PIN 14

// TIMER
hw_timer_t *timer0 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// ZASTAVICE DOGAĐAJA
volatile bool flag_int0 = false;
volatile bool flag_int1 = false;
volatile bool flag_int2 = false;
volatile bool flag_timer = false;
volatile bool flag_distance = false;

// HOLD MEHANIZAM ZA SERIAL PLOTTER
unsigned long int0_hold_until = 0;
unsigned long int1_hold_until = 0;
unsigned long int2_hold_until = 0;
unsigned long timer_hold_until = 0;

// HC-SR04 mjerenje
volatile unsigned long echo_rising_time = 0;
volatile unsigned long echo_falling_time = 0;
volatile bool echo_high = false;
volatile float last_distance_cm = 0.0f;

// Kritična sekcija
portMUX_TYPE isrMux = portMUX_INITIALIZER_UNLOCKED;

// ---------------------------------------------------------
// SERIAL PLOTTER FUNKCIJA
// ---------------------------------------------------------
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

// ---------------------------------------------------------
// ISR za tipkala
// ---------------------------------------------------------

void IRAM_ATTR isr_int0() {
  portENTER_CRITICAL_ISR(&isrMux);
  flag_int0 = true;
  portEXIT_CRITICAL_ISR(&isrMux);

  int0_hold_until = millis() + 100;
}

void IRAM_ATTR isr_int1() {
  portENTER_CRITICAL_ISR(&isrMux);
  flag_int1 = true;
  portEXIT_CRITICAL_ISR(&isrMux);

  int1_hold_until = millis() + 100;
}

void IRAM_ATTR isr_int2() {
  portENTER_CRITICAL_ISR(&isrMux);
  flag_int2 = true;
  portEXIT_CRITICAL_ISR(&isrMux);

  int2_hold_until = millis() + 100;
}

// ---------------------------------------------------------
// TIMER ISR
// ---------------------------------------------------------

void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  flag_timer = true;
  portEXIT_CRITICAL_ISR(&timerMux);

  timer_hold_until = millis() + 100;
}

// ---------------------------------------------------------
// HC-SR04 ECHO ISR
// ---------------------------------------------------------

void IRAM_ATTR echo_isr() {
  unsigned long now = micros();
  bool level = digitalRead(HCSR04_ECHO_PIN);

  if (level) {
    echo_rising_time = now;
    echo_high = true;
  } else {
    if (echo_high) {
      echo_falling_time = now;
      echo_high = false;

      unsigned long pulse = echo_falling_time - echo_rising_time;
      float distance = (float)pulse / 58.0f;
      last_distance_cm = distance;

      if (distance < 100.0f) {
        portENTER_CRITICAL_ISR(&isrMux);
        flag_distance = true;
        portEXIT_CRITICAL_ISR(&isrMux);
      }
    }
  }
}

void hcsr04_trigger() {
  digitalWrite(HCSR04_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(HCSR04_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(HCSR04_TRIG_PIN, LOW);
}

// ---------------------------------------------------------
// TIMER SETUP
// ---------------------------------------------------------

void setupTimer() {
  timer0 = timerBegin(0, 80, true);
  timerAttachInterrupt(timer0, &onTimer, true);
  timerAlarmWrite(timer0, 500000, true);  // 500 ms
  timerAlarmEnable(timer0);
}

// ---------------------------------------------------------
// SETUP
// ---------------------------------------------------------

void setup() {
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

  attachInterrupt(BTN_INT0_PIN, isr_int0, FALLING);
  attachInterrupt(BTN_INT1_PIN, isr_int1, FALLING);
  attachInterrupt(BTN_INT2_PIN, isr_int2, FALLING);

  attachInterrupt(HCSR04_ECHO_PIN, echo_isr, CHANGE);

  setupTimer();

  Serial.println("RUS_Lab1_prekidi - start");
}

// ---------------------------------------------------------
// LOOP
// ---------------------------------------------------------

void loop() {
  // Periodički trig za HC-SR04
  static unsigned long lastTrig = 0;
  unsigned long now = millis();
  if (now - lastTrig > 200) {
    lastTrig = now;
    hcsr04_trigger();
  }

  // PRIORITETI:
  // 1) udaljenost
  // 2) INT0
  // 3) INT1
  // 4) INT2
  // 5) TIMER

  if (flag_distance) {
    portENTER_CRITICAL(&isrMux);
    flag_distance = false;
    portEXIT_CRITICAL(&isrMux);

    digitalWrite(LED_ALERT_PIN, HIGH);
    delay(50);
    digitalWrite(LED_ALERT_PIN, LOW);
  }

  if (flag_int0) {
    portENTER_CRITICAL(&isrMux);
    flag_int0 = false;
    portEXIT_CRITICAL(&isrMux);

    digitalWrite(LED_INT0_PIN, !digitalRead(LED_INT0_PIN));
  }

  if (flag_int1) {
    portENTER_CRITICAL(&isrMux);
    flag_int1 = false;
    portEXIT_CRITICAL(&isrMux);

    digitalWrite(LED_INT1_PIN, !digitalRead(LED_INT1_PIN));
  }

  if (flag_int2) {
    portENTER_CRITICAL(&isrMux);
    flag_int2 = false;
    portEXIT_CRITICAL(&isrMux);

    digitalWrite(LED_INT2_PIN, !digitalRead(LED_INT2_PIN));
  }

  if (flag_timer) {
    portENTER_CRITICAL(&timerMux);
    flag_timer = false;
    portEXIT_CRITICAL(&timerMux);

    digitalWrite(LED_TIMER_PIN, !digitalRead(LED_TIMER_PIN));
  }

  // PLOTTER OUTPUT (svakih 50 ms)
  static unsigned long lastPlot = 0;
  if (millis() - lastPlot > 50) {
    lastPlot = millis();
    plotData();
  }
}
