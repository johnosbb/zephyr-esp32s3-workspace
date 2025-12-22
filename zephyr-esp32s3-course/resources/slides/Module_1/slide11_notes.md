# Slide 11 Notes

## Kconfig / prj.conf Introduction

- `prj.conf` sets Kconfig symbols that shape features, drivers, and build options.
- Kconfig enforces dependencies and provides defaults for each board and module.
- Overlay configs (e.g., `boards/*.conf`) fine-tune per-board tweaks.

## Key idea

Kconfig is Zephyr’s feature switchboard; `prj.conf` is where you flip the switches for your app.

## Narration

In Zephyr, the prj.conf file is where you configure which features, drivers, and subsystems are included in your application. 
You do this by setting Kconfig symbols, which control everything from logging and networking to individual device drivers and kernel options.

Kconfig also takes care of dependencies and sensible defaults. 
If a feature depends on something else, Kconfig ensures that requirement is met, and it automatically applies board- and SoC-specific defaults where appropriate. 
This helps prevent invalid configurations and keeps builds consistent.

In addition to prj.conf, Zephyr supports overlay configuration files, such as those under boards. 
These allow you to fine-tune settings for a specific board without changing the main application configuration, 
which is especially useful when the same application targets multiple hardware platforms.

## References

- Zephyr Kconfig reference
- `prj.conf` best practices
