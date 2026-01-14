# Library Documentation

This project builds two libraries: `tools` and `nhd12864`. Both are C++17 and are intended for Linux targets with spidev and libgpiod. The display stack also depends on FreeType2 for UTF-8 text rendering.

## Library: tools

Purpose: low-level SPI and GPIO helpers.

### Provided headers

- `include/spi_linux.h`
- `include/gpio_gpiod.h`

### SpiLinux

Minimal wrapper around Linux spidev.

```cpp
#include "spi_linux.h"

SpiLinux spi("/dev/spidev1.0");
spi.open(8000000, 0);
// ...
spi.close();
```

Key API:

- `open(uint32_t speed_hz = 8000000, uint8_t mode = 0)`
- `write(const uint8_t* data, size_t len)`
- `close()`

### GpioLine

Wrapper for a single GPIO line using libgpiod.

```cpp
#include "gpio_gpiod.h"

GpioLine dc(262, true, false, "/dev/gpiochip0", "nhd12864-dc");
GpioLine rst(226, true, true, "/dev/gpiochip0", "nhd12864-rst");

rst.set(false);
```

Key API:

- `GpioLine(int line_offset, bool output, bool initial_value, std::string chip_path = "/dev/gpiochip0", std::string consumer = "nhd12864")`
- `set(bool value)`
- `get() const`

### SoftPwm

Simple software PWM on top of `GpioLine`.

```cpp
SoftPwm bl(dc, 200); // 200 Hz
bl.start(50);        // 50 percent duty cycle
```

Key API:

- `start(int duty_percent)`
- `set_duty(int duty_percent)`
- `stop()`

## Library: nhd12864

Purpose: display stack for ST7565-class 128x64 LCDs.

### Provided headers

- `include/st7565.h`
- `include/graphics.h`
- `include/ft_text.h`
- `include/four_line_display.h`

### St7565

Driver for ST7565-class controllers. It uses `SpiLinux` and two GPIO lines (D/C and RESET).

```cpp
#include "st7565.h"

St7565 lcd(spi, dcLine, rstLine);
lcd.reset();
lcd.init();
lcd.set_contrast(0x20);
```

Key API:

- `reset()`
- `init()`
- `set_contrast(uint8_t v)`
- `display_on(bool on)`
- `set_framebuffer(const std::vector<uint8_t>& fb)`
- `clear()`

### MonoGfx

Tiny 1bpp framebuffer helper with basic drawing primitives.

```cpp
#include "graphics.h"

MonoGfx gfx(128, 64);
gfx.clear();
gfx.rect(0, 0, 127, 63);
```

Key API:

- `clear()`
- `pixel(int x, int y, bool on = true)`
- `hline(...)`, `vline(...)`, `rect(...)`, `fill_rect(...)`
- `text(int x, int y, const std::string& s, bool on = true)`

### FtText

Minimal FreeType-based UTF-8 renderer that draws into a page-packed 1bpp framebuffer.

```cpp
#include "ft_text.h"

FtText text;
text.load_font("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
text.set_pixel_size(16);
text.draw_utf8(fb, 128, 64, 0, 16, "Status: OK");
```

Key API:

- `load_font(const std::string& font_path)`
- `set_pixel_size(int px)`
- `draw_utf8(std::vector<unsigned char>& fb, int width, int height, int x, int y, const std::string& utf8, bool on = true)`

### FourLineDisplay

High-level helper for a fixed 4-line layout (small, large, small, small). It renders text into a framebuffer compatible with the ST7565 driver.

```cpp
#include "four_line_display.h"

FourLineDisplay display(128, 64, 12, 28);
if (!display.initialize("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")) {
    return 1;
}

display.puts(0, "Status: OK");
display.puts(1, "Count: 42");
display.puts(2, "FuelFlux NHD");
display.puts(3, "Ver 2.0");

const auto& fb = display.render();
```

Key API:

- `initialize(const std::string& font_path)`
- `uninitialize()`
- `is_initialized() const`
- `length(unsigned int line_id) const`
- `puts(unsigned int line_id, const std::string& text)`
- `get_text(unsigned int line_id) const`
- `clear_line(unsigned int line_id)`
- `clear_all()`
- `render()`
- `get_framebuffer() const`

Notes:

- `render()` draws all four lines; call it after updating the text.
- Text that exceeds the line capacity is clipped.
- The library is not thread-safe; protect shared instances externally if needed.

## Linking notes

- `tools` links against libgpiod.
- `nhd12864` links against `tools` and FreeType2.
