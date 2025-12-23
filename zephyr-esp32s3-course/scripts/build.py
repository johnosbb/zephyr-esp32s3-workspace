#!/usr/bin/env python3
import argparse, os, platform, subprocess, sys
from pathlib import Path

def run(cmd, cwd=None):
    print("+", " ".join(cmd))
    subprocess.check_call(cmd, cwd=cwd)

def main():
    repo = Path(__file__).resolve().parent.parent
    p = argparse.ArgumentParser()
    p.add_argument("--app", default="welcome")
    p.add_argument("--board", default="esp32s3_devkitc/esp32s3/procpu")
    p.add_argument("--overlay", default="boards/esp32s3_devkitc.overlay")
    p.add_argument("--build-dir", default="")
    p.add_argument("--clean", action="store_true")
    p.add_argument("--flash", action="store_true")
    p.add_argument("--monitor", action="store_true")
    p.add_argument("--debug", action="store_true")
    p.add_argument("--port", default="")
    p.add_argument("--gdb-path", default="")
    args = p.parse_args()

    build = Path(args.build_dir) if args.build_dir else Path("build") / args.app
    src = Path("apps") / args.app

    if args.clean and build.exists():
        if platform.system() == "Windows":
            run(["cmd","/c","rmdir","/s","/q", str(build.resolve())])
        else:
            run(["rm","-rf", str(build)])

    west = ["west","build","-p","always","-b",args.board,"-d",str(build),str(src),
            "--", f"-DDTC_OVERLAY_FILE={args.overlay}"]
    run(west, cwd=repo)

    if args.flash:
        cmd = ["west","flash","-d",str(build)]
        if args.port: cmd += ["--esp-device", args.port]
        run(cmd, cwd=repo)

    elf = build / "zephyr" / "zephyr.elf"
    if args.debug:
        if not elf.exists():
            sys.exit(f"ELF not found: {elf}")
        gdb = args.gdb_path
        if not gdb:
            sdk = os.environ.get("ZEPHYR_SDK_INSTALL_DIR","")
            if sdk:
                g = Path(sdk)/"xtensa-espressif_esp32s3_zephyr-elf"/"bin"
                gdb = str((g/("xtensa-espressif_esp32s3_zephyr-elf-gdb.exe"
                               if platform.system()=="Windows" else
                               "xtensa-espressif_esp32s3_zephyr-elf-gdb")).resolve())
        if not gdb or not Path(gdb).exists():
            sys.exit("GDB not found. Use --gdb-path or set ZEPHYR_SDK_INSTALL_DIR.")
        run([gdb, str(elf)], cwd=repo)

    if args.monitor:
        if not build.exists():
            sys.exit(f"Build dir not found: {build}")
        cmd = ["west", "espressif", "monitor"]
        if args.port:
            cmd += ["-p", args.port]
        run(cmd, cwd=build)

if __name__ == "__main__":
    try:
        main()
    except subprocess.CalledProcessError as e:
        sys.exit(e.returncode)


# Usage
# --app <name>       Required. The application folder inside apps/
# --clean            Deletes the build directory before rebuilding
# --flash            Flash the firmware to the ESP32S3 after building
# --monitor          Open Espressif’s serial monitor after flashing
# --port <port>      Specify USB/serial port (COMx on Windows, /dev/ttyUSBx on Linux)
# --debug            Launch GDB (only if supported by your SDK setup)
# --board <board>    Optional override; defaults to esp32s3_devkitc/esp32s3/procpu
# --overlay <file>   Optional overlay; defaults to app’s boards/*.overlay if present

# python scripts/build.py --app welcome --clean
# python scripts/build.py --app welcome --clean --flash
# python scripts/build.py --app welcome --flash --monitor --port /dev/ttyUSB0
# python scripts\build.py --app welcome --flash --monitor --port COM12
