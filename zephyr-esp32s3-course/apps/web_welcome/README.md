# Web Welcome (Zephyr Networking Intro)

This example introduces Zephyr networking with a simple HTTP server. The device
connects to Wi-Fi, obtains an IPv4 address via DHCP, and serves a single modern
welcome page.

## Configure Wi-Fi
If `wifi_secrets.conf` is missing, it will be auto-created on the first build.
Edit `wifi_secrets.conf` and set:
- `CONFIG_WIFI_CREDENTIALS_STATIC_SSID`
- `CONFIG_WIFI_CREDENTIALS_STATIC_PASSWORD`

If your network is open, change the security type in `prj.conf` to:
```
CONFIG_WIFI_CREDENTIALS_STATIC_TYPE_OPEN=y
```

## Build/flash
```bash
python scripts/build.py --app web_welcome --clean --flash --monitor --port <COMx-or-/dev/ttyUSBx>
```

## Usage
- Watch the log for the DHCP IPv4 address.
- Open `http://<device-ip>/` in your browser (the app also prints this as `Browser: http://.../`).
