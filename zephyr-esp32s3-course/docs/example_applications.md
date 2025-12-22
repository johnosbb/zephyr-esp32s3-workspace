# Example Applications



## Stage 1 — Fundamentals 

These introduce Zephyr’s core runtime, threading, GPIO, and debugging.

### Welcome

* Print to console using printk or logging subsystem
* Show Kconfig and devicetree basics

### Blinky

* GPIO output
* Thread timing (k_sleep, k_timer)

### Button Input

* GPIO input
* Interrupts & debouncing
* Basic state machines

### Debug Demo

* Step-through debugging
* Stack frames, variables, call stack
* Using Zephyr logging levels


Stage 2 — Concurrency, Scheduling, and More Zephyr Basics

These expand on Zephyr behavior before adding external sensors.

### Multi-Thread Blinky

* Two LEDs blinking at different rates in separate threads
* Demonstrates threading, priorities, and scheduling

### Producer–Consumer Queue Demo

* One thread generates data
* Another consumes it
* Uses k_msgq or k_fifo

### Periodic Work / Workqueue Example

* Using Zephyr’s workqueues
* Offload time-consuming work from ISRs

## Stage 3 — I²C and Device Drivers (Use Your OLED + MCP23017)

This is where students start working with real hardware beyond GPIO.

### OLED Display “Hello World”

- I²C device initialization
- Using an existing Zephyr display driver
- Drawing text or graphics
- Introduces devicetree overlays

## GPIO Expander: MCP23017 Control Demo

* Use MCP23017 to control extra LEDs or read extra buttons
* Shows I²C + GPIO expander driver
* Teaches how Zephyr abstracts extended GPIOs

Great teaching moment:
Students see how “virtual GPIO pins” appear under /dev/gpio@22 or similar.

## Stage 4 — Sensor Framework (Use Radar, ADXL345, TOF Distancer, Ultrasonic)

Zephyr has a unified SENSOR API — this is perfect for our hardware.

### Accelerometer Demo (ADXL345 — GY-291)

* Using Zephyr’s sensor API
* Reading acceleration vectors
* Triggering readings via interrupts
* Display values on OLED for bonus points

### Distance Measurement Demo (HC-SR04 Ultrasonic)

* Use GPIO triggering and precise timing
* Demonstrate busy-wait, interrupts, or timer capture
* Introduces real-time constraints

### Laser ToF Distance Demo (TOF050C/0200C/0400C)

* I²C sensor reading
* Compare ToF accuracy vs ultrasonic

Discuss real-world sensor trade-offs (great teaching moment)

### Human Presence Detection Demo (HLK-LD2410C Radar Sensor)

* Parse UART output packets
* Convert to presence/distance values
* Use logging and OLED display to show detections
* Demonstrates UART + protocol parsing + event-driven design

## Stage 5 — Positioning & Communication

Introduce more advanced APIs.

### GPS Driver Demo (GT-U7)

* UART input
* Parse NMEA messages (GGA, RMC)
* Extract latitude, longitude, speed
* Optionally log over USB or display coordinates on OLED

### Combined Sensor Fusion Demo

* Accelerometer + GPS
* Track movement
* Discuss filtering & sensor reliability
* This feels like a “mini-project”

## Stage 6 — Actuators, PWM & Motion Control

Use your servos and dual motor driver.

### Servo Control via PWM

* Use Zephyr’s PWM API
* Sweep servo angle
* Teach duty cycle + pulse width concepts 

### Motor Driver Demo (TB6612 / DRV8833)

* Use PWM + GPIO for direction and speed
* Simple “motor ramp-up” example
* Add button for start/stop
* Optionally use OLED for speed display

## Stage 7 — Full Mini-Projects

* These combine multiple APIs into real IoT-style applications.
* Gesture-Controlled Servo (Accelerometer → Servo)
* Tilt sensor controls servo angle
* Great for demonstrating sensor → actuator pipeline

### Presence-Activated Display (Radar → OLED)

* Show a message only when someone approaches
* Add timeout/sleep for power management
* Smart Distance Meter (ToF → OLED → Logging)
* Continuous distance measurement
* Change display color or icon based on thresholds

### Environmental Awareness Node (GPS + Radar + Accelerometer)

* A “multi-sensor embedded node”
* Show unified output on OLED
* This becomes an excellent capstone demo



## PCB Design

Strapping Pins
The ESP32-S3 chip has the following strapping pins:

* GPIO 0
* GPIO 3
* GPIO 45
* GPIO 46
* 
These pins are used to put the ESP32 into bootloader or flashing mode. On most development boards with built-in USB/Serial, we don’t need to worry about the state of these pins. The board puts the pins in the right state for flashing or boot mode. However, we should avoid using these pins on our projects.

| Feature / Peripheral            | Signal Description                  | ESP32-S3 Pin |
|---------------------------------|--------------------------------------|--------------|
| **LEDs & Buttons**              |                                      |              |
| LED1                            | GPIO LED                             | GPIO8        |
| LED2                            | GPIO LED                             | GPIO9        |
| Button0                         | Active-low button                    | GPIO10       |
|                                 |                                      |              |
| **I²C Bus (Shared)**            |                                      |              |
| I2C SDA                         | Shared SDA for SSD1306, SHT40, AUX  | GPIO1        |
| I2C SCL                         | Shared SCL for SSD1306, SHT40, AUX  | GPIO2        |
|                                 |                                      |              |
|                                 |                                      |              |
| **GPS Module (GT-U7)**          | UART                                 |              |
| GPS_RX                          | Module RX ← ESP32 TX                 | GPIO21       |
| GPS_TX                          | Module TX → ESP32 RX                 | GPIO18       |
|                                 |                                      |              |
| **Human Presence Sensor (HLK-LD2410C)** |                            |              |
| Radar_TX                        | Module → ESP32 RX                    | GPIO36       |
| Radar_RX                        | Module ← ESP32 TX                    | GPIO35       |
| Radar_OUT                       | Optional presence interrupt          | GPIO7        |
|                                 |                                      |              |
| **Ultrasonic Sensor (HC-SR04)** |                                      |              |
| TRIG                            | Trigger output                       | GPIO47       |
| ECHO                            | Echo input (via divider)             | GPIO4        |
|                                 |                                      |              |
| **Motor Driver – DRV8833**      |                                      |              |
| AIN1                            | Motor A Input 1                      | GPIO16       |
| AIN2                            | Motor A Input 2                      | GPIO5        |
| STBY                            | Motor Standby,must be HIGH to enable the driver        | GPIO15       |
|                                 |                                      |              |
| **SPI Bus (ADXL345 – SPI 4-wire)** |                                  |              |
| SPI SCLK (SCL)                  | SPI clock                            | GPIO12       |
| SPI MOSI (SDA)                  | Master-out, slave-in                 | GPIO11       |
| SPI MISO (SDO)                  | Master-in, slave-out                 | GPIO13       |
| SPI CS (ADXL345)                | Chip select                          | GPIO14       |
|                                 |                                      |              |
| **Optional ADXL345 Interrupts** |                                      |              |
| ADXL_INT1                       | Motion interrupt                     | GPIO37       |
| ADXL_INT2                       | Motion interrupt                     | GPIO38       |




Diver Network for Echo Output from the Ultrasonic Sensor (HC-SR04)

```
5V (Echo output)
   |
   R1 = 10 kΩ
   |
-----> node → GPIO4
   |
   R2 = 15 kΩ
   |
  GND
```


ADXL345 Pin   | Arduino Pin | ESP32-S3 Pin (Recommended)
--------------|-------------|----------------------------
CS            | 10          | GPIO14
SDO (MISO)    | 12          | GPIO13
SDA (MOSI)    | 11          | GPIO11
SCL (SCLK)    | 13          | GPIO12
VCC           | 3V3         | 3V3
GND           | GND         | GND