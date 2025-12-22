# Repository Guidelines
## Project Structure & Module Organization
- Workspace root is a west workspace; `zephyr-esp32s3-course` is the manifest repo alongside `zephyr/`, `modules/`, `.west/`, and `.venv/` created by the bootstrap scripts.
- Course layout: `apps/` holds per-app Zephyr projects (CMakeLists.txt, prj.conf, src/, optional boards/ overlays), `scripts/` contains helpers (`build.py`, `create_new_app.py`), `docs/` and `slides/` store course material, and generated output lives under `build/<app>/`.
- Avoid committing build artifacts; prefer creating new apps via the helper script so naming and project() lines stay consistent.

## Build, Test, and Development Commands
- One-time setup: run `./scripts/bootstrap.sh` (Linux/macOS) or `./scripts/bootstrap.ps1` (Windows), then activate `.venv` before building.
- Main path: `python scripts/build.py --app welcome --clean --flash --monitor --port COM12` (runs west build/flash with the default overlay).
- Manual west alternative: `west build -b esp32s3_devkitc/esp32s3/procpu -d build/welcome apps/welcome -- -DDTC_OVERLAY_FILE=boards/esp32s3_devkitc.overlay` then `west flash -d build/welcome --esp-device COM12` if you need finer control.
- Scaffold a new app from `apps/welcome`: `python scripts/create_new_app.py --new-app sensor_demo --clean-artifacts`.

## Coding Style & Naming Conventions
- Follow Zephyr C style (tabs, K&R braces, use const, minimize globals); keep one module per file where practical.
- App folders, overlays, and config files use lower_snake_case; Kconfig symbols stay uppercase with underscores.
- Store overlays in `boards/`, runtime config in `prj.conf`, and shared headers inside each app (`src/` or `include/`).

## Testing Guidelines
- Always build once with `--clean` when configs or overlays change to avoid stale artifacts.
- For automated checks, add `tests/` under an app using Zephyr's `twister`; run `west twister -T apps/<app>/tests` when present.
- Capture serial/monitor output for feature changes and note expected log lines in PRs.

## Commit & Pull Request Guidelines
- Commits: short, present-tense, imperative (`add second application`, `fix overlay path`), and scoped to one logical change.
- PRs: describe intent, mention board/port used, list commands run (build/flash/tests), and attach relevant logs or screenshots. Link issues when applicable and request review after local checks pass.
