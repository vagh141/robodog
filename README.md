# robodog

Arduino sketches for the robodog project, running on an ESP32 (ESP32-D0WD) dev board mounted on a GVS expansion board.

## Sketches

| Sketch | What it does |
|---|---|
| `ServoSweep/` | Sweeps a servo on GPIO 13 from 0° to 180° and back, continuously. |

## Hardware

- ESP32 Dev Module (classic ESP32, 4 MB flash)
- Expansion board with G / V / S headers per GPIO
- Servo on the row marked **13**: brown/black → G, red → V, yellow/orange → S

## Building and flashing

Uses the `esp32:esp32` core (3.x) from the Arduino IDE. No extra libraries are needed; the servo is driven with the built-in LEDC PWM.

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 --upload --port /dev/cu.usbserial-0001 ServoSweep
```

Or open `ServoSweep/ServoSweep.ino` in the Arduino IDE, select **ESP32 Dev Module**, and upload.

**Unplug the servo before flashing.** With the servo drawing current over plain USB power the flash chip drops out and esptool reports `Failed to communicate with the flash chip`. Plug it back in after the upload, or power the expansion board from its DC jack.
