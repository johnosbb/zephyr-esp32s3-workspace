# A Guide to Course Examples

This document is a quick index for all examples in `zephyr-esp32s3-course/apps`.

---

## `welcome`
**Brief description:** Intro app that prints periodic logs to confirm the toolchain and board are working.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC  
**Images:**
- **Bench photo**
  ![image](https://github.com/user-attachments/assets/f1c2520c-472e-4b58-b08f-53750ae8e78f)

**Images:**  
![welcome](https://github.com/user-attachments/assets/f1c2520c-472e-4b58-b08f-53750ae8e78f)

---

## `blinky`
**Brief description:** Basic LED toggle example (GPIO output fundamentals).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC (onboard/user LED)  
**Images:**

**Images:**  

---

## `button`
**Brief description:** Button interrupt/debounce example that controls an LED.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, push button, LED  
**Images:**
- **Wiring / state diagram**
  ![image](https://github.com/user-attachments/assets/3fe15598-2760-461a-a90e-1a331bdfe00b)

**Images:**  
![button](https://github.com/user-attachments/assets/3fe15598-2760-461a-a90e-1a331bdfe00b)

---

## `logging_demo`
**Brief description:** Demonstrates Zephyr logging configuration and log output flow.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC  
**Images:** _Add log screenshot or architecture diagram here._

---

## `multi_thread_blinky`
**Brief description:** Two-thread scheduling demo with independent LED blink rates.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, 2x LEDs  
**Images:**
- **Timing/thread diagram**
  ![image](https://github.com/user-attachments/assets/4b1f61b4-a19f-480b-b0c7-35a86aed5cf5)

**Images:**  
![multi_thread](https://github.com/user-attachments/assets/4b1f61b4-a19f-480b-b0c7-35a86aed5cf5)

---

## `periodic_work`
**Brief description:** Workqueue/timer pattern for periodic tasks.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, LEDs  
**Images:** _Add workqueue timing diagram here._

---

## `producer_consumer`
**Brief description:** Inter-thread communication using a producer-consumer model.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, LEDs  
**Images:** _Add queue/flow diagram here._

---

## `debug_demo`
**Brief description:** GDB-friendly demo for breakpoints/watchpoints and runtime inspection.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `debug.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, USB cable (USB Serial/JTAG)  
**Other resources:**
- [Installing OCD](./installing_ocd.pdf)

---

## `oled_ssd1306`
**Brief description:** Basic SSD1306 OLED output over I2C.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SSD1306 OLED (I2C)  
**Images:**
- **OLED module**
  ![image](https://github.com/user-attachments/assets/ec71dd26-d286-4c2e-a11e-26c63dfdbcd7)
- **Wiring schematic**
  ![image](https://github.com/user-attachments/assets/01fb9f1d-8de2-4415-923b-9f5617c1a326)

**Images:**  
![oled1](https://github.com/user-attachments/assets/ec71dd26-d286-4c2e-a11e-26c63dfdbcd7)  
![oled2](https://github.com/user-attachments/assets/01fb9f1d-8de2-4415-923b-9f5617c1a326)

---

## `sht40_i2c`
**Brief description:** Reads SHT40 temperature/humidity and displays values (OLED + logs).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SHT40 sensor (I2C), SSD1306 OLED  
**Images:**
- **SHT40 module**
  ![image](https://github.com/user-attachments/assets/f538124b-1d33-4c4d-84a6-a16c0222620d)
- **Wiring schematic**
  ![schematic](https://github.com/user-attachments/assets/3c85e3db-b897-4c12-980f-a84104452aef)

**Images:**  
![sht40](https://github.com/user-attachments/assets/f538124b-1d33-4c4d-84a6-a16c0222620d)  
![sht40_schematic](https://github.com/user-attachments/assets/3c85e3db-b897-4c12-980f-a84104452aef)

---

## `mcp3008_oled`
**Brief description:** SPI ADC (MCP3008) sampling with OLED display and Wi-Fi config support.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, MCP3008 ADC, SSD1306 OLED, analog input source  
**Images:**
- **MCP3008 module**
  ![mcp3008](https://github.com/user-attachments/assets/9ea2e4c5-61dd-4a42-9b87-1e3c9a765db1)
- **OLED module**
  ![image](https://github.com/user-attachments/assets/ea15a861-e187-4d50-820d-972b5f2de8e8)
- **Wiring schematic**
  ![Schematic](https://github.com/user-attachments/assets/257c8b68-f8ac-4f31-af88-76f583fa74dc)

**Images:**  
![mcp3008](https://github.com/user-attachments/assets/9ea2e4c5-61dd-4a42-9b87-1e3c9a765db1)  
![mcp3008_setup](https://github.com/user-attachments/assets/ea15a861-e187-4d50-820d-972b5f2de8e8)  
![mcp3008_schematic](https://github.com/user-attachments/assets/257c8b68-f8ac-4f31-af88-76f583fa74dc)

---

## `sg90_stepper`
**Brief description:** PWM servo control/state-machine demo using SG90.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, SG90 servo, external 5V servo power (recommended)  
**Images:**
- **SG90 servo**
  ![sg90](https://github.com/user-attachments/assets/ef1a1def-0d68-440d-865a-2da7970ecc15)
- **Wiring diagram**
  ![image](https://github.com/user-attachments/assets/57246f8c-425f-40b6-b539-e0590687e6dd)
- **PWM scope capture**
  ![sg90_scope](https://github.com/user-attachments/assets/43b1057e-241d-4128-ae5b-0122c48d5edf)

**Images:**  
![sg90](https://github.com/user-attachments/assets/ef1a1def-0d68-440d-865a-2da7970ecc15)  
![sg90_setup](https://github.com/user-attachments/assets/57246f8c-425f-40b6-b539-e0590687e6dd)  
![sg90_scope](https://github.com/user-attachments/assets/43b1057e-241d-4128-ae5b-0122c48d5edf)

---

## `web_welcome`
**Brief description:** Intro Wi-Fi + HTTP server example that serves a simple web page.  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `sections-rom.ld`, `wifi_secrets.conf`, `wifi_secrets_example.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access  
**Images:**
- **Web UI screenshot**
  ![webpage](https://github.com/user-attachments/assets/40b4a039-f613-4d8c-a7ce-bf703e7da357)

**Images:**  
![web_welcome](https://github.com/user-attachments/assets/40b4a039-f613-4d8c-a7ce-bf703e7da357)

---

## `web_controller`
**Brief description:** Web-based telemetry/control example (sensor values + remote LED control).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `sections-rom.ld`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access, SHT40, MCP3008, LEDs  
**Images:**
- **Web control UI screenshot**
  ![web_page](https://github.com/user-attachments/assets/b080961b-29b8-4672-ad7b-e7ee940a581c)

**Images:**  
![web_controller](https://github.com/user-attachments/assets/b080961b-29b8-4672-ad7b-e7ee940a581c)

---

## `trace_demo`
**Brief description:** Zephyr tracing demo (CTF RAM backend + Trace Compass workflow).  
**Key files:** `CMakeLists.txt`, `prj.conf`, `src/main.c`, `README.md`, `wifi_secrets.conf`, `boards/esp32s3_devkitc.overlay`  
**Parts used:** ESP32-S3 DevKitC, Wi-Fi network access  
**Other resources:**
- [Setting up Trace](./setting_up_trace.pdf)

