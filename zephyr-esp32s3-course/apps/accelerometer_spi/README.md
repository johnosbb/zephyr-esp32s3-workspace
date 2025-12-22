# Accelerometer SPI Demo (ADXL345 + OLED)

Reads acceleration vectors from an ADXL345 over I2C and shows them on the SSD1306 OLED. Data-ready interrupts trigger a work item that fetches and prints the latest XYZ values.

## Wiring (ESP32-S3 DevKitC)
- ADXL345 (I2C): `SDA` -> GPIO1, `SCL` -> GPIO2, address `0x53`.
- ADXL345 interrupt: `INT2` -> GPIO38 (data ready). `INT1` on GPIO37 is available if you want a second interrupt.
- OLED: `VCC` -> 3V3, `GND` -> GND, `SDA` -> GPIO1, `SCL` -> GPIO2, address `0x3C`.
- LEDs (optional cues): `led0` GPIO8, `led1` GPIO9 (not used in code but left aliased).

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Enables the SSD1306 at `0x3C` on `i2c0` and sets `zephyr,display`.
- Places an `adxl345@53` node on `i2c0` with `int2-gpios = <&gpio1 38 GPIO_ACTIVE_HIGH>` (ODR=100 Hz).

## Config (`prj.conf`)
- Display + CFB: `CONFIG_DISPLAY`, `CONFIG_SSD1306`, `CONFIG_CHARACTER_FRAMEBUFFER`.
- Sensor stack: `CONFIG_SENSOR`, `CONFIG_ADXL345`, `CONFIG_ADXL345_TRIGGER`.
- Minimal libc, logging enabled.

## Behavior (`src/main.c`)
- Sets OLED pixel format, unblanks, and initializes CFB once.
- Registers a data-ready trigger on the ADXL345; the ISR submits a work item.
- Work handler fetches `SENSOR_CHAN_ACCEL_XYZ`, logs the values, and prints three lines (X/Y/Z in g) on the OLED.
- If trigger setup fails, it falls back to polling at ~200 ms.

## Build/flash
```bash
python scripts/build.py --app accelerometer_spi --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If you move CS or INT pins, adjust `cs-gpios` and `int2-gpios` in the overlay and rebuild clean. If your OLED uses `0x3D`, change `reg` accordingly.
