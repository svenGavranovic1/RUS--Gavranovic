# RUS Lab 1 — Prekidi i HC‑SR04 (ESP32)

Ovaj projekt demonstrira rad s prekidima na ESP32 mikrokontroleru.  
Implementirani su:

- prekidi tipkala (INT0, INT1, INT2)
- hardverski timer (500 ms)
- ultrazvučni senzor HC‑SR04 (prekid na ECHO pinu)
- vizualizacija svih događaja u Serial Plotteru

Projekt je organiziran modularno, svaki podsustav ima svoj `.c` i `.h` modul, a `main.c` služi kao centralna aplikacijska logika.

## Struktura projekta

/src
main.c
buttons.c
buttons.h
timer.c
timer.h
hcsr04.c
hcsr04.h
plotter.c
plotter.h
interrupts.c
interrupts.h

/wokwi
main.ino   ← verzija za simulaciju u Wokwi-ju

## Control Flow Graph (CFG) - Logika unutar loop()

flowchart TD
    Start((Loop Start)) --> Trig{200ms?}
    Trig -- Yes --> SendTrig[hcsr04_trigger] --> DistFlag
    Trig -- No --> DistFlag{flag_distance?}
    
    DistFlag -- True --> Alert[LED_ALERT On/Off] --> Int0Flag
    DistFlag -- False --> Int0Flag{flag_int0?}
    
    Int0Flag -- True --> Toggle0[Toggle LED_INT0] --> Int1Flag
    Int0Flag -- False --> Int1Flag{flag_int1?}
    
    Int1Flag -- True --> Toggle1[Toggle LED_INT1] --> Int2Flag
    Int1Flag -- False --> Int2Flag{flag_int2?}
    
    Int2Flag -- True --> Toggle2[Toggle LED_INT2] --> TimerFlag
    Int2Flag -- False --> TimerFlag{flag_timer?}
    
    TimerFlag -- True --> ToggleT[Toggle LED_TIMER] --> PlotCheck
    TimerFlag -- False --> PlotCheck{50ms?}
    
    PlotCheck -- Yes --> Plot[plotData] --> End((Loop End))
    PlotCheck -- No --> End

## Sequence Diagram (UML) - Interakcija Hardvera

sequenceDiagram
    participant User as Korisnik/Prepreka
    participant Sensor as HC-SR04 / Tipkalo
    participant ISR as ESP32 ISR (Prekid)
    participant Loop as ESP32 Main Loop

    Note over User, Sensor: Fizički događaj
    User->>Sensor: Pritisak tipkala / Odjek ultrazvuka
    Sensor->>ISR: Električni signal (Pin Change)
    
    Note right of ISR: Prekid procesora!
    ISR->>ISR: Postavi flag = true
    ISR-->>Loop: Povratak u normalan rad
    
    Note over Loop: Sljedeća iteracija petlje
    Loop->>Loop: Provjera flag-a
    Loop->>Sensor: Akcija (Upali LED)

Code

---

## Opis modula

### **buttons.c / buttons.h**
- ISR‑ovi za tipkala (INT0, INT1, INT2)
- postavljanje zastavica prekida
- “pulse hold” mehanizam za Serial Plotter
- obrada tipkala i upravljanje LED‑icama

### **timer.c / timer.h**
- konfiguracija hardverskog timera
- ISR timera (svakih 500 ms)
- obrada timer događaja i LED indikacija

### **hcsr04.c / hcsr04.h**
- ISR za ECHO pin (CHANGE prekid)
- izračun udaljenosti
- trigger funkcija
- obrada udaljenosti i LED upozorenja

### **plotter.c / plotter.h**
- formatirani ispis podataka u Serial Plotter
- udaljenost + INT0/1/2 + timer (svaki s offsetom radi preglednosti)

### **interrupts.c / interrupts.h**
- centralna registracija svih prekida
- attachInterrupt() za tipkala i HC‑SR04
- inicijalizacija hardverskog timera

### **main.c**
- inicijalizacija sustava
- glavni loop
- periodičko triganje HC‑SR04
- obrada svih događaja
- slanje podataka u Serial Plotter

---

##  Serial Plotter vizualizacija

Projekt šalje sljedeće signale:

| Signal | Opis | Vrijednosti |
|--------|------|-------------|
| `dist` | udaljenost u cm | 0–100 |
| `int0` | prekid tipkala 0 | 10 / 11 |
| `int1` | prekid tipkala 1 | 20 / 21 |
| `int2` | prekid tipkala 2 | 30 / 31 |
| `timer` | timer prekid | 40 / 41 |

Offseti omogućuju da se sve linije jasno vide u Serial Plotteru.

---

## 🛠️ Pokretanje projekta

### **Wokwi simulacija**
U folderu `/wokwi` nalazi se `main.ino` koji je spreman za pokretanje u Wokwi‑ju.

### **ESP32 (stvarni uređaj)**
1. Otvoriti projekt u Arduino IDE-u  
2. Uključiti sve `.c` i `.h` datoteke iz `/src`  
3. Odabrati ESP32 Dev Module  
4. Upload

---

## Dokumentacija (Doxygen)

Projekt je u potpunosti dokumentiran Doxygen anotacijama i nalazi se na sljedećoj poveznici: https://svengavranovic1.github.io/RUS--Gavranovic/files.html


## Autor

Sven  
Tehničko Veleučilište u Zagrebu
Kolegij: RUS — Razvoj ugradbenih sustava
