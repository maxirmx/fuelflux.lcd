# NHD12864 C++ Libraries and Demo

This project provides C++ libraries for driving displays on Linux over SPI, plus a demo application that renders a four-line status view. It targets boards like the Orange Pi Zero 2W but should work on any Linux system with spidev and libgpiod.

## Contents

- **tools**: Linux SPI and GPIO helpers (spidev + libgpiod)
- **nhd12864**: Display stack (ST7565 + ILI9488 drivers, framebuffer helpers, FreeType text, FourLineDisplay)
- **nhd12864_demo**: Sample program that updates the LCD every 500 ms

## Requirements

- CMake 3.16+
- libgpiod (dev headers)
- FreeType2 (dev headers)
- A working SPI device (for example `/dev/spidev1.0`)

On Debian/Ubuntu/Armbian:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libgpiod-dev libfreetype6-dev
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## Demo application

The demo shows a four-line status screen using `FourLineDisplay`.

Binary: `./build/nhd12864_demo`

Command line options:

- `--spidev <path>`: SPI device path (default: `/dev/spidev1.0`)
- `--chip <path>`: GPIO chip path (default: `/dev/gpiochip0`)
- `--dc <offset>`: GPIO line offset for D/C (default: `262`)
- `--rst <offset>`: GPIO line offset for RESET (default: `226`)
- `--font <path>`: TTF/OTF font path (default: `/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf`)
- `--display <type>`: Display type: `st7565` (default) or `ili9488`

Example:

```bash
sudo ./build/nhd12864_demo --spidev /dev/spidev1.0 --chip /dev/gpiochip0 --dc 262 --rst 226 \
  --font /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf

# ILI9488 3.5" TFT SPI module (480x320)
sudo ./build/nhd12864_demo --display ili9488 --spidev /dev/spidev1.0 --chip /dev/gpiochip0 --dc 262 --rst 226 \
  --font /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf
```

Notes:

- libgpiod uses GPIO line offsets, not header pin numbers. See `scripts/gpio_mapping_notes.md` if you need help mapping lines.
- If the display is mirrored or upside-down, adjust the SEG/COM direction in `src/st7565.cpp`.

MIT License in `LICENSE`.
