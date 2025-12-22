# Slide 13 Notes

## Development Environment Setup (Windows + Linux)

- Install Zephyr prerequisites: Python, west, CMake, ninja, toolchain/SDK.
- Use the Zephyr SDK or a vendor toolchain; set `ZEPHYR_BASE` and initialize west.
- Verify with `west zephyr-export` and a sample build on your platform (Windows or Linux).

## Key idea

Consistent setup across Windows and Linux ensures reproducible builds and faster onboarding.

## Narration

Setting up Zephyr starts with Python and the `west` tool, then the build tools (CMake, ninja), and a cross-toolchain such as the Zephyr SDK. On both Windows and Linux, you clone the workspace, run `west init` and `west update`, and export environment variables via `zephyr-export`. Verify the environment by building a sample app for your target board. Keeping the steps scripted and documented means new developers can get to build-and-flash quickly on any host OS.

## References

- Zephyr Getting Started (Windows/Linux)
- Zephyr SDK installation guide
