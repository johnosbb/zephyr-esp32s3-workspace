param(
  [string]$SdkDir = "C:\Users\johno\zephyr-sdk-0.17.4"  # change if you installed elsewhere
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# Move to workspace root (this script lives in zephyr-esp32s3-workspace\scripts)
Set-Location (Join-Path $PSScriptRoot '..')

# --- Python virtual environment + west ---
python -m venv .venv
. .\.venv\Scripts\Activate.ps1
python -m pip install --upgrade pip
pip install west

# --- Initialize west workspace using the course subdir as manifest ---
west init -l zephyr-esp32s3-course
west update
west zephyr-export

# --- Zephyr Python requirements ---
# Zephyr tree will be at .\zephyr after west update
pip install -r zephyr\scripts\requirements-base.txt
pip install pyelftools

# --- Verify SDK ---
$gdb = Join-Path $SdkDir 'xtensa-espressif_esp32s3_zephyr-elf\bin\xtensa-espressif_esp32s3_zephyr-elf-gdb.exe'
if (-not (Test-Path $gdb)) {
    Write-Host @"
Zephyr SDK not detected.

1) Download and install the Windows SDK:
   https://github.com/zephyrproject-rtos/sdk-ng/releases

2) Set environment variables (replace path if needed):
   setx ZEPHYR_TOOLCHAIN_VARIANT zephyr
   setx ZEPHYR_SDK_INSTALL_DIR "$SdkDir"

Then open a NEW PowerShell and rerun builds.
"@
    exit 0
}

# Persist env for future shells; current shell also needs them
setx ZEPHYR_TOOLCHAIN_VARIANT zephyr | Out-Null
setx ZEPHYR_SDK_INSTALL_DIR "$SdkDir" | Out-Null
$env:ZEPHYR_TOOLCHAIN_VARIANT = 'zephyr'
$env:ZEPHYR_SDK_INSTALL_DIR = $SdkDir

Write-Host "Bootstrap complete. SDK: $env:ZEPHYR_SDK_INSTALL_DIR"
