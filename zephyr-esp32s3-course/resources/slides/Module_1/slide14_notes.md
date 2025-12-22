# Slide 14 Notes

## VS Code Setup + Recommended Extensions

- Use the Zephyr VS Code extension (or C/C++, CMake, Python, Device Tree helpers).
- Configure the workspace to point to `ZEPHYR_BASE` and your west workspace.
- Enable formatting, intellisense, and launch configs for build/flash/debug tasks.

## Key idea

A tuned VS Code setup speeds navigation, editing, and debugging for Zephyr projects.

## Narration

VS Code becomes powerful for Zephyr when you add the right extensions: C/C++ language support, CMake tools, Python, Devicetree syntax highlighting, and the Zephyr extension if available. Point the workspace at your west root and `ZEPHYR_BASE` so includes resolve. Add tasks or launch configurations to build, flash, and open a debug session directly from the editor. With intellisense and formatting in place, you avoid context switching and keep a smooth inner loop.

## References

- Zephyr VS Code extension docs
- C/C++ and CMake Tools extension guides
