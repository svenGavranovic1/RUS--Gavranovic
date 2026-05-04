# Teorijska analiza trajanja baterije

## Pretpostavke

- Kapacitet baterije: 2500 mAh
- Aktivna struja: 80 mA
- Deep Sleep struja: 0.15 mA
- Aktivna faza: 3 s
- Sleep interval: varijabilan 20–40 s
- Za izračun se koristi srednji sleep interval: 30 s
- Ukupno vrijeme ciklusa: 33 s

## Formula

```text
I_avg = (I_active × t_active + I_sleep × t_sleep) / ukupno vrijeme
```

## Izračun

```text
I_avg = (80 mA × 3 s + 0.15 mA × 30 s) / 33 s
I_avg = (240 + 4.5) / 33
I_avg = 7.41 mA
```

## Procjena trajanja baterije

```text
trajanje = kapacitet baterije / prosječna struja
trajanje = 2500 mAh / 7.41 mA
trajanje ≈ 337 h ≈ 14 dana
```

## Napomena

Vrijednosti su teorijske. U stvarnom sustavu rezultat bi ovisio o stvarnom ESP32 modulu, DHT22 senzoru, regulatoru napona, LED indikatoru, temperaturi, kapacitetu baterije i načinu spajanja. Wokwi ne simulira stvarnu potrošnju energije.
