# SSD1306 OLED (128x64 I2C) Demo

This sample shows how to drive a 0.96" 128x64 SSD1306 OLED over I2C on the ESP32-S3 DevKitC. It draws a few text lines and simple shapes using Zephyr’s character frame buffer (CFB).

## Wiring
- Display supply: `VCC` → 3V3, `GND` → GND.
- I2C0 (default pins for this board): `SDA` → GPIO1, `SCL` → GPIO2.
- Device address: `0x3C` (most 0.96" modules). The overlay assumes this.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Sets `zephyr,display = &ssd1306_oled` and enables the SSD1306 node on `&i2c0` at `0x3c`.
- Uses the standard 128x64 timings/offsets; adjust if your panel differs.

## Config (`prj.conf`)
- Enables the display stack: `CONFIG_DISPLAY`, `CONFIG_SSD1306`, `CONFIG_CHARACTER_FRAMEBUFFER` (CFB helper), plus logging and minimal libc.

## Build/flash
```bash
python scripts/build.py --app oled_ssd1306 --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If you prefer west directly:  
  `west build -b esp32s3_devkitc/esp32s3/procpu -d build/oled_ssd1306 apps/oled_ssd1306`  
  `west flash -d build/oled_ssd1306 --esp-device <COMx-or-/dev/ttyUSBx>`

## What the code does (`src/main.c`)
- Grabs the display from the `zephyr,display` chosen node.
- Initializes the CFB helper, clears the buffer, prints three lines of text, and draws a box with a crosshair.
- Calls `display_blanking_off()` before drawing to ensure the panel is active.
- Leaves the content on-screen; the main loop just sleeps.

## Troubleshooting
- If you see nothing: verify wiring, address (`0x3C` vs `0x3D`), and that SDA/SCL are on GPIO1/2. Adjust the overlay `reg` or pinctrl if needed.
- For different resolutions, update `width`/`height` in the overlay and tweak positions in `main.c`.
