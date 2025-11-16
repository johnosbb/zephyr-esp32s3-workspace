#!/usr/bin/env python3
"""
Create a new Zephyr app by cloning an existing one.

Script location (expected):
    zephyr-esp32s3-course/scripts/create_new_app.py

Apps directory:
    zephyr-esp32s3-course/apps/
"""

import argparse
import shutil
import sys
from pathlib import Path
import re


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Create a new Zephyr app from an existing template app."
    )

    parser.add_argument(
        "--new-app",
        required=True,
        help="Name of the new app folder to create (under apps/).",
    )
    parser.add_argument(
        "--template",
        default="welcome",
        help="Existing app folder to clone from (under apps/). Default: welcome",
    )

    # New behavior: rename is the default; user can opt out
    parser.add_argument(
        "--keep-project-name",
        action="store_true",
        help="Do NOT change the project(...) line; keep the template's project name.",
    )

    parser.add_argument(
        "--clean-artifacts",
        action="store_true",
        help="Remove common build artifacts from the cloned app.",
    )

    return parser.parse_args()


def find_paths() -> tuple[Path, Path]:
    """Locate the course root and apps root based on the script location."""
    script_path = Path(__file__).resolve()
    scripts_dir = script_path.parent
    course_root = scripts_dir.parent
    apps_root = course_root / "apps"
    return course_root, apps_root


def copy_template_app(apps_root: Path, template: str, new_app: str) -> Path:
    source = apps_root / template
    dest = apps_root / new_app

    if not source.is_dir():
        raise SystemExit(f"Template '{template}' not found under {apps_root}")

    if dest.exists():
        raise SystemExit(f"Destination '{new_app}' already exists under {apps_root}")

    shutil.copytree(source, dest)

    # Remove any nested .git directory if present
    git_dir = dest / ".git"
    if git_dir.exists():
        shutil.rmtree(git_dir)

    return dest


def clean_artifacts(dest: Path) -> None:
    """Remove common build artifacts from the cloned app."""
    dir_names = {
        "build",
        "zephyr",
        "CMakeFiles",
    }

    file_globs = {
        "CMakeCache.txt",
        "cmake_install.cmake",
        "compile_commands.json",
        "*.elf",
        "*.bin",
        "*.hex",
        "*.map",
        "*.log",
        "*.dts",
        "*.dtb",
        ".config",
    }

    for path in dest.rglob("*"):
        if path.is_dir() and path.name in dir_names:
            shutil.rmtree(path, ignore_errors=True)

    for pattern in file_globs:
        for path in dest.rglob(pattern):
            try:
                path.unlink()
            except OSError:
                pass


def rename_cmake_project(dest: Path, new_app: str) -> None:
    """
    Rewrite the project(...) line in CMakeLists.txt to match the new app name.
    """
    cmake_file = dest / "CMakeLists.txt"
    if not cmake_file.is_file():
        print(f"Warning: no CMakeLists.txt in {dest}; skipping project rename.")
        return

    text = cmake_file.read_text(encoding="utf-8")

    project_line_re = re.compile(r"^[ \t]*project\s*\([^)]+\)[ \t]*$", re.MULTILINE)
    new_line = f"project({new_app} LANGUAGES C)"

    if project_line_re.search(text):
        text = project_line_re.sub(new_line, text)
    else:
        find_pkg_re = re.compile(
            r"(find_package\s*\(\s*Zephyr[^\)]*\)\s*\r?\n)",
            re.MULTILINE,
        )
        if find_pkg_re.search(text):
            text = find_pkg_re.sub(rf"\1{new_line}\n", text)
        else:
            text = new_line + "\n" + text

    cmake_file.write_text(text, encoding="utf-8")


def main() -> None:
    args = parse_args()
    _, apps_root = find_paths()

    dest = copy_template_app(apps_root, args.template, args.new_app)

    if args.clean_artifacts:
        clean_artifacts(dest)

    # Default: rename project; only skip if user asked to keep original name
    if not args.keep_project_name:
        rename_cmake_project(dest, args.new_app)

    print(f"Created new app: {dest}")


if __name__ == "__main__":
    main()


#Usage:

# cd zephyr-esp32s3-workspace
# source .venv/bin/activate
# cd zephyr-esp32s3-course

# # Create from default template apps/welcome
# python scripts/create_new_app.py --new-app my_new_app  --clean-artifacts

# # Create from a different template app
# python scripts/create_new_app.py --new-app sensor_demo --template base_sensor --clean-artifacts