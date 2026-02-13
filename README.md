# ST7565/ILI9488 LCD C++ Libraries and Demo

This project provides two C++ libraries for driving 128x64 ST7565-class monochrome LCDs and 480x320 ILI9488-based TFT displays on Linux, plus a demo application that renders a four-line status view. It targets boards like the Orange Pi Zero 2W but should work on any Linux system with spidev and libgpiod.

## Contents

- **tools**: Linux SPI and GPIO helpers (spidev + libgpiod)
- **lcd_display**: Display stack (ST7565 + ILI9488 drivers, framebuffer helpers, FreeType text, FourLineDisplay)
- **lcd_demo**: Sample program that updates the LCD every 500 ms

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

The demo shows a four-line status screen using `FourLineDisplay` and can target either:

- `st7565` (default): 128x64 monochrome ST7565-class modules
- `ili9488`: 3.5" TFT 480x320 modules such as LCDWiki MSP3520 (write-only/no touch)

Binary: `./build/lcd_demo`

Command line options:

- `--spidev <path>`: SPI device path (default: `/dev/spidev1.0`)
- `--model <st7565|ili9488>`: display controller (default: `st7565`)
- `--spi-hz <hz>`: SPI clock (default: `8_000_000` for ST7565, `32_000_000` for ILI9488)
- `--chip <path>`: GPIO chip path (default: `/dev/gpiochip0`)
- `--dc <offset>`: GPIO line offset for D/C (default: `271`)
- `--rst <offset>`: GPIO line offset for RESET (default: `256`)
- `--font <path>`: TTF/OTF font path (default: `/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf`)

Example:

```bash
sudo ./build/lcd_demo --model st7565 --spidev /dev/spidev1.0 --chip /dev/gpiochip0 --dc 271 --rst 256 \
  --font /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf

# ILI9488 (LCDWiki 3.5\" SPI module MSP3520)
sudo ./build/lcd_demo --model ili9488 --spidev /dev/spidev1.0 --chip /dev/gpiochip0 --dc 271 --rst 256 \
  --spi-hz 32000000 --font /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf
```

Notes:

- libgpiod uses GPIO line offsets, not header pin numbers. See `scripts/gpio_mapping_notes.md` if you need help mapping lines.
- If the display is mirrored or upside-down, adjust the SEG/COM direction in `src/st7565.cpp`.

MIT License in `LICENSE`.
