#pragma once

#include <cstdint>
#include <vector>

#include "gpio_gpiod.h"
#include "spi_linux.h"

class Ili9488 {
public:
    Ili9488(SpiLinux& spi, GpioLine& dc, GpioLine& rst, int width = 480, int height = 320);

    void reset();
    void init();
    void clear(uint16_t rgb565 = 0x0000);

    // Accepts 1bpp page-packed framebuffer (same format as FourLineDisplay)
    void set_framebuffer_mono(const std::vector<uint8_t>& fb, uint16_t fg_rgb565 = 0xFFFF,
                              uint16_t bg_rgb565 = 0x0000);

private:
    void cmd(uint8_t value);
    void data(const uint8_t* data, size_t size);
    void data8(uint8_t value);
    void set_window(int x0, int y0, int x1, int y1);

    SpiLinux& spi_;
    GpioLine& dc_;
    GpioLine& rst_;
    int width_;
    int height_;
};

