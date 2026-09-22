# robodog


3D model link:
https://a360.co/4yQa9Da

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

No extra libraries are needed; the servo is driven with the built-in LEDC PWM. The sketch needs Arduino core 3.x (`ledcAttach` API).

### PlatformIO (VS Code)

The repo root has a `platformio.ini`, so open this folder in VS Code with the PlatformIO extension and it appears under **PlatformIO > Projects**. If the list is stale, use **Projects > Add Existing** and pick this folder. Then use Build / Upload / Monitor from the PlatformIO toolbar, or from a terminal:

```sh
pio run                 # build
pio run -t upload       # flash (auto-detects the port)
pio device monitor      # serial monitor at 115200
```

The `platformio.ini` uses the [pioarduino](https://github.com/pioarduino/platform-espressif32) platform because the stock `espressif32` platform is stuck on Arduino core 2.0.x, which lacks `ledcAttach`. First build downloads the platform and toolchain.

### Arduino IDE / arduino-cli

Uses the `esp32:esp32` core (3.x).

```sh
arduino-cli compile --fqbn esp32:esp32:esp32 --upload --port /dev/cu.usbserial-0001 ServoSweep
```

Or open `ServoSweep/ServoSweep.ino` in the Arduino IDE, select **ESP32 Dev Module**, and upload.

**Unplug the servo before flashing.** With the servo drawing current over plain USB power the flash chip drops out and esptool reports `Failed to communicate with the flash chip`. Plug it back in after the upload, or power the expansion board from its DC jack.
