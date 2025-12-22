# SG90 Hobby Servo State Machine Demo

Drives an SG90-style hobby servo with a single PWM output. A button cycles: Off → Right → Left → Off. The on-board LED mirrors “servo active”.

## Wiring (ESP32-S3 DevKitC)
- Servo signal (yellow/orange) -> GPIO15
- Servo VCC (red) -> 5V
- Servo GND (black/brown) -> GND
- Button: on-board BOOT button (GPIO10) via alias `sw0`.
- LED: on-board LED (GPIO8) via alias `led0`.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Enables LEDC channel 0 on GPIO15 for PWM and exposes it as alias `servo0`.
- Reuses on-board LED/button aliases.

## Config (`prj.conf`)
- GPIO, PWM, logging enabled; larger main stack.

## Behavior (`src/main.c`)
- Debounced button advances state: Off (PWM idle, LED off) → Right pulse (~2.0 ms) → Left pulse (~1.0 ms) → Off.
- LED is on while PWM is driving the servo.

## Build/flash
```bash
python scripts/build.py --app sg90_stepper --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
