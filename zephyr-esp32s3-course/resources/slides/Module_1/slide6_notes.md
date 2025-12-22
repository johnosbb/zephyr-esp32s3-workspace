# Slide 6 Notes

## Why We Choose the ESP32-S3


* ESP32-S3 is low cost, widely available, and powerful.
* Built-in Wi-Fi and BLE support in Zephyr.
* Excellent choice for learning because you can focus on Zephyr APIs, not vendor SDK complexity.
* Zephyr abstracts peripherals cleanly: GPIO, I2C, SPI, Wi-Fi, and more.
* Integrated debug support (USB JTAG) makes development and troubleshooting easy.

## Key idea

The ESP32-S3 is a developer-friendly platform for learning Zephyr

## Narration

The ESP32-S3 is a low-cost microcontroller that offers a surprising amount of performance for its price. 
It’s widely available, easy to source, and powerful enough to handle real IoT applications. 
That makes it a great platform whether you're prototyping or building production-ready systems.

Zephyr also includes built-in support for the ESP32-S3’s Wi-Fi and Bluetooth Low Energy features. 
We don’t need to manage these networking stacks yourself; they integrate directly into Zephyr’s APIs. 
This lets you focus on building your application instead of wrestling with connectivity.

Another advantage is that the ESP32-S3 is one of the easiest platforms for learning Zephyr. 
Instead of dealing with vendor-specific SDKs or complex setup tools, you work directly with Zephyr’s unified APIs. 
This reduces friction and helps you learn the RTOS without getting bogged down in vendor differences.

Zephyr also gives you a clean abstraction layer for peripherals such as GPIO, I2C, SPI, and Wi-Fi and all of these interfaces are available on the ESP32-S3. 
This abstraction means our code becomes more portable and easier to maintain on other hardware platforms or other variants of the ESP32. 
We can easily move our application to another microcontroller with only minimal changes.

And finally, the ESP32-S3 includes integrated USB JTAG debugging support. That means you get professional-grade debugging without needing any extra hardware. 
It makes development smoother, gives you great insight into remote debugging and it helps us diagnose and fix issues far more quickly.
