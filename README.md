# Development Environment Setup

This course uses **Zephyr**, **west**, and a **Python virtual environment**.  
The setup is designed so that students can get started with minimal effort.

---

## Overview of the Environment

The workspace root is the GitHub repo (zephyr-esp32s3-workspace), and inside it the zephyr-esp32s3-course directory contains the manifest (west.yml) and all course material. The bootstrap script creates a Python virtual environment, installs west, initializes the workspace using west init -l zephyr-esp32s3-course, and downloads Zephyr and its modules into the workspace. This structure means students can simply clone the repo, run the bootstrap script, and have a full Zephyr environment created automatically.

Inside zephyr-esp32s3-course, the apps/ directory is the main home for the examples. Each app lives in its own subfolder, for example apps/welcome/, with the conventional Zephyr structure: CMakeLists.txt, prj.conf, and src/main.c (plus any extra source files). Device tree overlays or app-specific board tweaks sit in apps/<app-name>/boards/ (as we have with apps/welcome/boards/esp32s3_devkitc.overlay).

# 1. First-Time Setup (Required Once)

Clone the repo and run the bootstrap script.

## Linux / macOS

```bash
git clone https://github.com/johnosbb/zephyr-esp32s3-course.git
cd zephyr-esp32s3-course
./scripts/bootstrap.sh
```

## Windows (PowerShell)

```powershell
git clone https://github.com/johnosbb/zephyr-esp32s3-course.git
cd zephyr-esp32s3-course
.\scripts\bootstrap.ps1
```

### What the bootstrap script does

- Creates a Python virtual environment (`.venv`)
- Installs **west**
- Initializes west using this repo as the manifest
- Downloads Zephyr + required modules (`west update`)
- Checks for the **Zephyr SDK**
- If the SDK is missing, it prints clear instructions on how to install it  
  (no admin rights required if installed in your home directory)

You can build and flash apps in the same terminal immediately after bootstrap completes.

---

# 2. Installing the Zephyr SDK (Only If Requested)

If the bootstrap script cannot find the SDK, it prints instructions.

Choose one of the installation methods below.

---

## Linux x86_64: Install Zephyr SDK with wget (recommended)

The course examples assume the **Zephyr SDK** is installed in your home directory.

````bash
# Choose the SDK version you want to use (must be compatible with the Zephyr version in west.yml)
SDK_VERSION=0.17.0

# Download the SDK archive for Linux x86_64
wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"

# Extract and move into your home directory
tar xf "zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"
mv "zephyr-sdk-${SDK_VERSION}" "$HOME/zephyr-sdk"

# Run SDK setup script (no sudo needed for a user-local install)
"$HOME/zephyr-sdk/setup.sh"

# Tell Zephyr to use this SDK (for this terminal session)
export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk"

Or more permanently place these in your .bashrc file


```bash
echo 'export ZEPHYR_TOOLCHAIN_VARIANT=zephyr' >> "$HOME/.bashrc"
echo 'export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk"' >> "$HOME/.bashrc"
```

Then reload your shell configuration in the current terminal:

```bash
source "$HOME/.bashrc"
```

You can verify they are set with:

```bash
echo "$ZEPHYR_TOOLCHAIN_VARIANT"
echo "$ZEPHYR_SDK_INSTALL_DIR"
```

---

## Windows — Standard Install

1. Download the Windows SDK installer (`.exe`) from:
   https://github.com/zephyrproject-rtos/sdk-ng/releases

2. Run the installer. The default path usually looks like:

```

C:\zephyr-sdk-0.17.4

```

3. Set environment variables:

```powershell
setx ZEPHYR_TOOLCHAIN_VARIANT zephyr
setx ZEPHYR_SDK_INSTALL_DIR "C:\zephyr-sdk-0.17.4"
```

4. Close PowerShell and open a **new** PowerShell window so the variables take effect.

---

# 3. Everyday Usage (After the First Setup)

Whenever you open a new terminal, activate the environment first.

## Linux / macOS

```bash
cd zephyr-esp32s3-course
source .venv/bin/activate
```

## Windows (PowerShell)

```powershell
cd zephyr-esp32s3-course
.\.venv\Scripts\Activate.ps1
```

You can now build any course application:

```bash
python scripts/build.py --app welcome --clean --flash
```

or in PowerShell:

```powershell
python scripts/build.py --app welcome --clean --flash
```

---

# 4. When Should You Re-Run Bootstrap?

Only run the bootstrap script again if:

- You deleted the `.venv` folder
- You checked out the repo in a fresh location
- You intentionally want to download Zephyr modules again

For normal use, **do not** rerun bootstrap.

---

# 5. Quick Environment Checks

After activating your environment, you can verify everything is set up:

## Linux / macOS

```bash
west --version
echo $ZEPHYR_TOOLCHAIN_VARIANT
echo $ZEPHYR_SDK_INSTALL_DIR
```

## Windows (PowerShell)

```powershell
west --version
echo $env:ZEPHYR_TOOLCHAIN_VARIANT
echo $env:ZEPHYR_SDK_INSTALL_DIR
```

If all three show meaningful values, your environment is ready.
