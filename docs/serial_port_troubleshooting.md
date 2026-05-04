# Serial Port Troubleshooting

If `west flash` fails to connect, the chip flashes but the console is
silent or garbled, or your terminal cannot open the port at all — the
problem is almost always one of a small number of issues. This page
walks the common ones in roughly the order you will hit them.

Read the symptom table first; jump to the section that matches.

## Symptom-to-fix table

| Symptom                                                                    | Section                                                | Most likely cause                                                  |
| -------------------------------------------------------------------------- | ------------------------------------------------------ | ------------------------------------------------------------------ |
| Flash fails: `Invalid head of packet (0x1B)`                               | [§1 Two USB ports on the DevKitC](#1-two-usb-ports-on-the-esp32-s3-devkitc) | Plugged into the **USB** port; the running app owns the interface |
| Flash fails: `Failed to connect: no response`                              | [§1](#1-two-usb-ports-on-the-esp32-s3-devkitc), [§4 Cable](#4-bad-or-charge-only-cable) | Same as above when the app is quiet, OR a charge-only cable        |
| Console output is garbled gibberish                                        | [§6 Baud rate (no autobaud)](#6-baud-rate-no-autobaud-115200-fixed) | Terminal not set to 115200 8-N-1                                   |
| Console is blank after a successful flash                                  | [§5 Port already in use](#5-port-already-in-use), [§1](#1-two-usb-ports-on-the-esp32-s3-devkitc) | Another tool holding the port, OR pointing at the wrong port      |
| Linux: `Permission denied: '/dev/ttyUSB0'`                                 | [§2 Permissions (Linux)](#2-permissions-linux)         | User is not in the `dialout` group                                 |
| Windows: no COM port appears at all                                        | [§3 Drivers (Windows)](#3-drivers-windows)             | CP210x or CH340 USB-UART driver not installed                      |
| Flash succeeds but the board does not appear to boot                       | [§7 Reset behaviour after flash](#7-reset-behaviour-after-flash) | Auto-reset failed; chip is still in download mode                  |
| Intermittent failures: works once, fails on the next attempt               | [§1](#1-two-usb-ports-on-the-esp32-s3-devkitc), [§5](#5-port-already-in-use) | Application is racing for the USB-CDC interface, OR a stale handle holding the port |

If nothing in the table matches, run through [§8 Triage
checklist](#8-triage-checklist-before-asking-for-help) before asking for
help.

---

## 1) Two USB ports on the ESP32-S3 DevKitC

This is the single most common pitfall. The DevKitC has **two micro-USB
sockets, side by side**, labelled **UART** and **USB** on the board:

| Port | Connects to                                                | Linux device              | Windows  | Auto-reset for flashing? |
| ---- | ---------------------------------------------------------- | ------------------------- | -------- | ------------------------ |
| UART | external USB-UART bridge (CP210x or CH340)                 | `/dev/ttyUSB0`            | `COMx`   | **Yes** — DTR/RTS lines drive BOOT/EN, esptool flips the chip into download mode automatically |
| USB  | the SoC's built-in USB Serial/JTAG peripheral              | `/dev/ttyACM0`            | `COMx`   | **No**  — once an app boots, the running firmware owns this interface; the boot ROM is no longer reachable through it |

### What goes wrong

Plug only the USB cable in. First flash works (empty flash → ROM
bootloader is in charge of the USB interface). The app boots. The USB
interface flips from "ROM download" to "app stdout". Next `west flash`
attempt: esptool opens the port, expects bootloader bytes, and reads
log output instead — typically the byte `0x1B` (ASCII ESC, the start
of an ANSI colour escape from Zephyr's logger). Hence:

```
A fatal error occurred: Failed to connect to Espressif device:
Invalid head of packet (0x1B): Possible serial noise or corruption.
```

### Fixes, in order of preference

1. **Move the cable to the UART port.** It enumerates as
   `/dev/ttyUSB0` on Linux (or a `COMx` on Windows). Then:
   ```
   west flash --esp-device /dev/ttyUSB0
   ```
   This is the workflow the course assumes by default.

2. **If you must use the USB port**, force download mode by hand
   each time you flash:
   - Hold the **BOOT** button (sometimes labelled IO0).
   - Tap the **EN** button (reset).
   - Release **BOOT**.
   - Now run `west flash`. The chip is in ROM download mode and the
     interface is owned by the boot ROM, not the running app.

3. **Best long-term workflow: plug both cables in.** Flash via the
   UART port, monitor logs and JTAG-debug via the USB port. No button
   presses, fastest iteration.

---

## 2) Permissions (Linux)

Out of the box, only `root` can open `/dev/ttyUSB*` and `/dev/ttyACM*`
on most Linux distributions. The error looks like:

```
serial.serialutil.SerialException: [Errno 13] could not open port
/dev/ttyUSB0: [Errno 13] Permission denied: '/dev/ttyUSB0'
```

### Fix

Add your user to the `dialout` group (Debian/Ubuntu/most distros) or
`uucp` (Arch/Fedora):

```bash
sudo usermod -aG dialout $USER
```

Then **log out and back in** (or reboot) — group membership is read
when your session starts, so an open shell will not pick up the new
group until then. Verify:

```bash
groups | grep -E 'dialout|uucp'
```

### When `udev` rules also fight you

If the device is owned by `root:root` rather than `root:dialout`, your
distribution's udev rules are not assigning the right group. The
Espressif and Zephyr SDKs ship a udev rules file you can install:

```bash
sudo cp ~/zephyr-sdk-0.17.4/sysroots/x86_64-pokysdk-linux/usr/share/openocd/contrib/60-openocd.rules \
       /etc/udev/rules.d/
sudo udevadm control --reload-rules && sudo udevadm trigger
```

Unplug and replug the cable after this.

---

## 3) Drivers (Windows)

The UART port on the DevKitC is a CP210x or CH340 USB-UART bridge. If
the driver is missing, no COM port appears in Device Manager when you
plug the board in (or it appears as an unknown device with a yellow
exclamation mark).

### Fix

- **CP210x** (Espressif silicon): install the Silicon Labs CP210x VCP
  driver from the Silicon Labs website.
- **CH340** (some clone boards): install the WCH CH340/CH341 driver
  from the WCH website.

After installing, unplug and replug the board. Open Device Manager →
**Ports (COM & LPT)**. You should see something like
`Silicon Labs CP210x USB to UART Bridge (COM7)`.

The **USB** port (the chip's built-in USB Serial/JTAG) does not need
a driver on modern Windows — it enumerates as a generic USB CDC ACM
device.

---

## 4) Bad or charge-only cable

USB-A-to-micro-B cables come in two flavours: **data + power** and
**power-only** (charge-only). Charge-only cables look identical, are
extremely common in laptop bags, and are the second most common cause
of "the board doesn't show up at all" after the wrong-port problem.

### How to tell

- The board powers on (you see the on-board power LED) but **no COM
  port / `/dev/ttyUSB*` / `/dev/ttyACM*` device appears**.
- A different cable from the same machine to the same port works.

### Fix

Use a known-good data cable. The cable that came with the DevKitC is
typically fine; phone-charging cables often are not. If in doubt,
keep one labelled "data" in your kit.

---

## 5) Port already in use

If a serial monitor (VS Code Serial Monitor, PuTTY, screen, minicom,
the IDF Monitor, the Arduino Serial Monitor, or a previous
`west espressif monitor` you forgot to close) is already holding the
port, `west flash` will fail because esptool cannot open the device.
The error is OS-specific:

- Linux: `[Errno 16] Device or resource busy: '/dev/ttyUSB0'`
- Windows: `Could not open COM7, the port is busy or doesn't exist`

### Fix

Close the other tool. On Linux you can find what holds the port:

```bash
sudo fuser /dev/ttyUSB0
sudo lsof /dev/ttyUSB0
```

Then close the offending process. On Windows, close any open serial
console window in VS Code, PuTTY, Arduino IDE, etc. The
`west espressif monitor` command especially likes to be left running
in another terminal — flash will fail until you exit it (Ctrl+]).

---

## 6) Baud rate (no autobaud, 115200 fixed)

The Zephyr UART console **does not autobaud**. It runs at a fixed rate
set at compile time by the board's devicetree (for `esp32s3_devkitc`,
`current-speed = <115200>` in the SoC DTS). The course's apps do not
override this, so every Part 1 demo runs at:

> **115200 baud, 8 data bits, no parity, 1 stop bit (8-N-1).**

### What goes wrong

If your terminal is set to a different rate (9600, 460800, etc.), the
chip's output looks like garbage — random bytes that nearly always
include high-bit characters. The chip is fine; the terminal is
misreading the bit timing.

### Fix

Set the terminal to **115200 8-N-1**. Examples:

```bash
# west's built-in monitor (defaults to 115200)
west espressif monitor

# Python miniterm
python -m serial.tools.miniterm /dev/ttyUSB0 115200

# screen
screen /dev/ttyUSB0 115200

# PuTTY: Connection type = Serial, Speed = 115200, Data bits = 8,
#        Stop bits = 1, Parity = None, Flow control = None
```

### What about flashing — doesn't that use a different rate?

Yes, but flashing is a separate conversation from the runtime console,
and it is **not autobaud either**. esptool sends a synchronisation
packet at a chosen rate (the `--baud` argument, often `921600` in this
course's `west flash` invocation) and the ROM bootloader negotiates
speed from there. The host dictates the speed; nothing is auto-detected.

This is why a successful flash does not prove your terminal is
configured correctly — the two conversations are independent.

---

## 7) Reset behaviour after flash

After `west flash` completes, the chip needs to *leave download mode*
and start running the new image. esptool tries to do this by toggling
the DTR/RTS lines on the UART bridge to issue a hard reset (this is
what `--after hard_reset` in the flash command line does).

If the auto-reset fails — most often because:
- you are flashing via the **USB** port (no DTR/RTS auto-reset wiring
  to BOOT/EN), or
- something is holding DTR/RTS asserted (some terminal emulators do
  this when they connect)

…then the chip will sit in download mode after flashing and look
"hung." It is not hung; it is waiting.

### Fix

Press the **EN** (reset) button on the board manually. The new image
will boot. Long-term, prefer flashing via the UART port so auto-reset
works.

---

## 8) Triage checklist before asking for help

Run through this before posting "it doesn't work" — answers to these
questions will narrow the problem to one of the sections above almost
every time.

1. **Which port is the cable plugged into — UART or USB?** (See §1.)
2. **Does the OS see *any* port appear when you plug the board in?**
   - Linux: `dmesg | tail` immediately after plugging in. Look for
     `cp210x` / `ch341` / `cdc_acm` lines and a `/dev/ttyUSB0` or
     `/dev/ttyACM0` device.
   - Windows: Device Manager → Ports (COM & LPT).
3. **Can you open the port?** A quick `cat /dev/ttyUSB0` (Linux) or
   opening it in PuTTY (Windows) at 115200 should not error. If it
   errors → §2 (Linux), §3 (Windows), or §5 (busy).
4. **Is your terminal set to 115200 8-N-1?** (See §6.)
5. **Have you tried a different cable?** (See §4.)
6. **Is anything else holding the port?** (See §5. Close all other
   serial consoles.)
7. **Have you tried the manual download-mode dance?** (Hold BOOT →
   tap EN → release BOOT → flash. See §1 fix #2.)

If all seven check out and you are still stuck, capture and send:

- The exact `west flash` (or `python scripts/build.py ... --flash`)
  command you ran.
- The full error output, *unedited*, from that command.
- The output of `dmesg | tail` (Linux) or a screenshot of Device
  Manager (Windows) immediately after plugging the board in.
- Which port (UART vs USB) the cable is in, and whether the on-board
  power LED is lit.

That bundle is enough to diagnose almost any remaining issue.

---


