# Web Controller (Zephyr Networking + Control)

This example builds on `web_welcome` by adding live telemetry and control:
- Reads SHT40 temperature + humidity over I2C.
- Reads all 8 channels from the MCP3008 ADC over SPI.
- Toggles two LEDs from the web page.

## Wiring (ESP32-S3 DevKitC)
- SHT40 on I2C0: SDA GPIO1, SCL GPIO2 (address 0x44).
- MCP3008 on SPI2: SCLK GPIO12, MOSI GPIO11, MISO GPIO13, CS GPIO17.
- LEDs: GPIO8 (LED0), GPIO9 (LED1).
- Power: 3V3 and GND shared.

## Configure Wi-Fi
If `wifi_secrets.conf` is missing, it will be auto-created on first build.
Edit `wifi_secrets.conf` and set:
- `CONFIG_WIFI_CREDENTIALS_STATIC_SSID`
- `CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD`

## Build/flash
```bash
python scripts/build.py --app web_controller --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```

## Usage
- Watch the log for the DHCP IPv4 address (it prints `Browser: http://.../`).
- Open that URL in your browser.
