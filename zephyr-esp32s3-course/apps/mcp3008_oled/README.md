# MCP3008 + OLED Demo

Reads a single MCP3008 ADC channel over SPI and shows the result on the SSD1306 OLED. Each button press advances the channel (0 → 7 → 0).

## Wiring (ESP32-S3 DevKitC)
- MCP3008 (SPI2): `SCLK` -> GPIO12, `MOSI` -> GPIO11, `MISO` -> GPIO13, `CS` -> GPIO17.
- OLED (I2C0): `SDA` -> GPIO1, `SCL` -> GPIO2, address `0x3C`.
- Button: GPIO10 (pull-up, active-low).
- Power: 3V3 and GND shared.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Sets `zephyr,display` to the SSD1306 on `i2c0`.
- Adds `mcp3008@0` on `spi2` with `cs-gpios = <&gpio0 17 GPIO_ACTIVE_LOW>`.

## Behavior (`src/main.c`)
- Reads channel 0 on boot and shows it on the OLED.
- Each button press increments the channel and updates the display.
- SPI Mode 0 is used (default CPOL=0, CPHA=0), 1 MHz.

## Build/flash
```bash
python scripts/build.py --app mcp3008_oled --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
