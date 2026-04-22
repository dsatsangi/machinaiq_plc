# MachinaIQ UPLC

Industrial HMI for Raspberry Pi Zero+ (Debian Trixie, ARM).
Qt6 Widgets · CMake · Serial + TCP/IP · Dark theme · `.deb` package.

---

## Why Qt Widgets (not QML)?

| Concern | Qt Widgets | QML |
|---------|-----------|-----|
| Pi Zero RAM | ~40 MB resident | ~80–120 MB (JS engine + QML runtime) |
| CPU (fbdev) | Low — native paint | Higher — scene graph GPU path |
| Touch | Adequate for industrial grids | Better for animation-heavy UX |
| Styling | QSS (CSS-like, powerful) | QML properties (more flexible) |
| Debug | Mature, well-documented | Debugging JS in embedded is painful |

**Verdict:** Pi Zero (1 GHz, 512 MB) fits Widgets better. QML's scene graph acceleration requires OpenGL ES, which adds latency on soft-framebuffer. For this industrial UI with no animations, Widgets wins on latency and memory.

---

## Project Structure

```
machinaiq_uplc/
├── CMakeLists.txt              Root build
├── cmake/packaging.cmake       CPack .deb settings
├── config/default_config.json  Default station hierarchy
├── debian/                     postinst, prerm scripts
├── resources/
│   ├── style.qss               Industrial dark theme
│   └── resources.qrc
├── systemd/machinaiq-uplc.service
└── src/
    ├── main.cpp
    ├── app/Application.h/.cpp  Bootstrap: config + comms + window
    ├── ui/
    │   ├── MainWindow           Stacked navigation host
    │   ├── HomeScreen           Live operational view
    │   ├── SettingsScreen       CRUD config hierarchy
    │   └── widgets/
    │       ├── StatusBanner     OK / NOK / RUNNING banner
    │       ├── ScrewCountWidget Current/total + time + program
    │       ├── ChannelSelector  CH1–CH8 touch buttons
    │       └── StatBox          Total / Pass / Reject tiles
    ├── comms/
    │   ├── CommManager          Routes inbound → State; outbound API
    │   ├── SerialHandler        QSerialPort in own QThread
    │   └── TcpHandler           QTcpSocket in own QThread, auto-reconnect
    ├── state/
    │   ├── AppState.h           Plain struct + enums
    │   └── StateManager         Thread-safe singleton, emits on main thread
    └── config/
        ├── ConfigManager        JSON load/save, hot-reload
        └── models/HierarchyModels.h
```

---

## Build (cross-compile on x86 host for ARM, or native on Pi)

### Prerequisites

```bash
# On Raspberry Pi OS / Debian Trixie (ARM)
sudo apt install \
    build-essential cmake ninja-build \
    qt6-base-dev qt6-serialport-dev \
    libqt6network6-dev
```

### Configure & Build

```bash
cd machinaiq_uplc
cmake -B build -G Ninja \
    -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)
```

### Run locally (dev)

```bash
# Simulated framebuffer (X11 desktop)
QT_QPA_PLATFORM=xcb ./build/machinaiq_uplc \
    --config config/default_config.json \
    --serial-port /dev/ttyUSB0

# Real Pi framebuffer
./build/machinaiq_uplc \
    --config /etc/machinaiq_uplc/config.json \
    --serial-port /dev/ttyUSB0 \
    --tcp-host 192.168.1.100 --tcp-port 5000
```

---

## Package as .deb

```bash
cd build
cpack -G DEB
# Produces: machinaiq-uplc_1.0.0_armhf.deb
```

### Install on Pi

```bash
sudo dpkg -i machinaiq-uplc_1.0.0_armhf.deb
sudo apt install -f   # resolve any missing deps
```

The `postinst` script:
1. Creates `/etc/machinaiq_uplc/` and copies `config.json`.
2. Enables and starts `machinaiq-uplc.service`.

### Service management

```bash
sudo systemctl status  machinaiq-uplc
sudo systemctl restart machinaiq-uplc
sudo journalctl -u machinaiq-uplc -f   # live logs
```

---

## Configuration

Edit `/etc/machinaiq_uplc/config.json` (JSON). Changes take effect on next
app start, or call `ConfigManager::instance().reload()` programmatically.

See `config/default_config.json` for a 2-screwdriver / 3-pattern example.

**Hierarchy limits:** Station → Screwdriver (≤5) → Pattern (≤5) → Channel (≤8)

---

## Assumed Communication Protocol

See `SKILL.md` for the full inbound/outbound frame table.
Short summary: newline-delimited ASCII, e.g.:

```
# Device → HMI
SCREW:3:4:142
STATUS:OK
COUNTER:7:7:0

# HMI → Device
SET_CHANNEL:3
RESET
REVERSE:ENABLE
```

---

## Cross-compile (optional, faster builds)

```bash
sudo apt install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf
# Then use a Qt6 ARM sysroot or build with dockcross
# cmake -DCMAKE_TOOLCHAIN_FILE=cmake/arm-toolchain.cmake ...
```

---

## Extending the app

Read `SKILL.md` — it covers adding commands, screens, config fields, and lists anti-patterns to avoid.
