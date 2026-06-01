# Work Queue + CONTAINER_OF Demo

Answers a very common question: a work-queue handler only receives
`struct k_work *work`, and `struct k_work` has no `void *user_data` or context
field of its own. So how does a single handler know *which* of many objects it
is acting on? The Zephyr answer is to **embed** a `struct k_work` inside your
own per-instance struct and recover that struct in the handler with
`CONTAINER_OF()`.

This example pretends to manage three "sensor devices". Each device owns its
own work item; one shared handler serves all of them and tells them apart via
`CONTAINER_OF`.

## Overlay (`boards/esp32s3_devkitc.overlay`)
- Defines two LED aliases: `led0` on GPIO8 and `led1` on GPIO9. Two of the
  three demo devices pulse an LED so you can see them; the third is log-only.

## Config (`prj.conf`)
- GPIO + logging enabled, minimal libc, UART console. Assertions/debug thread
  info remain enabled for learning.

## Behavior (`src/main.c`)
- `struct sensor_device` embeds a `struct k_work` plus private context
  (`name`, `id`, a `jobs_handled` counter, and an optional LED).
- `sensor_work_handler` is registered for **every** device. It calls
  `CONTAINER_OF(work, struct sensor_device, work)` to get back the owning
  device, then bumps that device's own counter and pulses its LED.
- A `k_timer` fires every 500 ms in ISR context and submits the next device's
  work item round-robin (the fast-top-half / deferred-bottom-half pattern).
- Because the handler runs on the system workqueue **thread**, it may sleep
  (`k_msleep`) to pulse the LED — something the timer ISR itself must not do.

Expected serial output (cycling through the three devices):
```text
<inf> work_container_of: CONTAINER_OF workqueue demo: 3 devices share one handler
<inf> work_container_of: Handler ran for device 'temp-sensor' (id=1) -- job #1
<inf> work_container_of: Handler ran for device 'humidity' (id=2) -- job #1
<inf> work_container_of: Handler ran for device 'accelerometer' (id=3) -- job #1
<inf> work_container_of: Handler ran for device 'temp-sensor' (id=1) -- job #2
...
```

## Scaling to "devices come and go"
For hot-pluggable hardware (USB, for example) the same mechanism applies: keep
a pool of these structs (a static array or a `k_mem_slab`), and when a device
appears, claim a free slot, `k_work_init()` its work item, store the device
context, and submit. The handler still recovers the right instance with
`CONTAINER_OF`. If you would rather separate the *data* from the *work item*,
the alternative is a single work item paired with a `k_fifo`/`k_msgq` that the
handler drains.

## Build/flash
```bash
python scripts/build.py --app work_container_of --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```
- If your LEDs use different GPIOs, update `boards/esp32s3_devkitc.overlay` and rebuild clean.
