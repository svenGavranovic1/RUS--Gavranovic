# Lab2 – ESP32 upravljanje potrošnjom energije

Projekt za laboratorijsku vježbu iz upravljanja potrošnjom energije mikrokontrolera.

## Platforma

* ESP32
* Wokwi simulator
* Arduino okruženje

## Funkcionalnost

* Deep Sleep režim
* timer wake-up s promjenjivim intervalom od 20 do 40 sekundi
* dodatni GPIO/EXT0 wake-up tipkalom na D4/GPIO4
* software debounce tipkala
* DHT22 očitanje temperature i vlage
* spremanje zadnjih 10 mjerenja
* RTC varijable (`RTC\\\_DATA\\\_ATTR`)
* NVS/Preferences spremanje zbog ograničenja Wokwi simulacije
* LED indikator aktivne faze
* Wokwi vizualna sleep pauza za lakše testiranje tipkala na D2/GPIO2

## Wokwi

Datoteke za Wokwi nalaze se u direktoriju `wokwi/`:

* `main.ino`
* `diagram.json`
* `wokwi.toml`
* `libraries.txt`

## Dokaz rada

Primjer serijskog ispisa nalazi se u:

```text
rezultat/serial\\\_output.txt
```

## Napomena

Wokwi ne simulira stvarnu potrošnju energije. Projekt demonstrira logiku sleep/wake ciklusa i upravljanje stanjem, ali za stvarnu procjenu potrošnje potrebna je fizička hardverska platforma.



## Testiranje dual wake-upa

Timer interval je 20–40 s, a Wokwi vizualna sleep pauza ograničena je na 15 s kako bi se jasno i brzo vidjelo da **tipkalo WAKE na D4/GPIO4** može probuditi sustav prije isteka timera.

