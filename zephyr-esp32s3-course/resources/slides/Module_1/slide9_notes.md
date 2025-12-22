# Slide 9 Notes

## Zephyr Architecture Overview

- Modular RTOS built around a small, efficient microkernel.
- Hardware abstraction through drivers and devicetree.
- Subsystems for networking, sensors, storage, Bluetooth, power management, and more.
- Clear separation between kernel, drivers, and user application code.


## Key idea

Zephyr layers a small kernel with modular services so you pick only what your device needs.

## Narration

Zephyr is built around a small microkernel that handles the core tasks of scheduling, threading, and synchronization. 
Everything else is modular, so features are included only when your application needs them.

Above the kernel, Zephyr uses a hardware abstraction layer driven by device drivers and devicetree descriptions. 
This lets Zephyr support a wide range of microcontrollers through a consistent API.

On top of that, Zephyr provides rich subsystems for things like networking, sensors, file systems, storage, Bluetooth, and power management. 
These let you build complex embedded applications without reinventing the wheel.

As a developer, you write your application code on top of this layered architecture, so your logic stays clean and portable even when hardware changes.


## References

- Zephyr Architecture overview (Kernel and subsystems)
- Zephyr Devicetree and Kconfig guides
