# Trace Demo (CTF + RAM backend)

This example demonstrates Zephyr RTOS tracing on ESP32-S3 using:
- CTF (Common Trace Format)
- RAM tracing backend (ring buffer on target)
- GDB dump via USB-JTAG
- Visualization in Eclipse Trace Compass (open source)
- Heartbeat LED (`led0`) toggled every 500 ms to show liveness

Note: RAM backend capture is finite. Once `CONFIG_RAM_TRACING_BUFFER_SIZE` is filled,
new trace packets are dropped. The application should keep running, but trace data in
`channel0_0` will stop advancing until reset/reflash.

## Build/flash
```bash
python scripts/build.py --app trace_demo --clean --flash --monitor --port <COMx>
```

## Capture trace buffer with GDB
Connect GDB to the target (USB-JTAG) and dump the `ram_tracing` buffer.
See `docs/setting_up_trace.md` for the full host-side setup steps.
```
(gdb) info address ram_tracing_start
(gdb) info address ram_tracing_end
(gdb) dump binary memory data/channel0_0 <ram_tracing_start> <ram_tracing_end>
```

## Prepare CTF folder
```
mkdir data
copy /Y %ZEPHYR_BASE%\\subsys\\tracing\\ctf\\tsdl\\metadata data\\metadata
```

Linux/macOS:
```
mkdir -p data
cp "$ZEPHYR_BASE/subsys/tracing/ctf/tsdl/metadata" data/metadata
cp data/channel0_0 data/channel0_0
```

## Open in Trace Compass
1) Open Trace Compass
2) File → Open Trace
3) Select the `data/` folder (it must contain `metadata` + `channel0_0`)
