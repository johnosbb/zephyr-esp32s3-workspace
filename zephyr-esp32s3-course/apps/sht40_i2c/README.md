# SHT40 + SSD1306 (I2C) Demo

Reads temperature and humidity from a Sensirion SHT40 sensor and shows the values on a 0.96" 128x64 SSD1306 OLED, both sharing I2C0 on the ESP32-S3 DevKitC.

## Wiring
- Display: `VCC` -> 3V3, `GND` -> GND, `SDA` -> GPIO1, `SCL` -> GPIO2, address `0x3C` (default).
- SHT40: `VCC` -> 3V3, `GND` -> GND, `SDA` -> GPIO1, `SCL` -> GPIO2, address `0x44` (default).

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Sets `zephyr,display = &ssd1306_oled` and enables the SSD1306 node on `&i2c0` at `0x3c`.
- Adds `sht4x@44` on the same I2C bus (`compatible = "sensirion,sht4x"`). Change `reg` to `0x45` if your sensor uses the secondary address.

## Config (`prj.conf`)
- Enables display + character framebuffer and the SHT4x sensor stack: `CONFIG_DISPLAY`, `CONFIG_SSD1306`, `CONFIG_CHARACTER_FRAMEBUFFER`, `CONFIG_SENSOR`, `CONFIG_SHT4X`, plus logging and minimal libc; float formatting is enabled via `CONFIG_CBPRINTF_FP_SUPPORT`.

## Build/flash
```bash
python scripts/build.py --app sht40_i2c --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- West alternative:  
  `west build -b esp32s3_devkitc/esp32s3/procpu -d build/sht40_i2c apps/sht40_i2c`  
  `west flash -d build/sht40_i2c --esp-device <COMx-or-/dev/ttyUSBx>`

## What the code does (`src/main.c`)
- Gets the display from `zephyr,display` and the SHT40 node label `sht4x_0`.
- Sets pixel format to `PIXEL_FORMAT_MONO01`, unblanks the panel, then initializes CFB once.
- Each second: fetches a sample via `sensor_sample_fetch`, reads temperature and humidity channels, logs them, and prints two formatted lines on the OLED using CFB helpers.

## Troubleshooting
- If the display stays blank, verify the 0x3C address; change the overlay `reg` if your panel is 0x3D.
- If SHT40 reads fail, check pull-ups on SDA/SCL and confirm the address (0x44 vs 0x45). Ensure both devices share ground and 3V3.
