# Slide 16 Notes

## Logging System Explained

- Zephyr logging provides leveled logs (error to debug) with deferred processing.
- Backends include RTT, UART, and others; choose per board and bandwidth needs.
- Configure levels and formatting via `CONFIG_LOG_*` options in `prj.conf`.

## Key idea

Zephyr’s flexible logging lets you balance insight and performance across targets.

## Narration

Zephyr’s logging subsystem gives structured, leveled output without blocking the main application. Messages can be processed immediately or deferred, and routed through backends like UART or RTT depending on your hardware. You control verbosity and formatting in `prj.conf` with `CONFIG_LOG_DEFAULT_LEVEL` and backend-specific options. Properly tuned logging speeds troubleshooting while keeping runtime overhead low on resource-constrained boards.

## References

- Zephyr Logging subsystem docs
- `CONFIG_LOG_*` options in Kconfig
