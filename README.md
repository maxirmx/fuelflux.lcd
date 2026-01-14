# NHD-C12864A1Z-FSW-FBW-HTT on Orange Pi Zero 2W (C++)

This is a **C++** bundle for Orange Pi Zero 2W + Armbian/Ubuntu, using:
- SPI via Linux **spidev** (`/dev/spidev1.0` recommended on Zero 2W header)
- GPIO via **libgpiod** (`/dev/gpiochip*`)
- ST7565-class init + 128×64 framebuffer + tiny 5×7 font
- Optional backlight PWM (software) and heater on/off (MOSFET) via GPIO
- Optional `systemd` service + install helper

## 1) Enable SPI on Orange Pi Zero 2W (Armbian)

Use either:
```bash
sudo armbian-config
# System → Hardware → enable spi-spidev → reboot
```

After reboot you should see `/dev/spidev0.0`.

## 2) Wiring (recommended pins)

Using the same wiring as the Python bundle (header **physical** pin numbers):

- LCD SCL  → OPi **23** (SPI0 SCLK)
- LCD SI   → OPi **19** (SPI0 MOSI)
- LCD /CS  → OPi **24** (SPI0 CE0)
- LCD A0   → OPi **22** (GPIO25 suggested)
- LCD /RST → OPi **11** (GPIO17 suggested)
- LCD VDD  → **3V3** (pin 1 or 17)
- LCD VSS  → **GND**
- LCD LED+ → **3V3 rail** (not a GPIO)
- LCD LED- → GND or MOSFET drain (for dimming)
- Heater H+ → **+12 V**, Heater H- → MOSFET drain (low-side switch)

## 3) Dependencies

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libgpiod-dev libfreetype6-dev fonts-dejavu-core
```

## 4) IMPORTANT: GPIO line numbering on Orange Pi

`libgpiod` uses **GPIO line offsets on a gpiochip**, not “header pin numbers”.
On Armbian images, the offsets often (but not always) match familiar GPIO numbers.

Find the correct chip + line offsets like this:
```bash
ls -l /dev/gpiochip*
gpioinfo /dev/gpiochip0 | less
gpioinfo /dev/gpiochip1 | less
```

Look for lines that correspond to the pins you wired (DC and RST). If you’re unsure,
temporarily export a pin in a safe way by toggling it with a small script/tool and watching
`gpioinfo` “consumer” / state changes.

This bundle defaults to:
- DC line offset **25**
- RST line offset **17**
- Backlight PWM line offset **18**
- Heater line offset **23**
on `/dev/gpiochip0`.

If those don’t match your image/device-tree, change them in:
- CLI flags (`--dc`, `--rst`, `--chip`), or
- `/etc/nhd12864/nhd12864.conf` after install.

## 5) Build

```bash
mkdir -p build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

## 6) Run

LCD demo (requires root for GPIO access on most setups):
```bash
sudo ./build/nhd12864_demo --spidev /dev/spidev0.0 --chip /dev/gpiochip0 --dc 25 --rst 17
```

## Troubleshooting

- If the LCD is blank: confirm `/dev/spidev0.0` exists and wiring of **A0/DC** and **RST** is correct.
- If image is mirrored/upside-down: in `src/st7565.cpp` adjust:
  - `0xA0` ↔ `0xA1` (SEG direction)
  - `0xC0` ↔ `0xC8` (COM direction)
- If SPI is noisy: reduce speed in `src/main_demo.cpp` (`spi.open(8000000, 0)` → 1000000).

MIT License in `LICENSE`.


## UTF-8 (English + Russian/Cyrillic) rendering

This bundle now uses **FreeType** to render UTF-8 text (including Cyrillic and Ё/ё).

Default font:
- `/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf`

Run with explicit font/size:

```bash
sudo ./build/nhd12864_demo --spidev /dev/spidev1.0 --chip /dev/gpiochip0 --dc 262 --rst 226 \
  --font /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf --px 16
```
