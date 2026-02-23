# Hardware Diagnostics App

Quick diagnostics for the ESP32-S3 DevKitC setup with OLED + SHT40 on I2C, two LEDs, a button, and an SG90 servo. The app scans I2C, reads the sensor, shows status on the OLED, blinks LEDs, and sweeps the servo on boot.

## Wiring (ESP32-S3 DevKitC)
- I2C0: `SDA` -> GPIO1, `SCL` -> GPIO2
- OLED (SSD1306): address `0x3C`
- SHT40: address `0x44`
- LED0 -> GPIO8, LED1 -> GPIO9 (active high)
- Button -> GPIO10 (pull-up, active low)
- SG90 PWM -> GPIO15 (LEDC channel 0)
- MCP3008 (SPI2): `SCLK` -> GPIO12, `MOSI` -> GPIO11, `MISO` -> GPIO13, `CS` -> GPIO17

## What it does
- **I2C status**: reports OLED/SHT40 readiness based on actual device reads.
- **OLED**: shows device status and live readings.
- **SHT40**: logs temperature/humidity once per second.
- **MCP3008**: reads one channel when the button is pressed; cycles CH0..CH7.
- **LEDs**: alternate blink every second.
- **Button**: logs pressed/released state.
- **Servo**: sweeps left -> right -> center once at boot.

## Expected output
- Console logs should show SHT40 readings each second and MCP3008 updates on button press.
- OLED should show "HW Diagnostics" with status lines.
- LEDs should blink alternately.
- Button logs should toggle when pressed.
- Servo should move once at startup.

## Build/flash
```bash
python scripts/build.py --app hardware_diagnostics --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
