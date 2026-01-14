# NHD-C12864A1Z-FSW-FBW-HTT on Orange Pi Zero 2W (C++)

This repository contains:
1. **Four Line Display Library** - A reusable high-level API for managing 4-line displays with mixed font sizes
2. **Low-level Hardware Driver** - ST7565 LCD driver with SPI and GPIO support

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

## Four Line Display Library

The **FourLineDisplay** library provides a high-level, reusable API for managing displays with 4 lines:
- **Line 0**: Small font (default: 12px)
- **Line 1**: Large font (default: 28px)
- **Line 2**: Small font (default: 12px)
- **Line 3**: Small font (default: 12px)

### API Reference

```cpp
#include "four_line_display.h"

// Constructor
FourLineDisplay(int width = 128, int height = 64, 
                int small_font_size = 12, int large_font_size = 28);

// Initialize the library with a font file
bool initialize(const std::string& font_path);

// Uninitialize and cleanup
void uninitialize();

// Get maximum characters per line
unsigned int length(unsigned int line_id);

// Set text for a line (line_id: 0-3)
void puts(unsigned int line_id, const std::string& text);

// Get current text
std::string get_text(unsigned int line_id) const;

// Clear operations
void clear_line(unsigned int line_id);
void clear_all();

// Render to framebuffer
const std::vector<unsigned char>& render();
```

### Example Usage

```cpp
#include "four_line_display.h"
#include "st7565.h"

// Create display (128x64, small font 12px, large font 28px)
FourLineDisplay display(128, 64, 12, 28);

// Initialize with font
if (!display.initialize("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")) {
    std::cerr << "Failed to initialize display\n";
    return 1;
}

// Check line capacities
std::cout << "Line 0 can hold " << display.length(0) << " chars\n";
std::cout << "Line 1 can hold " << display.length(1) << " chars\n";

// Set content
display.puts(0, "Status: OK");
display.puts(1, "Temp: 25°C");
display.puts(2, "Pressure: 1013");
display.puts(3, "Ver 2.0");

// Render and send to LCD
const auto& framebuffer = display.render();
lcd.set_framebuffer(framebuffer);

// Cleanup
display.uninitialize();
```

### Four Line Demo

Run the dedicated four-line demo:

```bash
sudo ./build/nhd12864_fourline_demo --spidev /dev/spidev0.0 --chip /dev/gpiochip0 \
  --dc 25 --rst 17 --font /usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf
```

## Testing

The library includes comprehensive unit tests using GoogleTest.

### Build Tests

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
cmake --build build -j
```

### Run Tests

```bash
./build/test_four_line_display
```

Or with CTest:

```bash
cd build
ctest --output-on-failure
```

### Disable Tests

To build without tests:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
cmake --build build -j
```
