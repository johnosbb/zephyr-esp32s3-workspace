# Producer–Consumer Queue Demo

Two threads exchange messages over a Zephyr `k_msgq`. The producer generates data and pulses one LED, the consumer drains the queue and pulses the second LED so you can see the flow on hardware.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Defines two LED aliases: `led0` on GPIO8 (producer pulse) and `led1` on GPIO9 (consumer pulse). Adjust pins if your board wires LEDs differently.

## Config (`prj.conf`)
- Enables GPIO and logging, uses minimal libc, and keeps UART console for logs. Assertions/debug thread info remain enabled to aid tracing.

## Behavior (`src/main.c`)
- `k_msgq` holds up to 8 `struct msg` items (`seq`, `value`).
- Producer thread (prio 7) pushes a message every 200 ms and short-pulses `led0` (30 ms). Logs when the queue is full.
- Consumer thread (prio 8) blocks on `k_msgq_get`, pulses `led1` (60 ms) when a message arrives, and logs the contents. A small delay after consume shows scheduling interplay.

## Build/flash
```bash
python scripts/build.py --app producer_consumer --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If your LEDs use different GPIOs, update `boards/esp32s3_devkitc.overlay` and rebuild clean.
