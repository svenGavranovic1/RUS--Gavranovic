# Lab2 – Upravljanje potrošnjom energije mikrokontrolera

## Kratak opis rješenja

Projekt demonstrira upravljanje energetskim režimima na ESP32 mikrokontroleru u Wokwi simulatoru. Implementirana je **Varijanta B – datalogger okoliša**, uz dodatni event-driven mehanizam buđenja tipkalom. Sustav se periodički budi pomoću timera, očitava temperaturu i vlagu s DHT22 senzora, sprema zadnjih 10 mjerenja u RTC memoriju te nakon popunjavanja spremnika ispisuje podatke u serijski monitor.

Dodatno je implementirano:

* **varijabilni timer sleep interval** od 20 do 40 sekundi
* **GPIO/EXT0 wake-up tipkalom** na pinu D4/GPIO4
* **software debounce** za tipkalo
* **NVS/Preferences spremanje stanja** kao Wokwi-kompatibilna prilagodba

Zbog ograničenja Wokwi simulatora stanje se dodatno sprema u NVS memoriju pomoću `Preferences`, kako bi brojači i mjerenja bili vidljivi kroz više simulacijskih reset/sleep ciklusa.

## Wokwi link

Dodati poveznicu nakon izrade projekta:

**Wokwi:** TODO: dodati link na Wokwi projekt

## Sažetak

|Stavka|Odgovor|
|-|-|
|Platforma|ESP32|
|Varijanta|B – Datalogger okoliša + dodatni event-driven wake-up|
|Sleep mode|Deep Sleep|
|Buđenje|Timer wake-up + GPIO/EXT0 wake-up tipkalom|
|Timer funkcija|`esp\_sleep\_enable\_timer\_wakeup()`|
|GPIO wake-up funkcija|`esp\_sleep\_enable\_ext0\_wakeup(GPIO\_NUM\_4, 0)`|
|Čuvanje stanja|RTC memorija, `RTC\_DATA\_ATTR`; u Wokwi dodatno NVS/`Preferences`|
|Senzor|DHT22 u Wokwi simulatoru|
|Broj mjerenja|Zadnjih 10 mjerenja|
|Debouncing|Software debounce: stabilno LOW stanje tipkala 50 ms|
|Wokwi link|https://wokwi.com/projects/463122053343102977|

## Struktura repozitorija

```text
Lab2/
├── Lab2.md
├── README.md
├── main.ino
├── diagram.json
├── wokwi.toml
├── libraries.txt
├── src/
│   └── main.ino
├── docs/
│   ├── izvjestaj.md
│   ├── analiza\_baterije.md
│   └── dijagram\_stanja.md
├── wokwi/
│   ├── main.ino
│   ├── diagram.json
│   ├── wokwi.toml
│   └── libraries.txt
└── rezultat/
    └── serial\_output.txt
```

## Pokretanje u Wokwi

1. Otvoriti Wokwi i odabrati predložak **ESP32**.
2. U projekt kopirati datoteke iz direktorija `wokwi/`:

   * `main.ino`
   * `diagram.json`
   * `wokwi.toml`
   * `libraries.txt`
3. Pokrenuti simulaciju.
4. Otvoriti Serial Monitor i pratiti poruke: učitavanje stanja, wake-up, mjerenje, spremanje, sleep.
5. Za test dodatnog event-driven buđenja pritisnuti tipkalo **WAKE** spojeno na D4/GPIO4.

## Napomena

Wokwi omogućuje testiranje logike sleep/wake ciklusa, ali ne omogućuje stvarno mjerenje potrošnje energije. U simulatoru je dodana NVS/Preferences pohrana jer RTC memorija kroz Deep Sleep nije uvijek pouzdano očuvana. Za stvarnu procjenu potrošnje potrebno je koristiti fizički sklop i odgovarajuću mjernu opremu.

## Napomena o spajanju

* LED je spojena klasično: `D2/GPIO2 -> otpornik 220 Ω -> anoda LED (A)`, a katoda LED (C) ide na `GND`.
* DHT22 je spojen na `D15/GPIO15`.
* Tipkalo za dodatni wake-up spojeno je na `D4/GPIO4` i `GND`. U kodu se koristi `INPUT\_PULLUP`, pa je pritisnuto stanje logička nula (`LOW`).

## Napomena o razlogu buđenja u Wokwi simulatoru

Na stvarnom ESP32 mikrokontroleru nakon prvog ciklusa očekivani razlog buđenja je `ESP\_SLEEP\_WAKEUP\_TIMER` ili `ESP\_SLEEP\_WAKEUP\_EXT0`, ovisno o tome je li sustav probuđen timerom ili tipkalom. U Wokwi simulatoru `esp\_deep\_sleep\_start()` može rezultirati reset ciklusom simuliranog čipa, pa funkcija `esp\_sleep\_get\_wakeup\_cause()` ne mora vratiti stvarni wake-up razlog. Zato kod nakon prvog ciklusa ispisuje simulirani razlog buđenja, a stanje se za potrebe simulacije dodatno sprema u NVS (`Preferences`).



## Testiranje tipkala

Timer wake-up postavljen je na varijabilni interval od 20 do 40 sekundi. Za Wokwi je dodana vizualna sleep pauza do 15 sekundi kako bi se u simulatoru praktično testiralo tipkalo `WAKE` spojeno na `D4/GPIO4` bez predugog čekanja.

