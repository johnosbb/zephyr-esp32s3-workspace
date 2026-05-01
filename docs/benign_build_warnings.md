# Benign Build Warnings

When you build a course app for `esp32s3_devkitc` on Zephyr v4.0.0, the
output will contain a small set of warnings that look alarming but are
expected. They appear on every clean build in this workspace and are not
caused by the application code.

This page lists each one verbatim, explains the cause, and tells you what
you would need to see for it to indicate a real problem. Anything **not**
listed here should be treated as noteworthy — read it, decide whether it
is cosmetic or real, and act accordingly.

---

## 1) Devicetree: missing `#address-cells` on the ESP32 interrupt controller

### Presentation

```
CMake Warning at .../zephyr/cmake/modules/dts.cmake:417 (message):
  dtc raised one or more warnings:

  .../zephyr.dts:NN.NN-NN.N:
  Warning (interrupt_provider): /soc/interrupt-controller@600c2000:
  Missing #address-cells in interrupt provider
```

### Cause

Internal quirk of the upstream Zephyr ESP32 SoC devicetree — the SoC
interrupt controller node does not declare `#address-cells`. The `dtc`
tool warns about it for every Zephyr-on-ESP32 build. The course apps do
not modify this node and cannot fix it.

### When it would be a real problem

If the warning names a node from your **app overlay** (anything under
`boards/esp32s3_devkitc.overlay`), it is real. The path
`/soc/interrupt-controller@600c2000` is the giveaway that this is the
upstream SoC node, not your overlay.

---

## 2) Kconfig: `MCUBOOT_UPDATE_FOOTER_SIZE` ... got the value `''`

### Presentation

```
warning: MCUBOOT_UPDATE_FOOTER_SIZE (defined at subsys/dfu/Kconfig:55)
was assigned the value '0x1000' but got the value ''. Check these
unsatisfied dependencies: MCUBOOT_IMG_MANAGER (=n), IMG_MANAGER (=n).
```

### Cause

`prj.conf` (or a sysbuild fragment) suggests a footer size, but the
symbol's dependencies (`MCUBOOT_IMG_MANAGER`, `IMG_MANAGER`) are not
enabled because this app does not use OTA / DFU. Kconfig is telling you
the suggestion was discarded — which is exactly what you want for a
non-OTA app.

### When it would be a real problem

If the app **does** use the image manager (look for
`CONFIG_MCUBOOT_IMG_MANAGER=y` in `prj.conf`) and you still see this
warning, the dependency chain is broken and OTA will not work.

---

## 3) Kconfig: `BOOT_MAX_IMG_SECTORS` ... got the value `''`

### Presentation

```
warning: BOOT_MAX_IMG_SECTORS (defined at .../mcuboot/boot/zephyr/Kconfig:449)
was assigned the value '512' but got the value ''. Check these
unsatisfied dependencies: (!BOOT_MAX_IMG_SECTORS_AUTO) (=n).
```

### Cause

The Module 3 MCUboot policy uses `BOOT_MAX_IMG_SECTORS_AUTO=y`, which
makes MCUboot compute the sector count from the active flash layout.
That auto-mode disables the manual `BOOT_MAX_IMG_SECTORS` symbol — so
any value assigned to it is correctly discarded.

### When it would be a real problem

If you have explicitly turned off auto-sectors with
`CONFIG_BOOT_MAX_IMG_SECTORS_AUTO=n` and still see this warning, the
manual override is not landing and MCUboot may misread the slot
geometry. Otherwise: ignore.

---

## 4) `cc1.exe: warning: is shorter than expected`

### Presentation

```
[2/N] Generating ../../../../include/generated/xtensa_handlers_tmp.c
cc1.exe: warning:  is shorter than expected
```

### Cause

Emitted by the Espressif Xtensa toolchain during the interrupt-handler
codegen step (it runs `cc1` in `-E` preprocess mode against a short
input file). It is a quirk of the toolchain on Windows and appears once
per Zephyr image — so for a sysbuild app you will see it twice (once
for the application image, once for MCUboot).

### When it would be a real problem

It only matters if it is followed by an actual `error:` line from the
same step, or if step `[N/M] Generating ... xtensa_handlers_tmp.c`
fails. By itself the warning is informational.

---


If the only warnings present are the four documented above, the build
is clean. Anything else needs a closer look.

---


