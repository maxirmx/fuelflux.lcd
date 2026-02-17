#include <gtest/gtest.h>
#include "ft_text.h"
#include <string>
#include <vector>
#include <fstream>

// Test fixture for FtText tests
class FtTextTest : public ::testing::Test {
protected:
    void SetUp() override {
        ft_text = std::make_unique<FtText>();
    }

    void TearDown() override {
        ft_text.reset();
    }

    std::unique_ptr<FtText> ft_text;
    
    // Helper function to check if a font file exists
    bool font_exists(const std::string& path) {
        std::ifstream file(path);
        return file.good();
    }
    
    // Helper function to count non-zero bytes in framebuffer
    size_t count_nonzero_bytes(const std::vector<unsigned char>& fb) {
        size_t count = 0;
        for (auto byte : fb) {
            if (byte != 0) count++;
        }
        return count;
    }
    
    // Helper function to check if a specific pixel is set
    bool is_pixel_set(const std::vector<unsigned char>& fb, int width, int x, int y) {
        int page = y / 8;
        int bit = y % 8;
        size_t idx = page * width + x;
        if (idx >= fb.size()) return false;
        return (fb[idx] & (1 << bit)) != 0;
    }
};

// Test: Constructor creates valid object
TEST_F(FtTextTest, ConstructorSucceeds) {
    EXPECT_NE(ft_text, nullptr);
}

// Test: Load font with invalid path fails
TEST_F(FtTextTest, LoadFontWithInvalidPathFails) {
    EXPECT_THROW(ft_text->load_font("/nonexistent/path/to/font.ttf"), std::runtime_error);
}

// Test: Load font with valid path succeeds
TEST_F(FtTextTest, LoadFontWithValidPathSucceeds) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    // Skip test if font doesn't exist
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    EXPECT_NO_THROW(ft_text->load_font(font_path));
}

// Test: Set pixel size without loading font
TEST_F(FtTextTest, SetPixelSizeWithoutFont) {
    EXPECT_NO_THROW(ft_text->set_pixel_size(16));
    EXPECT_NO_THROW(ft_text->set_pixel_size(12));
    EXPECT_NO_THROW(ft_text->set_pixel_size(24));
}

// Test: Set pixel size after loading font
TEST_F(FtTextTest, SetPixelSizeWithFont) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    EXPECT_NO_THROW(ft_text->set_pixel_size(16));
    EXPECT_NO_THROW(ft_text->set_pixel_size(12));
    EXPECT_NO_THROW(ft_text->set_pixel_size(24));
}

// Test: Draw UTF-8 without loading font fails
TEST_F(FtTextTest, DrawUtf8WithoutFontFails) {
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    EXPECT_THROW(ft_text->draw_utf8(fb, 128, 64, 0, 0, "Test"), std::runtime_error);
}

// Test: Draw empty string doesn't crash
TEST_F(FtTextTest, DrawEmptyString) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    EXPECT_NO_THROW(ft_text->draw_utf8(fb, 128, 64, 0, 0, ""));
    
    // Framebuffer should remain all zeros
    EXPECT_EQ(count_nonzero_bytes(fb), 0u);
}

// Test: Draw simple ASCII text
TEST_F(FtTextTest, DrawSimpleAsciiText) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Hello");
    
    // Framebuffer should have non-zero bytes (rendered text)
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw Russian (Cyrillic) text
TEST_F(FtTextTest, DrawRussianText) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // "Привет" (Hello in Russian)
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Привет");
    
    // Framebuffer should have non-zero bytes (rendered text)
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw multiple Russian words
TEST_F(FtTextTest, DrawMultipleRussianWords) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // "Привет Мир" (Hello World in Russian)
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Привет Мир");
    
    // Framebuffer should have non-zero bytes (rendered text)
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw mixed ASCII and Russian text
TEST_F(FtTextTest, DrawMixedAsciiAndRussianText) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // Mixed text: "Hello Привет"
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Hello Привет");
    
    // Framebuffer should have non-zero bytes (rendered text)
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw text with newline
TEST_F(FtTextTest, DrawTextWithNewline) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Line1\nLine2");
    
    // Framebuffer should have non-zero bytes (rendered text)
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw Russian text with newline
TEST_F(FtTextTest, DrawRussianTextWithNewline) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // "Строка1\nСтрока2" (Line1\nLine2 in Russian)
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Строка1\nСтрока2");
    
    // Framebuffer should have non-zero bytes (rendered text)
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw at different positions
TEST_F(FtTextTest, DrawAtDifferentPositions) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(12);
    
    // Draw at (0,0)
    std::vector<unsigned char> fb1(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb1, 128, 64, 0, 0, "X");
    size_t count1 = count_nonzero_bytes(fb1);
    
    // Draw at (10,10)
    std::vector<unsigned char> fb2(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb2, 128, 64, 10, 10, "X");
    size_t count2 = count_nonzero_bytes(fb2);
    
    // Both should have rendered something
    EXPECT_GT(count1, 0u);
    EXPECT_GT(count2, 0u);
    
    // The framebuffers should be different (text at different positions)
    EXPECT_NE(fb1, fb2);
}

// Test: Draw with different pixel sizes
TEST_F(FtTextTest, DrawWithDifferentPixelSizes) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    
    // Draw with size 12
    ft_text->set_pixel_size(12);
    std::vector<unsigned char> fb1(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb1, 128, 64, 0, 0, "Test");
    size_t count1 = count_nonzero_bytes(fb1);
    
    // Draw with size 24
    ft_text->set_pixel_size(24);
    std::vector<unsigned char> fb2(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb2, 128, 64, 0, 0, "Test");
    size_t count2 = count_nonzero_bytes(fb2);
    
    // Both should have rendered something
    EXPECT_GT(count1, 0u);
    EXPECT_GT(count2, 0u);
    
    // Larger font should typically use more pixels
    EXPECT_GT(count2, count1);
}

// Test: Draw with 'on' parameter set to false (should not draw)
TEST_F(FtTextTest, DrawWithOnParameterFalse) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    // First fill with all ones
    std::vector<unsigned char> fb(128 * 64 / 8, 0xFF);
    
    // Draw with on=false should clear pixels
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "Test", false);
    
    // Some bytes should now be less than 0xFF
    bool has_changed = false;
    for (auto byte : fb) {
        if (byte != 0xFF) {
            has_changed = true;
            break;
        }
    }
    EXPECT_TRUE(has_changed);
}

// Test: Reload font with different font file
TEST_F(FtTextTest, ReloadFont) {
    const std::string font_path1 = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    const std::string font_path2 = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    
    if (!font_exists(font_path1) || !font_exists(font_path2)) {
        GTEST_SKIP() << "Font files not available";
    }
    
    // Load first font
    EXPECT_NO_THROW(ft_text->load_font(font_path1));
    
    // Reload with second font
    EXPECT_NO_THROW(ft_text->load_font(font_path2));
    
    // Should be able to draw after reload
    ft_text->set_pixel_size(16);
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    EXPECT_NO_THROW(ft_text->draw_utf8(fb, 128, 64, 0, 0, "Test"));
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Various Cyrillic letters coverage
TEST_F(FtTextTest, DrawVariousCyrillicLetters) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    // Test various Russian letters: А, Б, В, Г, Д, Е, Ё, Ж, З, И, Й, К, Л, М, Н, О, П, Р, С, Т, У, Ф, Х, Ц, Ч, Ш, Щ, Ъ, Ы, Ь, Э, Ю, Я
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "АБВГДЕЁЖЗИЙКЛМН");
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
    
    // Test lowercase: а, б, в, г, д, е, ё, ж, з, и, й, к, л, м, н, о, п, р, с, т, у, ф, х, ц, ч, ш, щ, ъ, ы, ь, э, ю, я
    std::vector<unsigned char> fb2(128 * 64 / 8, 0);
    ft_text->draw_utf8(fb2, 128, 64, 0, 0, "абвгдеёжзийклмн");
    EXPECT_GT(count_nonzero_bytes(fb2), 0u);
}

// Test: UTF-8 boundary cases (1-byte, 2-byte, 3-byte characters)
TEST_F(FtTextTest, DrawUtf8BoundaryCharacters) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // Mix of 1-byte (A), 2-byte (©), 3-byte (€) characters
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "A©€");
    
    // Should render without crashing
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Long text that exceeds width
TEST_F(FtTextTest, DrawLongTextExceedingWidth) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // Very long text that will exceed display width
    ft_text->draw_utf8(fb, 128, 64, 0, 0, "This is a very long text that will exceed the display width");
    
    // Should handle gracefully and render what fits
    EXPECT_GT(count_nonzero_bytes(fb), 0u);
}

// Test: Draw at negative positions (should clip)
TEST_F(FtTextTest, DrawAtNegativePosition) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // Draw at negative position - should clip gracefully
    EXPECT_NO_THROW(ft_text->draw_utf8(fb, 128, 64, -10, -10, "Test"));
}

// Test: Draw at position beyond framebuffer
TEST_F(FtTextTest, DrawBeyondFramebuffer) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    if (!font_exists(font_path)) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ft_text->load_font(font_path);
    ft_text->set_pixel_size(16);
    
    std::vector<unsigned char> fb(128 * 64 / 8, 0);
    // Draw beyond framebuffer - should clip gracefully
    EXPECT_NO_THROW(ft_text->draw_utf8(fb, 128, 64, 200, 100, "Test"));
    
    // Framebuffer should remain all zeros (nothing visible)
    EXPECT_EQ(count_nonzero_bytes(fb), 0u);
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
