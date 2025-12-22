# Slide 7 Notes

## Zephyr from the Arduino Perspective

- Arduino is easy to use and ideal for beginners, but not designed for complex RTOS tasks.
- Arduino “sketches” run a single, long loop with limited concurrency.
- Peripheral handling is simplified, but lacks structured device drivers found in Zephyr.
- Memory management is minimal, making larger systems harder to scale safely.
- No built-in RTOS features like threads, scheduling, IPC, or power-aware execution.
- Zephyr offers a professional-grade environment while still supporting many Arduino-class MCUs.

## Key idea

Arduino is great for quick prototyping; Zepyyr offers structure.


