# Slide 15 Notes

## Building, Flashing & Debugging

- Build with `west build -b <board> -d build/<app> <app-path>`; keep outputs per app.
- Flash with `west flash` (or vendor-specific options) and monitor serial output.
- Debug via `west debug` or IDE launch configs using your board’s debug probe.

## Key idea

Repeatable build/flash/debug commands keep the development loop tight and predictable.

## Narration

Zephyr standardizes the development loop through west. You configure and compile with `west build`, keeping artifacts in a dedicated build directory. For deployment, `west flash` drives the correct runner for your board, whether that’s an ESP32 loader, OpenOCD, or a vendor tool. When debugging, `west debug` brings up GDB with the right server, and IDEs can hook into the same flow. Knowing these commands—and their common flags—lets you iterate quickly and troubleshoot hardware confidently.

## References

- Zephyr west build/flash/debug commands
- Board-specific runner documentation
