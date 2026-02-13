#include "ili9488.h"

#include <algorithm>
#include <chrono>
#include <stdexcept>
#include <thread>

Ili9488::Ili9488(SpiLinux& spi, GpioLine& dc, GpioLine& rst, int width, int height)
    : spi_(spi), dc_(dc), rst_(rst), width_(width), height_(height) {}

void Ili9488::cmd(uint8_t value) {
    dc_.set(false);
    spi_.write(&value, 1);
}

void Ili9488::data(const uint8_t* payload, size_t size) {
    dc_.set(true);
    spi_.write(payload, size);
}

void Ili9488::data8(uint8_t value) { data(&value, 1); }

void Ili9488::reset() {
    rst_.set(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    rst_.set(true);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
}

void Ili9488::init() {
    // Generic ILI9488 bring-up sequence for 4-wire SPI write-only mode.
    cmd(0x01); // Software reset
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    cmd(0x11); // Sleep out
    std::this_thread::sleep_for(std::chrono::milliseconds(120));

    cmd(0x3A); // Interface pixel format
    data8(0x55); // 16-bit/pixel (RGB565)

    cmd(0x36); // Memory access control
    data8(0x28); // BGR order + row/column direction for landscape 480x320

    cmd(0x21); // Display inversion on (typical for ILI9488 modules)
    cmd(0x29); // Display on
}

void Ili9488::set_window(int x0, int y0, int x1, int y1) {
    uint8_t col[] = {
        static_cast<uint8_t>((x0 >> 8) & 0xFF),
        static_cast<uint8_t>(x0 & 0xFF),
        static_cast<uint8_t>((x1 >> 8) & 0xFF),
        static_cast<uint8_t>(x1 & 0xFF),
    };

    uint8_t row[] = {
        static_cast<uint8_t>((y0 >> 8) & 0xFF),
        static_cast<uint8_t>(y0 & 0xFF),
        static_cast<uint8_t>((y1 >> 8) & 0xFF),
        static_cast<uint8_t>(y1 & 0xFF),
    };

    cmd(0x2A); // Column address set
    data(col, sizeof(col));

    cmd(0x2B); // Row address set
    data(row, sizeof(row));

    cmd(0x2C); // Memory write
}

void Ili9488::clear(uint16_t rgb565) {
    set_window(0, 0, width_ - 1, height_ - 1);

    std::vector<uint8_t> chunk(4096);
    for (size_t i = 0; i + 1 < chunk.size(); i += 2) {
        chunk[i] = static_cast<uint8_t>((rgb565 >> 8) & 0xFF);
        chunk[i + 1] = static_cast<uint8_t>(rgb565 & 0xFF);
    }

    dc_.set(true);
    int total_pixels = width_ * height_;
    while (total_pixels > 0) {
        const int pixels_this_chunk = std::min(total_pixels, static_cast<int>(chunk.size() / 2));
        spi_.write(chunk.data(), static_cast<size_t>(pixels_this_chunk * 2));
        total_pixels -= pixels_this_chunk;
    }
}

void Ili9488::set_framebuffer_mono(const std::vector<uint8_t>& fb, uint16_t fg_rgb565,
                                   uint16_t bg_rgb565) {
    const int expected_size = width_ * (height_ / 8);
    if (static_cast<int>(fb.size()) != expected_size) {
        throw std::runtime_error("Framebuffer size mismatch for ILI9488 mono input");
    }

    set_window(0, 0, width_ - 1, height_ - 1);

    // Build a full-frame RGB565 buffer and write it in a single SPI transaction
    std::vector<uint8_t> frame(static_cast<size_t>(width_) * static_cast<size_t>(height_) * 2);
    dc_.set(true);

    for (int y = 0; y < height_; ++y) {
        const int page = y / 8;
        const uint8_t mask = static_cast<uint8_t>(1u << (y % 8));
        for (int x = 0; x < width_; ++x) {
            const bool on =
                (fb[static_cast<size_t>(page * width_ + x)] & mask) != 0;
            const uint16_t pixel = on ? fg_rgb565 : bg_rgb565;
            const size_t pixel_index =
                static_cast<size_t>(y) * static_cast<size_t>(width_) +
                static_cast<size_t>(x);
            const size_t byte_index = pixel_index * 2;
            frame[byte_index] = static_cast<uint8_t>((pixel >> 8) & 0xFF);
            frame[byte_index + 1] = static_cast<uint8_t>(pixel & 0xFF);
        }
    }

    spi_.write(frame.data(), frame.size());
}

