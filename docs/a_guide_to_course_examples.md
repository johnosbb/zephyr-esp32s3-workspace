# A Guide to Course Examples

This document is a quick index for all examples in `zephyr-esp32s3-course/apps`.

## `welcome`
**Brief description:** Intro app that prints periodic logs to confirm the toolchain and board are working.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `blinky`
**Brief description:** Basic LED toggle example (GPIO output fundamentals).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC (onboard/user LED)  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `button`
**Brief description:** Button interrupt/debounce example that controls an LED.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, push button, LED  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `logging_demo`
**Brief description:** Demonstrates Zephyr logging configuration and log output flow.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `multi_thread_blinky`
**Brief description:** Two-thread scheduling demo with independent LED blink rates.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, 2x LEDs  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `periodic_work`
**Brief description:** Workqueue/timer pattern for periodic tasks.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, LEDs  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `producer_consumer`
**Brief description:** Inter-thread communication using a producer-consumer model.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, LEDs  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `debug_demo`
**Brief description:** GDB-friendly demo for breakpoints/watchpoints and runtime inspection.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `debug.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, USB cable (USB Serial/JTAG)  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `trace_demo`
**Brief description:** Zephyr tracing demo (CTF RAM backend + Trace Compass workflow).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** `scripts/zephyr_trace_viewer.py`, Trace Compass setup notes

## `oled_ssd1306`
**Brief description:** Basic SSD1306 OLED output over I2C.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SSD1306 OLED (I2C)  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `sht40_i2c`
**Brief description:** Reads SHT40 temperature/humidity and displays values (OLED + logs).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SHT40 sensor (I2C), SSD1306 OLED  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `accelerometer_spi`
**Brief description:** Sensor readout demo with ADXL345 acceleration data and OLED visualization.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, ADXL345 accelerometer, SSD1306 OLED  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `pressure_sensor_spi`
**Brief description:** BMP280 pressure/temperature acquisition and display output.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, BMP280 sensor, SSD1306 OLED  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `mcp3008_oled`
**Brief description:** SPI ADC (MCP3008) sampling with OLED display and Wi-Fi config support.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, MCP3008 ADC, SSD1306 OLED, analog input source  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `sg90_stepper`
**Brief description:** PWM servo control/state-machine demo using SG90.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SG90 servo, external 5V servo power (recommended)  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `hardware_diagnostics`
**Brief description:** Combined hardware verification app (I2C, SPI, GPIO, PWM, display, sensors).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SSD1306 OLED, SHT40, MCP3008, SG90 servo, button, LEDs  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `web_welcome`
**Brief description:** Intro Wi-Fi + HTTP server example that serves a simple web page.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `sections-rom.ld`, `wifi_secrets.conf`, `wifi_secrets_example.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_

## `web_controller`
**Brief description:** Web-based telemetry/control example (sensor values + remote LED control).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `sections-rom.ld`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access, SHT40, MCP3008, LEDs  
**Images:** _Placeholder_  
**Schematics:** _Placeholder_  
**Other resources:** _Placeholder_
