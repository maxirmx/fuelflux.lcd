#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"
#include "ili9488.h"

TEST(Ili9488Test, MonoToRgb666ConvertsPixelsByBitLayout) {
    const int width = 2;
    const int height = 8;

    // page-major mono layout, 1 page for 8 pixels tall:
    // x=0: y0=1, y1=0, y2=1, y3=0, ... => 0b00000101
    // x=1: y0=0, y1=1, y2=0, y3=1, ... => 0b00001010
    const std::vector<uint8_t> mono = {0x05, 0x0A};

    const auto rgb = Ili9488::mono_to_rgb666(mono, width, height, 0xF800, 0x001F);

    ASSERT_EQ(rgb.size(), static_cast<size_t>(width * height * 3));

    // y0: [on, off] => [red, blue]
    // RGB666: 3 bytes per pixel (R, G, B)
    // 0xF800 (red in RGB565): red=(0xF800>>11)&0x1F=0x1F, then 0x1F<<3=0xF8
    EXPECT_EQ(rgb[0], 0xF8);  // Red channel
    EXPECT_EQ(rgb[1], 0x00);  // Green channel
    EXPECT_EQ(rgb[2], 0x00);  // Blue channel
    // 0x001F (blue in RGB565): blue=0x001F&0x1F=0x1F, then 0x1F<<3=0xF8
    EXPECT_EQ(rgb[3], 0x00);  // Red channel
    EXPECT_EQ(rgb[4], 0x00);  // Green channel
    EXPECT_EQ(rgb[5], 0xF8);  // Blue channel

    // y1: [off, on] => [blue, red]
    // 0x001F (blue in RGB565): blue=0x001F&0x1F=0x1F, then 0x1F<<3=0xF8
    EXPECT_EQ(rgb[6], 0x00);   // Red channel
    EXPECT_EQ(rgb[7], 0x00);   // Green channel
    EXPECT_EQ(rgb[8], 0xF8);   // Blue channel
    // 0xF800 (red in RGB565): red=(0xF800>>11)&0x1F=0x1F, then 0x1F<<3=0xF8
    EXPECT_EQ(rgb[9], 0xF8);   // Red channel
    EXPECT_EQ(rgb[10], 0x00);  // Green channel
    EXPECT_EQ(rgb[11], 0x00);  // Blue channel
}

TEST(Ili9488Test, MonoToRgb666RejectsInvalidSize) {
    const std::vector<uint8_t> mono = {0x00};
    EXPECT_THROW((void)Ili9488::mono_to_rgb666(mono, 128, 64), std::runtime_error);
}

TEST(Ili9488Test, MonoToRgb666RejectsInvalidGeometry) {
    const std::vector<uint8_t> mono(16, 0x00);
    EXPECT_THROW((void)Ili9488::mono_to_rgb666(mono, 8, 10), std::runtime_error);
}
