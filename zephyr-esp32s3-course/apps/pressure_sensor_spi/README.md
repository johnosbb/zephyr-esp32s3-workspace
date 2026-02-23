# BMP280 SPI + OLED Demo

Reads temperature and barometric pressure from a BMP280 over SPI and shows temperature, pressure, and derived altitude on the SSD1306 OLED. The sensor is polled at a fixed rate (no interrupt trigger).

## Wiring (ESP32-S3 DevKitC)
- BMP280 (SPI2): `SCLK` -> GPIO12, `MOSI` -> GPIO11, `MISO` -> GPIO13, `CS` -> GPIO17.
- OLED (I2C0): `SDA` -> GPIO1, `SCL` -> GPIO2, address `0x3C`.
- Power: 3V3 and GND shared.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Enables the SSD1306 at `0x3C` on `i2c0` and sets `zephyr,display`.
- Places a `bmp280@0` node on `spi2` with `compatible = "bosch,bme280"` and `spi-max-frequency = 100 kHz` (Mode 0), remapping SPI2 CS to GPIO17.

## Config (`prj.conf`)
- Display + CFB: `CONFIG_DISPLAY`, `CONFIG_SSD1306`, `CONFIG_CHARACTER_FRAMEBUFFER`.
- Sensor stack: `CONFIG_SENSOR`, `CONFIG_BME280`, `CONFIG_SPI`.
- Float formatting: `CONFIG_CBPRINTF_FP_SUPPORT`.
- Newlib libc for math support, logging enabled.

## Behavior (`src/main.c`)
- Initializes the OLED and prints three lines with temperature (C), pressure (kPa), and altitude (m).
- Polls the BMP280 every 500 ms; altitude is derived using standard sea-level pressure (101.325 kPa).
- Logs each sample to the console.

## Build/flash
```bash
python scripts/build.py --app pressure_sensor_spi --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If you change SPI pins, update the board pinctrl or overlay and rebuild clean. If your OLED uses `0x3D`, change `reg` accordingly.
