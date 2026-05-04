# Izvještaj – Lab2

## 1. Cilj zadatka

Cilj zadatka je proučiti mogućnosti smanjenja potrošnje energije na odabranom mikrokontroleru korištenjem sleep režima. U ovom rješenju naglasak je na logici upravljanja energijom, a ne na preciznom mjerenju potrošnje jer Wokwi simulator ne simulira stvarnu potrošnju energije.

## 2. Odabrana platforma i varijanta

- Platforma: ESP32
- Simulator: Wokwi
- Varijanta: B – Datalogger okoliša
- Sleep mode: Deep Sleep
- Primarni mehanizam buđenja: timer wake-up
- Dodatni mehanizam buđenja: GPIO/EXT0 wake-up tipkalom

## 3. Opis implementacije

Program je organiziran u odvojene cjeline:

- `handleWakeup()` – obrada nakon buđenja, očitanje senzora i spremanje mjerenja
- `activePhase()` – aktivna faza rada u kojoj LED svijetli 3 sekunde
- `enterDeepSleep()` – konfiguracija timer i GPIO wake-up mehanizma te ulazak u Deep Sleep
- `isWakeButtonPressedStable()` – software debounce provjera tipkala

Sustav se nakon pokretanja ili buđenja ponaša na sljedeći način:

1. učitava spremljeno stanje iz NVS memorije zbog Wokwi kompatibilnosti
2. evidentira broj pokretanja/buđenja
3. provjerava je li tipkalo stabilno pritisnuto
4. ispisuje razlog buđenja
5. očitava temperaturu i vlagu s DHT22 senzora
6. sprema mjerenje u RTC memoriju, a u Wokwi simulaciji dodatno u NVS/Preferences
7. nakon 10 mjerenja ispisuje spremljene podatke i resetira spremnik
8. pali LED tijekom aktivne faze od 3 sekunde
9. konfigurira sljedeći timer wake-up interval i GPIO wake-up tipkalom
10. u Wokwi demonstraciji prikazuje vidljivu sleep pauzu s ugašenom LED diodom
11. ulazi u Deep Sleep

## 4. Sleep mode

Korišten je ESP32 Deep Sleep. U ovom režimu glavni procesor prestaje s radom, a dio RTC memorije ostaje očuvan. Zbog toga se kroz više sleep/wake ciklusa na stvarnom ESP32 mogu čuvati varijable označene s `RTC_DATA_ATTR`.

U Wokwi simulatoru je uočeno da se nakon `esp_deep_sleep_start()` ponekad dobije reset simuliranog čipa umjesto potpunog očuvanja RTC stanja. Zato je u kod dodana biblioteka `Preferences`, odnosno NVS pohrana, kako bi se u simulatoru mogli jasno dokazati rast brojača i spremanje 10 mjerenja. Ovo je simulacijska prilagodba, dok je konceptualno rješenje i dalje temeljeno na RTC memoriji i Deep Sleep načinu rada.

U projektu se u RTC memoriji čuvaju:

- broj pokretanja/buđenja
- broj spremljenih mjerenja
- broj buđenja/događaja tipkalom
- polje temperatura
- polje vlaga
- zadnji odabrani sleep interval

## 5. Buđenje sustava

Buđenje je izvedeno na dva načina.

### 5.1 Timer wake-up

Primarni način buđenja je timer:

```cpp
esp_sleep_enable_timer_wakeup(sleepTimeUs);
```

Interval nije fiksan, nego se prije svakog ulaska u sleep odabire vrijednost između 20 i 40 sekundi. Time se demonstrira da se timer wake-up može konfigurirati za svaki ciklus rada, a interval je dovoljno kratak za praktično testiranje u Wokwi simulatoru.

### Zašto je timer interval postavljen na 20–40 s

Timer wake-up namjerno je postavljen na dulji varijabilni interval kako bi se u Wokwi simulaciji jasno vidjela razlika između periodičkog buđenja i događajno vođenog buđenja. Ako korisnik pritisne tipkalo prije isteka timera, sustav se budi ranije i evidentira event-driven događaj.


### 5.2 GPIO/EXT0 wake-up tipkalom

Dodatno je dodano event-driven buđenje tipkalom:

```cpp
esp_sleep_enable_ext0_wakeup(GPIO_NUM_4, 0);
```

Tipkalo je spojeno na D4/GPIO4 i GND, uz interni pull-up otpornik (`INPUT_PULLUP`). Pritisnuto tipkalo daje logičku nulu (`LOW`), pa se EXT0 wake-up konfigurira na razinu 0.

## 6. Debouncing tipkala

Mehanička tipkala mogu generirati više kratkih promjena stanja prilikom jednog fizičkog pritiska. To se naziva odbijanje kontakta ili debounce problem. Posljedica može biti višestruka detekcija jednog događaja, što povećava broj nepotrebnih buđenja i time potrošnju energije.

U ovom rješenju koristi se software debounce. Nakon detekcije da je tipkalo u stanju `LOW`, program čeka 50 ms i ponovno provjerava pin. Događaj se evidentira samo ako je pin i nakon tog vremena i dalje u stanju `LOW`.

```cpp
bool isWakeButtonPressedStable() {
  if (digitalRead(WAKE_BUTTON_PIN) != LOW) return false;
  unsigned long startTime = millis();
  while (millis() - startTime < DEBOUNCE_MS) yield();
  return digitalRead(WAKE_BUTTON_PIN) == LOW;
}
```

Ovo sprječava da se jedan fizički pritisak interpretira kao više logičkih događaja.

## 7. Senzor i mjerenja

U Wokwi simulatoru dodan je DHT22 senzor. Program očitava temperaturu i vlagu. Ako senzor iz nekog razloga ne vrati valjane podatke, koristi se simulirana fallback vrijednost kako bi logika dataloggera nastavila raditi.

Zadnjih 10 mjerenja sprema se u RTC varijable. Radi stabilnog prikaza u Wokwi simulatoru isti se podaci dodatno spremaju u NVS memoriju pomoću `Preferences`. Nakon desetog mjerenja podaci se ispisuju u serijski monitor i spremnik se resetira.

## 8. Aktivna faza rada

LED dioda spojena na D2/GPIO2 signalizira aktivnu fazu rada. LED svijetli 3 sekunde. Za upravljanje vremenom koristi se `millis()`, a ne samo `delay()` kao glavni mehanizam.

## 9. Upravljanje energijom

Nakon završetka aktivnog zadatka sustav odmah priprema ulazak u Deep Sleep. Prije ulaska u sleep:

- LED se isključuje
- konfigurira se timer buđenje
- konfigurira se GPIO/EXT0 buđenje tipkalom
- u Wokwi demonstraciji izvodi se vidljiva sleep pauza s ugašenom LED diodom
- sprema se stanje u NVS zbog ograničenja Wokwi simulacije
- završava se serijski ispis pomoću `Serial.flush()`

Nakon buđenja sustav ponovno inicijalizira potrebne module i nastavlja rad koristeći sačuvane podatke. Na stvarnom ESP32 to je RTC memorija, a u Wokwi demonstraciji dodatno se koristi NVS.

### 9.1 Wokwi vizualna sleep demonstracija

Budući da Wokwi u ovom projektu ne prikazuje pouzdano stvarno trajanje ESP32 Deep Sleepa, dodana je funkcija `visualSleepForWokwiDemo()`. Tijekom te faze LED je ugašena, a program čeka najviše 15 sekundi ili ranije prekida čekanje ako korisnik pritisne tipkalo. Planirani deep sleep interval i dalje ostaje varijabilan 20–40 sekundi, ali je vizualna pauza skraćena radi praktičnog testiranja. Ova funkcija služi samo za demonstraciju i dokaz rada u simulatoru. Stvarni Deep Sleep i dalje se konfigurira funkcijama `esp_sleep_enable_timer_wakeup()`, `esp_sleep_enable_ext0_wakeup()` i pokreće pozivom `esp_deep_sleep_start()`.

## 10. Usporedba sleep režima ESP32

| Režim | Logičko ponašanje | Vrijeme buđenja | Mogućnosti |
|---|---|---|---|
| Light Sleep | CPU se pauzira, RAM se zadržava | kraće | brže buđenje, manja ušteda od Deep Sleep |
| Deep Sleep | CPU se gasi, RTC memorija može ostati očuvana | dulje od Light Sleep | dobra ušteda energije, timer/GPIO/touch wake-up |
| Hibernation | isključuje se još više dijelova sustava | najdulje | najveća ušteda, ograničeno očuvanje stanja |

Za ovaj zadatak odabran je Deep Sleep jer je prikladan za datalogger koji većinu vremena miruje, a povremeno se budi za mjerenje ili na vanjski događaj.

## 11. Ograničenja Wokwi simulatora

Wokwi omogućuje testiranje logike programa, serijskog ispisa, senzora, tipkala i sleep/wake ciklusa. Međutim, ne omogućuje precizno mjerenje stvarne potrošnje energije. U ovoj simulaciji dodatno je uočeno da se nakon Deep Sleep poziva može prikazati reset simuliranog čipa, zbog čega RTC stanje nije uvijek pouzdano očuvano i stvarno trajanje Deep Sleepa nije vidljivo kao na fizičkom ESP32. Zato se za dokaz rada u simulatoru koristi NVS/Preferences pohrana i dodana je vizualna sleep pauza prije poziva `esp_deep_sleep_start()`. Zbog navedenih ograničenja u radu se ne donosi stvarna procjena potrošnje, nego samo teorijska analiza.

Na stvarnom ESP32 mikrokontroleru nakon prvog ciklusa očekivani razlog buđenja bio bi `ESP_SLEEP_WAKEUP_TIMER` ili `ESP_SLEEP_WAKEUP_EXT0`. U Wokwi simulatoru `esp_deep_sleep_start()` može rezultirati reset ciklusom simuliranog čipa, pa funkcija `esp_sleep_get_wakeup_cause()` ne mora vratiti stvarni wake-up razlog. Zato kod nakon prvog ciklusa ispisuje simulirani razlog buđenja, a stanje se za potrebe simulacije dodatno sprema u NVS (`Preferences`).

## 12. Zaključak

Implementacija prikazuje logiku upravljanja energijom na ESP32 mikrokontroleru. Sustav se periodički budi timerom ili događajno pomoću tipkala, izvršava kratku aktivnu fazu, sprema podatke u RTC memoriju, za Wokwi ih dodatno sprema u NVS i zatim se vraća u Deep Sleep.

Obavezan zaključak: implementacija prikazuje logiku upravljanja energijom, ali ne omogućuje stvarnu procjenu potrošnje energije. Za stvarnu analizu potrebno je koristiti fizičku hardversku platformu i odgovarajuće mjerne instrumente.
