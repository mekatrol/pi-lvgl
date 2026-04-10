# pi-lvgl
LVGL on Raspberry Pi with a 7-inch touch display (DRM + EVDEV).

This guide assumes:
- Fresh Raspberry Pi OS Lite (headless)
- No development tools installed yet
- You want to build and run this project from scratch

## 1. System prep (fresh machine)

Update the OS and install required packages:

```bash
sudo apt update
sudo apt full-upgrade -y
sudo apt install -y \
  git \
  build-essential \
  cmake \
  pkg-config \
  python3 \
  python3-venv \
  libdrm-dev \
  libevdev-dev
```

Reboot after upgrade:

```bash
sudo reboot
```

## 2. Enable graphics stack (DRM/KMS)

For modern LVGL DRM usage, ensure KMS is enabled.

Check `/boot/firmware/config.txt` has:

```ini
dtoverlay=vc4-kms-v3d
```

Then reboot:

```bash
sudo reboot
```

## 3. Clone project

```bash
mkdir -p ~/repos
cd ~/repos
git clone <your-repo-url> pi-lvgl
cd pi-lvgl
```

If you already copied the folder manually, just `cd` into it.

## 4. Build

### Clean build
```bash
rm -rf build
cmake -S . -B build
cmake --build build -j$(nproc)
```

### Build changes
```bash
cmake --build build -j$(nproc)
```

Expected output binary:

```bash
./build/app
```

## 5. Find display and touch devices

### 5.1 Find active DRM card

```bash
ls -l /dev/dri
for s in /sys/class/drm/card*-*/status; do
  echo "$s: $(cat "$s")"
done
```

Pick the card whose connector is `connected` (example: `card1` for `DSI-1`).

### 5.2 Find touch input event node

```bash
for e in /dev/input/event*; do
  n=$(basename "$e")
  name=$(cat "/sys/class/input/$n/device/name" 2>/dev/null)
  echo "$e -> $name"
done
```

Pick your touchscreen device (example: FT5x06 on `event4`).

## 6. Run

Example values (adjust for your hardware):

```bash
export LV_LINUX_DRM_CARD=/dev/dri/card1
export LV_LINUX_EVDEV_POINTER_DEVICE=/dev/input/event4
./build/app
```

## 7. Permissions (important)

User must have access to DRM and input devices:

```bash
groups
```

If `video` and `input` are missing:

```bash
sudo usermod -aG video,input $USER
```

Log out/in (or reboot) for group changes to apply.


## 8. Run as a systemd service

Service file in this repo:

```bash
/home/pi/repos/pi-lvgl/pi-lvgl.service
```

### 9.1 Install / set up service

```bash
sudo cp /home/pi/repos/pi-lvgl/pi-lvgl.service /etc/systemd/system/pi-lvgl.service
sudo systemctl daemon-reload
sudo systemctl enable pi-lvgl.service
```

### 9.2 Start service

```bash
sudo systemctl start pi-lvgl.service
```

### 9.3 Restart service

```bash
sudo systemctl restart pi-lvgl.service
```

### 9.4 Stop service

```bash
sudo systemctl stop pi-lvgl.service
```

### 9.5 Check status and logs

```bash
sudo systemctl status pi-lvgl.service
journalctl -u pi-lvgl.service -n 100 --no-pager
```

### 9.6 Disable autostart (keep installed)

```bash
sudo systemctl disable pi-lvgl.service
```

### 9.7 Uninstall service completely

```bash
sudo systemctl stop pi-lvgl.service || true
sudo systemctl disable pi-lvgl.service || true
sudo rm -f /etc/systemd/system/pi-lvgl.service
sudo systemctl daemon-reload
sudo systemctl reset-failed
```

## 9. Troubleshooting

### `drmGetCap DRM_CAP_DUMB_BUFFER failed`
- Wrong DRM card selected, or KMS not enabled.
- Re-check `/sys/class/drm/card*-*/status` and `config.txt`.

### `lv_evdev_create: open failed: No such file or directory`
- `LV_LINUX_EVDEV_POINTER_DEVICE` is wrong (e.g. placeholder `eventX`).
- Re-run the event discovery command and set the real `/dev/input/eventN`.

### `fatal error: drm.h: No such file or directory`
- Install DRM dev package:
  ```bash
  sudo apt install -y libdrm-dev
  ```
- Reconfigure and rebuild:
  ```bash
  cmake -S . -B build
  cmake --build build -j$(nproc)
  ```
