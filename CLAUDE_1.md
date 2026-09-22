# Robothund V1 — CLAUDE.md

Firbeint robot, 12 DOF, bygget av fire studenter (automatisering/kybernetikk, NTNU).
Denne fila gir alle fire sine Claude-assistenter samme kontekst. Den lastes inn i hver økt,
så den holdes kort: fakta, beslutninger, konvensjoner. Detaljer og begrunnelser ligger i
`docs/prosjektbeskrivelse.md` (v6).

**Prinsipp:** ferdig mekanikk, egen kode. Rammen er KDY0523s SpotMicro — all egen innsats går
til kinematikk, gangart, tilstandsestimering og regulering.

---

## Status

<!-- Oppdater ved hver ukekveld. Kort. -->

- Fase: 0 — bestilling
- Servoer bestilt: nei
- Printvekt målt (slicer): nei
- Sist målte robotvekt: —
- Blokkeringer: servovalg (se «Åpne beslutninger»)

---

## Maskinvare

| Del | Valg | Merk |
|---|---|---|
| Ramme | KDY0523 SpotMicro, CLS6336HV-variant | STL `thingiverse.com/thing:3445283`, STEP `thing:3761340` |
| Servo | 35 kg·cm HV, standard 40×20 mm — se åpen beslutning | 14 stk: 12 + 1 reserve + 1 offer |
| MCU | ESP32 WROOM-32 **classic** | Ikke S3 — S3 har bare 8 LEDC-kanaler |
| Servodriver | PCA9685 primært, ESP32 LEDC som reserve | Avgjøres av dødsonemåling i fase 2 |
| IMU | MPU-6050, mykmontert på skumtape, DLPF 44 Hz | BNO085 kommer i fase 5 som fasit |
| Strømmåling | INA226 + ekstern 50 A / 75 mV shunt | Fjern R100 fra modulen |
| Leddmåling | 3× ADS1115 (potmeter-tapping) | Fallback: 12× AS5600 bak 2× TCA9548A |
| Fotkontakt | 4× KW11-3Z mikrobryter, kort hendel | COM→GND, NO→GPIO m/pull-up |
| Batteri | 2S LiPo 5000 mAh 30–50C | Lad til 4,1–4,15 V/celle |
| Vert | Laptop over USB/WiFi | Raspberry Pi i runde 2. Ingen ROS i runde 1 |

**Strøm — to skinner, aldri én:**
- Servo: 7,4–8,4 V, 20–25 A topp, 14 AWG, 2200 µF lav-ESR + 100 nF nær skinnen.
  Servostrøm går via loddede samleskinner, **ikke** gjennom kretskortet.
- Logikk: 5 V via egen UBEC, < 1 A. Felles jord.
- 30 A blade-sikring rett etter batteriet. Nødstopp = anti-spark MOSFET-bryter (starter ≥ 6 V).

**I²C — to busser:**

| Buss | Enhet | Adresse |
|---|---|---|
| I²C0 | PCA9685 alene | 0x40 — slå av ALLCALL i MODE1 |
| I²C1 | INA226 | **0x41** (ikke 0x40) |
| I²C1 | ADS1115 ×3 | 0x48 / 0x49 / **0x4B** (ikke 0x4A) |
| I²C1 | MPU-6050 | 0x68 |
| I²C1 | BNO085 (fase 5) | 0x4A — holdt ledig |
| I²C1 | TCA9548A ×2 (fallback) | 0x72 / 0x73 (ikke 0x70) |

Pull-ups: mål bussen med alle moduler tilkoblet før noe ekstra monteres.

---

## Harde grenser

- **Maks robotvekt 2,5 kg.** Forventet ~2,0 kg. Vei ved hver milepæl → `docs/vektlogg.md`.
- **Servotemperatur 60 °C stopp.** Driftssyklus 3–4 min gange, hvile til under 40 °C.
- **Batteri under 6,6 V** → sikker positur, kutt servoskinne.
- **`max_foot_reach` = 100 mm** som utgangspunkt. 120 mm spiser 23 prosentpoeng momentmargin.
- Alltid kommandér sikker positur før strømmen kuttes.

Momentkrav ved 2,5 kg, 120 mm arm: 7,5 (stå) / 10,0 (kryp) / 15,0 (trav) kg·cm statisk,
×2 dynamisk. Varme, ikke stall-moment, er den bindende grensen.

---

## Kontraktfiler — eierskap

| Fil | Eier | Innhold |
|---|---|---|
| `config/leg_params.yaml` | Mekanikk | Lenkelengder, leddgrenser, kroppsmål, `max_foot_reach`, sikker positur |
| `config/servo_calibration.json` | Mekanikk | Per servo: `id`, `pin`, `min_us`, `max_us`, `zero_offset_deg`, `direction` + PCA9685 målt oscillatorfrekvens |
| `docs/protocol.md` | Embedded | Vert↔ESP32-protokoll og failsafe-tabell |

**Endrer du en fil du ikke eier: si fra til eieren først.** Kontraktfilene er det som lar fire
personer jobbe parallelt.

**Protokoll (kort):**
```
vert  → ESP32:  VEL <vx> <vy> <w>
                POSE <roll> <pitch> <yaw> <z>
                ACTION <navn>          # sit, stand, lie, wave — for LLM i runde 2
ESP32 → vert:   STATE <q0..q11> <qw> <qx> <qy> <qz> <vbat> <temp>
```

**Failsafe:** ingen kommando 200 ms → hold positur · 1 s → sikker positur, servo idle ·
IMU-timeout → hold positur · watchdog > 3× periode → servo idle, manuell restart.

---

## Roller

| Rolle | Ansvar |
|---|---|
| Mekanikk | SolidWorks, print, montering, kalibrering, vekt |
| Embedded | Kretskort, PWM, sanntidsløkke, IMU/ADC-drivere, failsafe |
| Regulering | IK, gangartgenerator, sensorfusjon, Simulink/Simscape |
| Vert | Simulator som snakker protokollen, telemetri, `ACTION`, Pi/ROS 2-forberedelse |

---

## Repostruktur

```
cad/        STL + STEP + egne endringer, med kilde og versjon
config/     leg_params.yaml, servo_calibration.json
firmware/   ESP32 (C++): IK, gangart, servodriver, IMU, failsafe
control/    MATLAB/Simulink + Python: modell, gangart, filtertuning
docs/       prosjektbeskrivelse.md, protocol.md, toleranser.md,
            vektlogg.md, kalibrering.md, måleprotokoller
CLAUDE.md
```

---

## Konvensjoner for kode og arbeid

- **Egen implementasjon, kjøpt fasit, sammenlign.** IK verifiseres mot Robotics System Toolbox.
  Eget filter tunes mot BNO085. Samme mønster overalt.
- **Sanntidsløkken skrives for hånd.** Ingen Embedded Coder på den. Kodegenerering kun for
  ren matte (IK-funksjonen).
- **Løkkerater:** servokommando 50 Hz (test 200 Hz) · IK = servoraten · IMU 200–400 Hz ·
  leddmåling 64–190 Hz · telemetri 10–30 Hz. Ingen vits å regne raskere enn servoen tar imot.
- **Trigg ADS1115 på fast fase i PWM-perioden** — da blir servoripple en konstant, ikke støy.
- **PCA9685-oscillatoren er ikke 25 MHz.** Mål med oscilloskop, sett `setOscillatorFrequency()`.
- **Enheter i kode:** SI (m, rad, N·m, s). Grader og kg·cm kun i dokumentasjon og logg.
- **Sjekk spenning først, kode sist.** De fleste «kodefeil» i servoroboter er strømforsyning.
- Kalibrering i lav knebøy — der er kneet nesten ubelastet.
- Første frie gange over madrass.

---

## Åpne beslutninger

1. **Servo.** CLS6336HV er dyr og lite solgt på AliExpress. Kandidater, alle standard 40×20:
   - **JX BLS-HV7146MG** — børsteløs, 47,8 kg·cm, 71 g, 40,5×20,5×36 mm, 6–8,4 V.
     Førstevalg hvis ≤ ~280 kr/stk inkl. mva ved 14 stk. Monteringsørenes høyde mot
     utgangsakselen er **ikke verifisert** — sjekk før bestilling.
   - **DSservo DS3235 SG** — coreless, 35 kg·cm, 60 g, 40×20×38,5 mm. Budsjettvalg.
     Spesifisert **5–7,4 V** → lad 2S til 4,1 V/celle. Velg 180°-varianten.
   - Ikke: DS3218MG (20 kg·cm, 4,8–6,8 V — for svak og feil spenning), RDS5160 (feil størrelse).
2. Potmeter-tapping: har servoen potmeter? Åpne offerservoen.
3. Dødsone og maks PWM-rate — måles i fase 2, avgjør PCA9685 vs LEDC.
4. Har instituttet en forsyning på 8,4 V / 25 A? Hvis nei, batteri inn i fase 3.

---

## Verifiserte fakta — motsi bare med kilde

- 22,6 kg·cm (PDI-HV5523MG) klarte ikke å reise SpotMicro — `mike4192/spotMicro` issue #15.
- DS3218/MG er 4,8–6,8 V, ikke HV.
- ESP32 ADC2 er død når WiFi er på (`espressif/arduino-esp32` #440).
- INA226 måler ±81,92 mV over shunt. Standardmodul (R100 = 0,1 Ω) → maks 0,8 A.
- AS5600 har fast adresse 0x36.
- ADS1115: 860 SPS totalt, ~1,16 ms per konvertering.
- Stall→kontinuerlig moment ≈ 20–33 % (Dynamixel XM430, DFRobot SER0066).
- AliExpress-priser vises **uten mva** — legg til 25 %.

---

## Ferdig V1

1. Går 5 m uten ledning, 3× på rad
2. Kryp-gangart over 15 mm hinder
3. IMU holder kroppen ±5° på 10° skråplan
4. Failsafe testet: kutt WiFi midt i gange → sikker positur
5. Repoet har kalibrerings-, toleranse-, vekt-, strøm- og temperaturlogg
6. Servohus < 60 °C etter 5 min gange

---

## Beslutningslogg

<!-- Nye beslutninger nederst, med dato. Ikke slett gamle. -->

- 2026-09 · Ferdige STL-er, ikke egendesignede bein. Egen geometri er runde 2.
- 2026-09 · Standard HV-hobbyservo i V1, ikke bussservo — STL-ene er servospesifikke.
- 2026-09 · ESP32 classic, PCA9685 + LEDC som reserve.
- 2026-09 · To I²C-busser. PCA9685 alene på den ene.
- 2026-09 · Servostrøm utenom kretskortet. Kortet = signal og logikk, 1 oz.
- 2026-09 · Ingen Pi/ROS i runde 1.
- 2026-09 · Sensorer kjøpes når de trengs. BNO085 i fase 5.
- 2026-09 · Fire personer, fire roller.
