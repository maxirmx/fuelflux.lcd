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
    void set_rotation(uint8_t rotation);

    void fill(uint16_t color565);
    void set_mono_framebuffer(const std::vector<uint8_t>& fb,
                              uint16_t fg_color565 = 0xFFFF,
                              uint16_t bg_color565 = 0x0000);

    static std::vector<uint8_t> mono_to_rgb565(const std::vector<uint8_t>& mono_fb,
                                               int width,
                                               int height,
                                               uint16_t fg_color565 = 0xFFFF,
                                               uint16_t bg_color565 = 0x0000);

private:
    void cmd(uint8_t b);
    void data(const uint8_t* p, size_t n);
    void set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);

    SpiLinux& spi_;
    GpioLine& dc_;
    GpioLine& rst_;
    int w_;
    int h_;
};
