# Slide 12 Notes

## Devicetree Introduction

- Devicetree describes what hardware exists and how it is connected.
- Board and SoC definitions live in .dts and .dtsi files.
- Applications customize hardware using Devicetree overlays.
- Zephyr generates headers and macros from Devicetree at build time.

## Key idea

Devicetree is the single source of truth for hardware description; code and drivers rely on it.

## Narration

Zephyr uses Devicetree to describe the hardware your application runs on — 
what devices exist, which buses they are connected to, and which pins they use. 
Instead of hardcoding hardware details in C code, all of that information lives in Devicetree files.

At the lowest level, hardware is described using .dts and .dtsi files. 
The .dtsi files usually define reusable pieces, such as SoCs or common peripherals, 
while .dts files describe a specific board by pulling those pieces together.

When you build an application, Zephyr combines the board’s Devicetree with any application-specific overlays. 
Overlays let you enable devices, change pins, or add new peripherals without modifying upstream board files, 
which keeps your project clean and easy to maintain.

From this combined description, the build system generates a binary Devicetree blob and C headers. 
Your code accesses devices using macros like DEVICE_DT_GET or DT_NODELABEL, 
which keeps hardware details out of your source code and makes applications portable and reproducible.


## References

- Zephyr Devicetree guide
- `DT_*` macro reference
