# A Guide to Course Examples

This document is a quick index for all examples in `zephyr-esp32s3-course/apps`.

## `welcome`
**Brief description:** Intro app that prints periodic logs to confirm the toolchain and board are working.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC  
**Images:**
<img width="659" height="491" alt="image" src="https://github.com/user-attachments/assets/f1c2520c-472e-4b58-b08f-53750ae8e78f" />


## `blinky`
**Brief description:** Basic LED toggle example (GPIO output fundamentals).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC (onboard/user LED)  
**Images:**
<img width="808" height="807" alt="esp32s3_diagram" src="https://github.com/user-attachments/assets/ef246d1d-b04e-4c62-bbd1-451a5806249e" />


## `button`
**Brief description:** Button interrupt/debounce example that controls an LED.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, push button, LED  
**Images:**
<img width="651" height="509" alt="image" src="https://github.com/user-attachments/assets/3fe15598-2760-461a-a90e-1a331bdfe00b" />


## `logging_demo`
**Brief description:** Demonstrates Zephyr logging configuration and log output flow.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC  



## `multi_thread_blinky`
**Brief description:** Two-thread scheduling demo with independent LED blink rates.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, 2x LEDs  
**Images:**
<img width="719" height="525" alt="image" src="https://github.com/user-attachments/assets/4b1f61b4-a19f-480b-b0c7-35a86aed5cf5" />


## `periodic_work`
**Brief description:** Workqueue/timer pattern for periodic tasks.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, LEDs  


## `producer_consumer`
**Brief description:** Inter-thread communication using a producer-consumer model.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, LEDs  


## `debug_demo`
**Brief description:** GDB-friendly demo for breakpoints/watchpoints and runtime inspection.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `debug.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, USB cable (USB Serial/JTAG)  
**Other resources:**
[](../installing_ocd.pdf)

## `trace_demo`
**Brief description:** Zephyr tracing demo (CTF RAM backend + Trace Compass workflow).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access  
[](../setting_up_trace.pdf)

## `oled_ssd1306`
**Brief description:** Basic SSD1306 OLED output over I2C.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SSD1306 OLED (I2C)  
**Images:**
<img width="387" height="351" alt="image" src="https://github.com/user-attachments/assets/ec71dd26-d286-4c2e-a11e-26c63dfdbcd7" />
<img width="803" height="556" alt="image" src="https://github.com/user-attachments/assets/01fb9f1d-8de2-4415-923b-9f5617c1a326" />



## `sht40_i2c`
**Brief description:** Reads SHT40 temperature/humidity and displays values (OLED + logs).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SHT40 sensor (I2C), SSD1306 OLED  
**Images:**
<img width="151" height="196" alt="image" src="https://github.com/user-attachments/assets/f538124b-1d33-4c4d-84a6-a16c0222620d" />
<img width="852" height="563" alt="schematic" src="https://github.com/user-attachments/assets/3c85e3db-b897-4c12-980f-a84104452aef" />



## `mcp3008_oled`
**Brief description:** SPI ADC (MCP3008) sampling with OLED display and Wi-Fi config support.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, MCP3008 ADC, SSD1306 OLED, analog input source  
**Images:**
<img width="238" height="192" alt="mcp3008" src="https://github.com/user-attachments/assets/9ea2e4c5-61dd-4a42-9b87-1e3c9a765db1" />
<img width="298" height="284" alt="image" src="https://github.com/user-attachments/assets/ea15a861-e187-4d50-820d-972b5f2de8e8" />
<img width="866" height="466" alt="Schematic" src="https://github.com/user-attachments/assets/257c8b68-f8ac-4f31-af88-76f583fa74dc" />


## `sg90_stepper`
**Brief description:** PWM servo control/state-machine demo using SG90.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SG90 servo, external 5V servo power (recommended)  
**Images:**
<img width="352" height="237" alt="sg90" src="https://github.com/user-attachments/assets/ef1a1def-0d68-440d-865a-2da7970ecc15" />
<img width="841" height="541" alt="image" src="https://github.com/user-attachments/assets/57246f8c-425f-40b6-b539-e0590687e6dd" />
<img width="2718" height="1440" alt="sg90_scope" src="https://github.com/user-attachments/assets/43b1057e-241d-4128-ae5b-0122c48d5edf" />





## `web_welcome`
**Brief description:** Intro Wi-Fi + HTTP server example that serves a simple web page.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `sections-rom.ld`, `wifi_secrets.conf`, `wifi_secrets_example.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access  
**Images:**
<img width="905" height="492" alt="webpage" src="https://github.com/user-attachments/assets/40b4a039-f613-4d8c-a7ce-bf703e7da357" />



## `web_controller`
**Brief description:** Web-based telemetry/control example (sensor values + remote LED control).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `sections-rom.ld`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access, SHT40, MCP3008, LEDs  
**Images:**
<img width="1148" height="573" alt="web_page" src="https://github.com/user-attachments/assets/b080961b-29b8-4672-ad7b-e7ee940a581c" />




