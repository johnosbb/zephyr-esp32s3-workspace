# Slide 10 Notes

## How Zephyr Projects Are Structured

- Standard layout: src/, prj.conf, CMakeLists.txt, and optional Devicetree overlays.
- prj.conf controls compile-time configuration through Kconfig.
- Devicetree overlays describe hardware, buses, and device connections.
- Build system uses CMake and west to generate the final executable.


## Key idea

Zephyr projects are organized around clear app directories plus shared workspace modules.

## Narration

Zephyr applications follow a predictable structure that keeps code and configuration organized. 
Your main application code lives in the src folder, while configuration is kept separate in files like prj.conf and CMakeLists.txt.

The prj.conf file enables or disables features at compile time using Zephyr’s Kconfig system. 
This is where you turn on logging, drivers, networking, and any subsystem your application requires.

If your hardware needs custom definitions or additional peripherals, you describe those in Devicetree overlay files. 
This allows Zephyr to match your software to the physical hardware layout.

Finally, the west tool and CMake work together to configure, build, and flash your application. 
This system makes Zephyr applications easy to reproduce and portable across boards.


## References

- Zephyr Application Development Guide (Workspace layout)
- `west` tool documentation
