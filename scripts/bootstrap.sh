#!/usr/bin/env bash
set -euo pipefail

# Move to repo root (where this script lives in scripts/)
cd "$(dirname "$0")/.."

# --- Python virtual environment + west ---
python3 -m venv .venv
# shellcheck disable=SC1091
source .venv/bin/activate
python -m pip install --upgrade pip
pip install west

# --- Initialize west workspace using the course subdir as manifest ---
west init -l zephyr-esp32s3-course
west update
west zephyr-export

# --- Zephyr Python requirements ---
# Zephyr tree will be at ./zephyr (inside this workspace) after west update
pip install -r zephyr/scripts/requirements-base.txt
# requirements-build.txt is not present in all versions, so just ensure pyelftools exists
pip install pyelftools

# --- Toolchain discovery: course assumes $HOME/zephyr-sdk ---
SDK_DIR="${ZEPHYR_SDK_INSTALL_DIR:-"$HOME/zephyr-sdk"}"

XTENSA_GDB_BASE="$SDK_DIR/xtensa-espressif_esp32s3_zephyr-elf/bin"
XTENSA_GDB="$XTENSA_GDB_BASE/xtensa-espressif_esp32s3_zephyr-elf-gdb"
[ -x "$XTENSA_GDB" ] || XTENSA_GDB="${XTENSA_GDB}.exe"

if [ -x "$XTENSA_GDB" ]; then
    export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
    export ZEPHYR_SDK_INSTALL_DIR="$SDK_DIR"
    echo "SDK detected at: $ZEPHYR_SDK_INSTALL_DIR"
else
  cat <<'EOF'

Zephyr SDK not detected in $HOME/zephyr-sdk.

This course assumes a user-local install in your home directory.

To install the SDK for Linux x86_64:

  SDK_VERSION=0.17.4

  wget "https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"

  tar xf "zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"
  mv "zephyr-sdk-${SDK_VERSION}" "$HOME/zephyr-sdk"

  "$HOME/zephyr-sdk/setup.sh"

Then, set the environment variables (for the current shell):

  export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
  export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk"

Optionally, make these permanent by adding to ~/.bashrc:

  echo 'export ZEPHYR_TOOLCHAIN_VARIANT=zephyr' >> "$HOME/.bashrc"
  echo 'export ZEPHYR_SDK_INSTALL_DIR="$HOME/zephyr-sdk"' >> "$HOME/.bashrc"

After installing, open a new terminal, go to the workspace, activate the venv, and run your build, e.g.:

  cd zephyr-esp32s3-workspace
  source .venv/bin/activate
  cd zephyr-esp32s3-course
  python scripts/build.py --app welcome --clean

EOF
fi

echo "Bootstrap finished."
