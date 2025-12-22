# Debug Demo (ESP32-S3)

This app is a small, GDB-friendly example that routes console output over the ESP32-S3 USB Serial/JTAG interface. It contains several globals, locals, and helper functions for stepping, setting breakpoints, and inspecting memory.

## DeviceTree overlay (`boards/esp32s3_devkitc.overlay`)
- Sets `zephyr,console` and `zephyr,shell-uart` to `&usb_serial` so printk/logging go to the USB CDC COM port (e.g., COM12).
- Marks `&usb_serial` as `status = "okay"` to enable the on-chip USB Serial/JTAG peripheral. No extra pins are needed beyond the native USB connector.

## Configuration
- `prj.conf` enables logging, assertions, debug info, and routes console/stdout over serial. It also keeps I2C/sensor options handy for future watchpoints.
- `debug.conf` is an optional fragment that raises log verbosity to `DBG` and keeps compiler debug optimizations enabled; pass it via `-DEXTRA_CONF_FILE=debug.conf`.

## Build/flash
```bash
# Standard build + flash (uses console over USB Serial/JTAG)
python scripts/build.py --app debug_demo --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>

# Build with the debug fragment
west build -b esp32s3_devkitc/esp32s3/procpu -d build/debug_demo apps/debug_demo \
  -- -DEXTRA_CONF_FILE=debug.conf
west flash -d build/debug_demo --esp-device <COMx-or-/dev/ttyUSBx>
```

## GDB tips (attach after flashing)
- Start a debug session:  
  `west debug -d build/debug_demo -- -ex "break update_state" -ex "continue"`
- Good breakpoints: `compute_next_step`, `update_state`, `checksum8`.
- Useful watchpoints: `watch state.accumulator`, `watch global_counter`, `watch -l sensor_ring[0]`.
- Inspect memory: `x/8bx sensor_ring`, `p state`, `p/x global_counter`, `info locals` inside `compute_next_step`.
- Single-step through the loop: `si` / `ni`; continue with `c`.

The main loop calls `update_state()` every ~700 ms, updating globals (`global_counter`, `state`, `sensor_ring`) and logging progress. This gives you stable symbols and predictable state changes to practice breakpoints, watchpoints, and memory inspection.
