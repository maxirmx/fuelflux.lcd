# Four Line Display Library Documentation

## Overview

The **FourLineDisplay** library provides a high-level, reusable API for managing 4-line displays with mixed font sizes. It is designed specifically for 128x64 monochrome displays but supports custom dimensions.

## Features

- **4-line layout** with configurable font sizes
- **Mixed font rendering**: Line 1 uses a large font, others use small font
- **UTF-8 support**: Render text in multiple languages including Cyrillic, emojis, etc.
- **Automatic character counting**: Query maximum characters per line
- **Simple API**: Initialize, set text, render
- **Thread-safe design**: No global state
- **Resource management**: Proper cleanup with RAII

## Display Layout

Default layout for 128x64 display:
```
+---------------------------+
| Line 0: Small (12px)      |  <- 0-11px
+---------------------------+
| Line 1: LARGE (28px)      |  <- 12-39px
+---------------------------+
| Line 2: Small (12px)      |  <- 40-51px
+---------------------------+
| Line 3: Small (12px)      |  <- 52-63px
+---------------------------+
```

## API Reference

### Constructor

```cpp
FourLineDisplay(int width = 128, 
                int height = 64,
                int small_font_size = 12, 
                int large_font_size = 28);
```

**Parameters:**
- `width` - Display width in pixels (default: 128)
- `height` - Display height in pixels (default: 64)
- `small_font_size` - Font size for lines 0, 2, 3 in pixels (default: 12)
- `large_font_size` - Font size for line 1 in pixels (default: 28)

**Example:**
```cpp
// Default 128x64 display
FourLineDisplay display;

// Custom configuration
FourLineDisplay custom_display(64, 32, 8, 16);
```

### Initialization

#### initialize()
```cpp
bool initialize(const std::string& font_path);
```

Initialize the display library with a TrueType font.

**Parameters:**
- `font_path` - Path to TTF or OTF font file

**Returns:**
- `true` on success
- `false` on failure (font not found or invalid)

**Example:**
```cpp
if (!display.initialize("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")) {
    std::cerr << "Failed to initialize\n";
    return 1;
}
```

#### uninitialize()
```cpp
void uninitialize();
```

Release all resources and cleanup. Called automatically by destructor.

**Example:**
```cpp
display.uninitialize();
```

#### is_initialized()
```cpp
bool is_initialized() const;
```

Check if the library is initialized.

**Returns:**
- `true` if initialized
- `false` otherwise

### Text Operations

#### length()
```cpp
unsigned int length(unsigned int line_id) const;
```

Get the maximum number of characters that can fit on a line.

**Parameters:**
- `line_id` - Line identifier (0-3)

**Returns:**
- Number of characters that fit on the line
- `0` if `line_id` is invalid

**Example:**
```cpp
unsigned int max_chars = display.length(0);
std::cout << "Line 0 can hold " << max_chars << " characters\n";
// Output: "Line 0 can hold 17 characters"
```

**Typical values for 128px width:**
- Lines 0, 2, 3 (12px font): ~17 characters
- Line 1 (28px font): ~7 characters

#### puts()
```cpp
void puts(unsigned int line_id, const std::string& text);
```

Set text for a specific line. Supports UTF-8 encoded strings.

**Parameters:**
- `line_id` - Line identifier (0-3)
- `text` - UTF-8 encoded text to display

**Example:**
```cpp
display.puts(0, "Status: OK");
display.puts(1, "25.5°C");
display.puts(2, "Привет мир");  // Russian text
display.puts(3, "Ver 1.0");
```

**Notes:**
- Text exceeding line capacity will be clipped during rendering
- Previous text on the line is overwritten
- Invalid line IDs are silently ignored

#### get_text()
```cpp
std::string get_text(unsigned int line_id) const;
```

Get the current text for a line.

**Parameters:**
- `line_id` - Line identifier (0-3)

**Returns:**
- Current text string
- Empty string if `line_id` is invalid

**Example:**
```cpp
std::string line1 = display.get_text(1);
std::cout << "Line 1 contains: " << line1 << "\n";
```

#### clear_line()
```cpp
void clear_line(unsigned int line_id);
```

Clear text from a specific line.

**Parameters:**
- `line_id` - Line identifier (0-3)

**Example:**
```cpp
display.clear_line(2);  // Clear line 2 only
```

#### clear_all()
```cpp
void clear_all();
```

Clear text from all lines.

**Example:**
```cpp
display.clear_all();  // Clear all 4 lines
```

### Rendering

#### render()
```cpp
const std::vector<unsigned char>& render();
```

Render all lines to the framebuffer.

**Returns:**
- Reference to framebuffer (page-packed 1bpp format)

**Example:**
```cpp
const auto& fb = display.render();
lcd.set_framebuffer(fb);
```

**Notes:**
- Framebuffer format: page-packed 1bpp (same as ST7565)
- Size: (width × height) / 8 bytes
- For 128×64: 1024 bytes

#### get_framebuffer()
```cpp
const std::vector<unsigned char>& get_framebuffer() const;
```

Get the current framebuffer without re-rendering.

**Returns:**
- Reference to current framebuffer

**Example:**
```cpp
const auto& fb = display.get_framebuffer();
// Use cached framebuffer
```

### Properties

#### get_width(), get_height()
```cpp
int get_width() const;
int get_height() const;
```

Get display dimensions.

**Example:**
```cpp
std::cout << "Display: " << display.get_width() 
          << "x" << display.get_height() << "\n";
```

#### get_small_font_size(), get_large_font_size()
```cpp
int get_small_font_size() const;
int get_large_font_size() const;
```

Get configured font sizes.

**Example:**
```cpp
std::cout << "Small font: " << display.get_small_font_size() << "px\n";
std::cout << "Large font: " << display.get_large_font_size() << "px\n";
```

## Complete Example

```cpp
#include "four_line_display.h"
#include "spi_linux.h"
#include "gpio_gpiod.h"
#include "st7565.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    try {
        // Initialize hardware
        SpiLinux spi("/dev/spidev1.0");
        spi.open(8000000, 0);
        
        GpioLine dcLine(262, true, false, "/dev/gpiochip0", "dc");
        GpioLine rstLine(226, true, true, "/dev/gpiochip0", "rst");
        
        St7565 lcd(spi, dcLine, rstLine);
        lcd.reset();
        lcd.init();
        
        // Initialize display library
        FourLineDisplay display(128, 64, 12, 28);
        if (!display.initialize("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")) {
            std::cerr << "Failed to initialize display\n";
            return 1;
        }
        
        // Show line capacities
        std::cout << "Line capacities:\n";
        for (int i = 0; i < 4; i++) {
            std::cout << "  Line " << i << ": " 
                      << display.length(i) << " chars\n";
        }
        
        int counter = 0;
        while (true) {
            // Update content
            display.puts(0, "System Active");
            display.puts(1, "T:" + std::to_string(25 + (counter % 10)));
            display.puts(2, "Pressure: 1013");
            display.puts(3, "Count: " + std::to_string(counter));
            
            // Render and display
            const auto& fb = display.render();
            lcd.set_framebuffer(fb);
            
            counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        
        display.uninitialize();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
```

## Font Recommendations

### Monospace Fonts
For best results, use monospace fonts:

**Linux:**
- DejaVu Sans Mono: `/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf`
- Liberation Mono: `/usr/share/fonts/truetype/liberation/LiberationMono-Regular.ttf`
- Ubuntu Mono: `/usr/share/fonts/truetype/ubuntu/UbuntuMono-R.ttf`

**Windows:**
- Consolas: `C:\Windows\Fonts\consola.ttf`
- Courier New: `C:\Windows\Fonts\cour.ttf`

### Font Size Guidelines

For 128×64 displays:
- **Small font**: 8-14px (recommended: 12px)
- **Large font**: 20-32px (recommended: 28px)

For custom displays, adjust proportionally:
- Small font ≈ height / 5.3
- Large font ≈ height / 2.3

## Error Handling

The library uses exceptions for critical errors and returns false/empty values for non-critical ones:

**Critical errors (throw exceptions):**
- Font rendering failures (from FreeType)
- Memory allocation failures

**Non-critical errors (return false/0/empty):**
- Invalid line IDs
- Initialization failures (returns false)
- Missing font files (returns false)

**Example:**
```cpp
try {
    FourLineDisplay display;
    
    // Returns false if font not found
    if (!display.initialize("nonexistent.ttf")) {
        std::cerr << "Font not found\n";
        return 1;
    }
    
    // Silently ignored (invalid line ID)
    display.puts(99, "Invalid");
    
    // May throw if FreeType fails
    const auto& fb = display.render();
    
} catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
    return 1;
}
```

## Thread Safety

The library is **not thread-safe**. If you need to access the same `FourLineDisplay` instance from multiple threads, use external synchronization:

```cpp
std::mutex display_mutex;

void update_display(FourLineDisplay& display, int line, const std::string& text) {
    std::lock_guard<std::mutex> lock(display_mutex);
    display.puts(line, text);
    display.render();
}
```

## Performance Considerations

1. **Rendering**: `render()` clears and re-renders all lines. For best performance:
   - Only call `render()` when content changes
   - Batch multiple `puts()` calls before rendering

2. **Character width estimation**: The `length()` method uses an estimate. Actual capacity may vary slightly based on font and specific characters used.

3. **Memory**: The library allocates:
   - Two FreeType font renderers (~100KB each)
   - One framebuffer (1024 bytes for 128×64)
   - Graphics context

## Limitations

1. **Fixed layout**: The 4-line structure with line 1 large is hard-coded
2. **No word wrapping**: Text exceeding line capacity is clipped
3. **Monochrome only**: 1bpp rendering
4. **Page alignment**: Font sizes should align reasonably with page boundaries for best results

## Advanced Usage

### Custom Dimensions

```cpp
// 64x32 OLED display
FourLineDisplay small_display(64, 32, 6, 14);
```

### Dynamic Content

```cpp
void update_sensor_data(FourLineDisplay& display, float temp, float pressure) {
    char buf[32];
    snprintf(buf, sizeof(buf), "T:%.1f°C", temp);
    display.puts(0, buf);
    
    snprintf(buf, sizeof(buf), "P:%.0fhPa", pressure);
    display.puts(2, buf);
    
    display.render();
}
```

### Multiple Displays

```cpp
FourLineDisplay display1(128, 64, 12, 28);
FourLineDisplay display2(128, 64, 12, 28);

display1.initialize(font_path);
display2.initialize(font_path);

display1.puts(0, "Display 1");
display2.puts(0, "Display 2");
```

## Troubleshooting

**Problem**: Text doesn't appear  
**Solution**: Verify initialization succeeded and font file exists

**Problem**: Text is clipped  
**Solution**: Check `length()` for line capacity, reduce text length

**Problem**: Incorrect character count from `length()`  
**Solution**: The estimate assumes monospace; actual capacity varies

**Problem**: Blurry or misaligned text  
**Solution**: Adjust font sizes to better align with page boundaries (multiples of 8)

**Problem**: Crashes during render()  
**Solution**: Ensure library is initialized and font is valid
