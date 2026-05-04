/**
 * Lab2 – Upravljanje potrošnjom energije mikrokontrolera
 * Varijanta B: Datalogger okoliša s periodičkim buđenjem
 * Platforma: ESP32, Wokwi
 *
 * Funkcionalnost:
 * - ESP32 se periodički budi pomoću timer wake-up mehanizma
 * - dodatno je omoguceno event-driven budenje tipkalom preko EXT0/GPIO wake-up mehanizma
 * - očitava DHT22 senzor u Wokwi simulatoru
 * - zadnjih 10 mjerenja čuva u RTC varijablama
 * - zbog ograničenja Wokwi simulatora stanje se dodatno sprema u NVS/Preferences
 * - nakon 10 mjerenja ispisuje podatke i resetira spremnik
 * - LED signalizira aktivnu fazu rada
 * - sleep interval je varijabilan radi demonstracije timer wake-up mehanizma
 * - Wokwi vizualni sleep skracen je na 15 s radi prakticnog testiranja tipkala
 */

#include <Arduino.h>
#include "esp_sleep.h"
#include "DHTesp.h"
#include <Preferences.h>

// ------------------------- Konfiguracija -------------------------
#define LED_PIN 2
#define DHT_PIN 15
#define WAKE_BUTTON_PIN 4
#define MIN_SLEEP_SEC 20ULL
#define MAX_SLEEP_SEC 40ULL
#define MAX_SAMPLES 10
#define ACTIVE_PHASE_MS 3000UL
#define DEBOUNCE_MS 50UL
#define WOKWI_VISUAL_SLEEP_ENABLED true
#define WOKWI_VISUAL_SLEEP_MS 15000UL

// ------------------------- RTC memorija --------------------------
// RTC_DATA_ATTR je predviđen za očuvanje podataka kroz Deep Sleep na stvarnom ESP32.
RTC_DATA_ATTR int bootCount = 0;
RTC_DATA_ATTR int sampleCount = 0;
RTC_DATA_ATTR int buttonWakeCount = 0;
RTC_DATA_ATTR float temperatures[MAX_SAMPLES];
RTC_DATA_ATTR float humidities[MAX_SAMPLES];
RTC_DATA_ATTR uint64_t lastSleepSeconds = 0;
RTC_DATA_ATTR int simulatedWakeSource = 0; // 0 = nepoznato/prvi start, 1 = timer, 2 = tipkalo

// ------------------------- Wokwi/NVS pohrana ---------------------
// U Wokwi simulatoru deep sleep ponekad završi resetom simuliranog čipa, pa RTC memorija
// nije uvijek očuvana. Preferences/NVS se koristi kao simulacijska prilagodba kako bi
// brojači i mjerenja bili vidljivi kroz više ciklusa.
Preferences prefs;

DHTesp dhtSensor;

// ------------------------- Pomoćne funkcije ----------------------

bool isWakeButtonPressedStable() {
  // Tipkalo je spojeno na GND, a koristi se interni pull-up.
  // LOW znaci da je tipkalo pritisnuto.
  if (digitalRead(WAKE_BUTTON_PIN) != LOW) {
    return false;
  }

  unsigned long startTime = millis();
  while (millis() - startTime < DEBOUNCE_MS) {
    yield();
  }

  return digitalRead(WAKE_BUTTON_PIN) == LOW;
}

String getWakeupReasonText(bool stableButtonPress) {
  esp_sleep_wakeup_cause_t wakeupReason = esp_sleep_get_wakeup_cause();

  switch (wakeupReason) {
    case ESP_SLEEP_WAKEUP_TIMER:
      return "Timer wake-up";
    case ESP_SLEEP_WAKEUP_EXT0:
      return "External GPIO wake-up EXT0";
    case ESP_SLEEP_WAKEUP_EXT1:
      return "External GPIO wake-up EXT1";
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      return "Touch wake-up";
    case ESP_SLEEP_WAKEUP_ULP:
      return "ULP wake-up";
    default:
      // Na stvarnom ESP32 nakon deep sleepa očekuju se ESP_SLEEP_WAKEUP_TIMER ili ESP_SLEEP_WAKEUP_EXT0.
      // U Wokwi simulatoru deep sleep se često prikaže kao reset simulacije, pa se
      // za demonstraciju koristi simulatedWakeSource spremljen u NVS prije reset ciklusa.
      if (bootCount > 1 && (stableButtonPress || simulatedWakeSource == 2)) {
        return "External GPIO wake-up tipkalom simuliran putem reset ciklusa u Wokwi";
      }
      if (bootCount > 1 && simulatedWakeSource == 1) {
        return "Timer wake-up simuliran putem reset ciklusa u Wokwi";
      }
      if (bootCount > 1) {
        return "Nastavak nakon simuliranog sleep/reset ciklusa u Wokwi";
      }
      return "Prvo pokretanje ili reset simulacije";
  }
}

void loadStateFromNVS() {
  bootCount = prefs.getInt("bootCount", 0);
  sampleCount = prefs.getInt("sampleCount", 0);
  buttonWakeCount = prefs.getInt("buttonWake", 0);
  lastSleepSeconds = prefs.getULong64("lastSleep", 0);
  simulatedWakeSource = prefs.getInt("simWake", 0);

  if (sampleCount < 0 || sampleCount > MAX_SAMPLES) {
    sampleCount = 0;
  }

  for (int i = 0; i < MAX_SAMPLES; i++) {
    char tempKey[8];
    char humKey[8];
    snprintf(tempKey, sizeof(tempKey), "t%d", i);
    snprintf(humKey, sizeof(humKey), "h%d", i);

    temperatures[i] = prefs.getFloat(tempKey, 0.0);
    humidities[i] = prefs.getFloat(humKey, 0.0);
  }

  Serial.println("STATE: Stanje ucitano iz NVS memorije (Wokwi kompatibilnost).");
}

void saveStateToNVS() {
  prefs.putInt("bootCount", bootCount);
  prefs.putInt("sampleCount", sampleCount);
  prefs.putInt("buttonWake", buttonWakeCount);
  prefs.putULong64("lastSleep", lastSleepSeconds);
  prefs.putInt("simWake", simulatedWakeSource);

  for (int i = 0; i < MAX_SAMPLES; i++) {
    char tempKey[8];
    char humKey[8];
    snprintf(tempKey, sizeof(tempKey), "t%d", i);
    snprintf(humKey, sizeof(humKey), "h%d", i);

    prefs.putFloat(tempKey, temperatures[i]);
    prefs.putFloat(humKey, humidities[i]);
  }

  Serial.println("STATE: Stanje spremljeno u NVS memoriju prije sleepa.");
}

void activePhase() {
  Serial.println("ACTIVE: LED ukljucen 3 s, obrada mjerenja u tijeku...");

  digitalWrite(LED_PIN, HIGH);
  unsigned long startTime = millis();

  while (millis() - startTime < ACTIVE_PHASE_MS) {
    yield();
  }

  digitalWrite(LED_PIN, LOW);
  Serial.println("ACTIVE: Aktivna faza zavrsena.");
}

TempAndHumidity readEnvironment() {
  TempAndHumidity data = dhtSensor.getTempAndHumidity();

  // Sigurnosna provjera: ako senzor ne vrati valjane podatke, koristi se simulirana vrijednost.
  if (isnan(data.temperature) || isnan(data.humidity)) {
    data.temperature = random(180, 310) / 10.0;
    data.humidity = random(350, 800) / 10.0;
    Serial.println("MEASURE: DHT22 nije vratio valjane podatke, koristim simulirano mjerenje.");
  }

  return data;
}

void storeMeasurement(float temperature, float humidity) {
  if (sampleCount < MAX_SAMPLES) {
    temperatures[sampleCount] = temperature;
    humidities[sampleCount] = humidity;
    sampleCount++;
  }
}

void printMeasurements() {
  Serial.println("BUFFER: Prikupljeno je 10 mjerenja.");
  Serial.println("BUFFER: Ispis zadnjih 10 mjerenja:");
  Serial.println("Br.\tTemperatura [C]\tVlaga [%]");

  for (int i = 0; i < MAX_SAMPLES; i++) {
    Serial.print(i + 1);
    Serial.print("\t");
    Serial.print(temperatures[i], 1);
    Serial.print("\t\t");
    Serial.println(humidities[i], 1);
  }
}

void resetMeasurementBuffer() {
  sampleCount = 0;

  for (int i = 0; i < MAX_SAMPLES; i++) {
    temperatures[i] = 0.0;
    humidities[i] = 0.0;
  }

  Serial.println("BUFFER: Spremnik resetiran.");
}

void handleWakeup() {
  bootCount++;

  bool stableButtonPress = isWakeButtonPressedStable();
  bool simulatedButtonWake = (simulatedWakeSource == 2);
  if (stableButtonPress || simulatedButtonWake) {
    buttonWakeCount++;
  }

  Serial.println("----------------------------------------");
  Serial.print("WAKE: Broj budenja/pokretanja: ");
  Serial.println(bootCount);
  Serial.print("WAKE: Razlog budenja: ");
  Serial.println(getWakeupReasonText(stableButtonPress));

  if (stableButtonPress || simulatedButtonWake) {
    Serial.print("EVENT: Tipkalo registrirano kao jedan logicki dogadaj nakon debounce/simulacijske provjere. Broj button dogadaja: ");
    Serial.println(buttonWakeCount);
  }

  if (lastSleepSeconds > 0) {
    Serial.print("WAKE: Prethodni sleep interval bio je ");
    Serial.print(lastSleepSeconds);
    Serial.println(" s.");
  }

  TempAndHumidity data = readEnvironment();

  Serial.print("MEASURE: Temperatura = ");
  Serial.print(data.temperature, 1);
  Serial.print(" C, Vlaga = ");
  Serial.print(data.humidity, 1);
  Serial.println(" %");

  storeMeasurement(data.temperature, data.humidity);

  Serial.print("STORE: Spremljeno mjerenje ");
  Serial.print(sampleCount);
  Serial.print("/");
  Serial.println(MAX_SAMPLES);

  if (sampleCount >= MAX_SAMPLES) {
    printMeasurements();
    resetMeasurementBuffer();
  }
}

uint64_t chooseNextSleepIntervalSeconds() {
  // Varijabilni sleep demonstrira da se timer wake-up moze konfigurirati prije svakog spavanja.
  // Interval je namjerno kraci radi prakticnog testiranja u Wokwi simulatoru.
  return random(MIN_SLEEP_SEC, MAX_SLEEP_SEC + 1);
}

void visualSleepForWokwiDemo() {
  if (!WOKWI_VISUAL_SLEEP_ENABLED) {
    simulatedWakeSource = 1;
    return;
  }

  Serial.println("SIM: Wokwi ne prikazuje pouzdano stvarno trajanje Deep Sleepa.");
  Serial.println("SIM: LED je ugasen tijekom ove demonstracijske sleep pauze.");
  Serial.println("SIM: Pritisni tipkalo na D4 za event-driven budenje prije isteka timera.");

  unsigned long startTime = millis();
  // Planirani deep sleep moze biti 20-40 s, ali vizualna Wokwi pauza je ogranicena
  // na 15 s da testiranje ne traje predugo.
  unsigned long plannedVisualSleepMs = (unsigned long)(lastSleepSeconds * 1000ULL);
  unsigned long visualSleepMs = min(plannedVisualSleepMs, WOKWI_VISUAL_SLEEP_MS);
  unsigned long lastPrint = 0;

  while (millis() - startTime < visualSleepMs) {
    digitalWrite(LED_PIN, LOW);

    if (isWakeButtonPressedStable()) {
      simulatedWakeSource = 2;
      Serial.println("SIM: Tipkalo je pritisnuto tijekom sleep faze.");
      Serial.println("SIM: Prekidam simulirani sleep i spremam button wake-up dogadaj.");
      return;
    }

    unsigned long elapsed = millis() - startTime;
    if (elapsed - lastPrint >= 5000UL) {
      lastPrint = elapsed;
      Serial.print("SIM: Sleep demonstracija traje, preostalo oko ");
      Serial.print((visualSleepMs - elapsed) / 1000UL);
      Serial.println(" s.");
    }

    yield();
  }

  simulatedWakeSource = 1;
  Serial.println("SIM: Timer interval je istekao u Wokwi demonstraciji.");
}

void enterDeepSleep() {
  Serial.println("POWER: Gasim nepotrebne periferije prije sleepa.");

  digitalWrite(LED_PIN, LOW);

  lastSleepSeconds = chooseNextSleepIntervalSeconds();
  uint64_t sleepTimeUs = lastSleepSeconds * 1000000ULL;

  // Mehanizam 1: periodičko buđenje timerom.
  esp_sleep_enable_timer_wakeup(sleepTimeUs);

  // Mehanizam 2: event-driven buđenje tipkalom.
  // GPIO4 je RTC GPIO pin, pa se može koristiti za EXT0 wake-up u Deep Sleep modu.
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0); // 0 = LOW, tipkalo pritisnuto prema GND

  Serial.print("SLEEP: Planirani Deep Sleep traje najvise ");
  Serial.print(lastSleepSeconds);
  Serial.println(" sekundi.");
  Serial.println("SLEEP: Budenje je moguce timerom ili pritiskom tipkala na D4/GPIO4.");
  Serial.println("SLEEP: Podaci su u RTC varijablama, a za Wokwi dodatno spremljeni u NVS.");

  // Simulacijski dodatak: Wokwi ne prikazuje uvijek stvarno trajanje ESP32 Deep Sleepa.
  // Ova pauza sluzi samo za vizualnu demonstraciju sleep faze i testiranje tipkala.
  visualSleepForWokwiDemo();

  saveStateToNVS();
  Serial.println("SLEEP: Pokrecem esp_deep_sleep_start().");
  Serial.flush();

  esp_deep_sleep_start();
}

// ------------------------- Arduino funkcije ----------------------

void setup() {
  Serial.begin(115200);
  delay(200); // kratko cekanje samo radi stabilizacije serijskog ispisa u Wokwi simulatoru

  prefs.begin("lab2", false);
  loadStateFromNVS();

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(WAKE_BUTTON_PIN, INPUT_PULLUP);

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  randomSeed(esp_random() ^ micros());

  handleWakeup();
  activePhase();
  enterDeepSleep();
}

void loop() {
  // Kod se ne izvrsava kontinuirano jer ESP32 nakon setup() ulazi u Deep Sleep.
}
