# Button/LED Demo (ESP32-S3 DevKitC)

This sample shows how to hook a GPIO interrupt to a button, debounce it, and toggle the on-board LED. It reuses the course build flow (`scripts/build.py`) and the board overlay in `boards/esp32s3_devkitc.overlay`.

## Hardware mapping (overlay)
- `led0` → `GPIO0_8`, active-high, defined under a `gpio-leds` node as `onboard_led`.
- `sw0` → `GPIO0_10`, pull-up + active-low, defined under `gpio-keys` as `user_button0`.
- Aliases `led0`/`sw0` make the binding portable; the C code uses `DT_ALIAS` to stay board-agnostic.

## Code highlights (`src/main.c`)
- Retrieves bindings with `GPIO_DT_SPEC_GET(DT_ALIAS(led0/sw0), gpios)` so pin numbers and flags live in DeviceTree, not code.
- Configures the LED as output inactive and the button as input, then enables both-edge interrupts; press starts debounce, release clears a latch.
- ISR (`button_pressed`) does minimal work: it reschedules a delayable work item for debouncing instead of touching hardware directly.
- Debounce handler (`debounce_work_handler`) runs after `DEBOUNCE_MS` (100 ms by default), re-reads the button, and toggles the LED only on a confirmed press; a `press_latched` flag prevents multiple toggles while the button is held.
- Uses Zephyr logging to confirm presses and failures; the main loop just sleeps because all action is interrupt-driven.

## Build/flash
```bash
python scripts/build.py --app button --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- Adjust `--port` or the board/overlay flags if your kit wiring differs.
- If bouncing persists, raise `DEBOUNCE_MS` in `src/main.c`; if long presses still retrigger, increase it slightly and keep the latch logic enabled.
