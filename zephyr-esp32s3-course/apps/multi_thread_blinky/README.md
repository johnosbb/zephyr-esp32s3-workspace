# Multi-Thread Blinky (two LEDs)

Two threads drive two LEDs at different rates to illustrate Zephyr threading, priorities, and scheduling on the ESP32-S3 DevKitC.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Defines two LED aliases: `led0` on GPIO8 and `led1` on GPIO9 (GPIO active-high). Adjust pins if your board wires LEDs differently.

## Config (`prj.conf`)
- Enables GPIO and logging, uses minimal libc, and sets a roomy main stack. Console is left on UART for easy log viewing.

## Behavior (`src/main.c`)
- Configures both LEDs as outputs.
- Spawns two threads with different priorities: `led0_blink` at 300 ms (higher priority) and `led1_blink` at 700 ms (lower priority).
- Each thread logs its toggles so you can observe preemption/scheduling alongside the LED activity.

## Build/flash
```bash
python scripts/build.py --app multi_thread_blinky --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If your LEDs use different GPIOs, update `boards/esp32s3_devkitc.overlay` and rebuild clean.
