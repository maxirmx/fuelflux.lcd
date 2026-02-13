#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"
#include "ili9488.h"

TEST(Ili9488Test, MonoToRgb565ConvertsPixelsByBitLayout) {
    const int width = 2;
    const int height = 8;

    // page-major mono layout, 1 page for 8 pixels tall:
    // x=0: y0=1, y1=0, y2=1, y3=0, ... => 0b00000101
    // x=1: y0=0, y1=1, y2=0, y3=1, ... => 0b00001010
    const std::vector<uint8_t> mono = {0x05, 0x0A};

    const auto rgb = Ili9488::mono_to_rgb565(mono, width, height, 0xF800, 0x001F);

    ASSERT_EQ(rgb.size(), static_cast<size_t>(width * height * 2));

    // y0: [on, off] => [red, blue]
    EXPECT_EQ(rgb[0], 0xF8);
    EXPECT_EQ(rgb[1], 0x00);
    EXPECT_EQ(rgb[2], 0x00);
    EXPECT_EQ(rgb[3], 0x1F);

    // y1: [off, on] => [blue, red]
    EXPECT_EQ(rgb[4], 0x00);
    EXPECT_EQ(rgb[5], 0x1F);
    EXPECT_EQ(rgb[6], 0xF8);
    EXPECT_EQ(rgb[7], 0x00);
}

TEST(Ili9488Test, MonoToRgb565RejectsInvalidSize) {
    const std::vector<uint8_t> mono = {0x00};
    EXPECT_THROW((void)Ili9488::mono_to_rgb565(mono, 128, 64), std::runtime_error);
}

TEST(Ili9488Test, MonoToRgb565RejectsInvalidGeometry) {
    const std::vector<uint8_t> mono(16, 0x00);
    EXPECT_THROW((void)Ili9488::mono_to_rgb565(mono, 8, 10), std::runtime_error);
}
