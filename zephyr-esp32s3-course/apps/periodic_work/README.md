# Periodic Workqueue Demo

Shows how to offload ISR-triggered tasks to Zephyr’s system workqueue. Two timers fire at different rates, submit work items, and pulse separate LEDs so you can see the scheduling.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Defines two LED aliases: `led0` on GPIO8 (fast work pulse) and `led1` on GPIO9 (slow work pulse). Adjust pins if your board wires LEDs differently.

## Config (`prj.conf`)
- GPIO + logging enabled, minimal libc, UART console. Assertions/debug thread info remain enabled for learning.

## Behavior (`src/main.c`)
- `k_timer` callbacks run in interrupt context and only submit work (`k_work_submit`).
- `fast_work` runs every 200 ms, pulses `led0` briefly, logs, and simulates modest processing.
- `slow_work` runs every 700 ms, pulses `led1`, logs, and simulates heavier processing.
- Main just configures LEDs and starts the timers; all work executes on the system workqueue thread.

## Build/flash
```bash
python scripts/build.py --app periodic_work --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If your LEDs use different GPIOs, update `boards/esp32s3_devkitc.overlay` and rebuild clean.
