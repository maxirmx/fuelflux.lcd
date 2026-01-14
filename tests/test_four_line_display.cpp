#include <gtest/gtest.h>
#include "four_line_display.h"
#include <string>
#include <vector>
#include <fstream>

// Test fixture for FourLineDisplay tests
class FourLineDisplayTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create display with default parameters
        display = std::make_unique<FourLineDisplay>(128, 64, 12, 28);
    }

    void TearDown() override {
        if (display && display->is_initialized()) {
            display->uninitialize();
        }
        display.reset();
    }

    std::unique_ptr<FourLineDisplay> display;
};

// Test: Constructor creates object with correct dimensions
TEST_F(FourLineDisplayTest, ConstructorSetsCorrectDimensions) {
    EXPECT_EQ(display->get_width(), 128);
    EXPECT_EQ(display->get_height(), 64);
    EXPECT_EQ(display->get_small_font_size(), 12);
    EXPECT_EQ(display->get_large_font_size(), 28);
}

// Test: Initial state is not initialized
TEST_F(FourLineDisplayTest, InitialStateNotInitialized) {
    EXPECT_FALSE(display->is_initialized());
}

// Test: Initialize with invalid font path fails
TEST_F(FourLineDisplayTest, InitializeWithInvalidFontFails) {
    bool result = display->initialize("/nonexistent/path/to/font.ttf");
    EXPECT_FALSE(result);
    EXPECT_FALSE(display->is_initialized());
}

// Test: Initialize succeeds with valid font (if available)
// Note: This test will be skipped if the font is not available
TEST_F(FourLineDisplayTest, InitializeWithValidFontSucceeds) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    // Skip test if font doesn't exist
    std::ifstream font_file(font_path);
    if (!font_file.good()) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    bool result = display->initialize(font_path);
    EXPECT_TRUE(result);
    EXPECT_TRUE(display->is_initialized());
}

// Test: Uninitialize works correctly
TEST_F(FourLineDisplayTest, UninitializeWorks) {
    display->uninitialize();
    EXPECT_FALSE(display->is_initialized());
}

// Test: Length calculation for valid line IDs
TEST_F(FourLineDisplayTest, LengthReturnsValidValues) {
    // Line 0, 2, 3: small font (12px) -> ~17 chars for 128px width
    // Line 1: large font (28px) -> ~7 chars for 128px width
    
    unsigned int len0 = display->length(0);
    unsigned int len1 = display->length(1);
    unsigned int len2 = display->length(2);
    unsigned int len3 = display->length(3);
    
    // Verify small font lines have more capacity than large font line
    EXPECT_GT(len0, len1);
    EXPECT_GT(len2, len1);
    EXPECT_GT(len3, len1);
    
    // Verify all small font lines have same capacity
    EXPECT_EQ(len0, len2);
    EXPECT_EQ(len0, len3);
    
    // Verify lengths are reasonable (not zero, not too large)
    EXPECT_GT(len0, 0u);
    EXPECT_GT(len1, 0u);
    EXPECT_LT(len0, 128u);
    EXPECT_LT(len1, 128u);
}

// Test: Length returns 0 for invalid line IDs
TEST_F(FourLineDisplayTest, LengthReturnsZeroForInvalidLineId) {
    EXPECT_EQ(display->length(4), 0u);
    EXPECT_EQ(display->length(5), 0u);
    EXPECT_EQ(display->length(100), 0u);
}

// Test: puts() sets text correctly
TEST_F(FourLineDisplayTest, PutsSetsTextCorrectly) {
    display->puts(0, "Line 0");
    display->puts(1, "Line 1");
    display->puts(2, "Line 2");
    display->puts(3, "Line 3");
    
    EXPECT_EQ(display->get_text(0), "Line 0");
    EXPECT_EQ(display->get_text(1), "Line 1");
    EXPECT_EQ(display->get_text(2), "Line 2");
    EXPECT_EQ(display->get_text(3), "Line 3");
}

// Test: puts() with invalid line ID does nothing
TEST_F(FourLineDisplayTest, PutsWithInvalidLineIdDoesNothing) {
    display->puts(4, "Invalid");
    EXPECT_EQ(display->get_text(4), "");
}

// Test: get_text() returns empty for invalid line ID
TEST_F(FourLineDisplayTest, GetTextReturnsEmptyForInvalidLineId) {
    EXPECT_EQ(display->get_text(4), "");
    EXPECT_EQ(display->get_text(100), "");
}

// Test: puts() overwrites previous text
TEST_F(FourLineDisplayTest, PutsOverwritesPreviousText) {
    display->puts(0, "First");
    EXPECT_EQ(display->get_text(0), "First");
    
    display->puts(0, "Second");
    EXPECT_EQ(display->get_text(0), "Second");
}

// Test: puts() handles UTF-8 text
TEST_F(FourLineDisplayTest, PutsHandlesUtf8Text) {
    std::string russian = "Привет";
    std::string emoji = "Hello 😀";
    
    display->puts(0, russian);
    display->puts(1, emoji);
    
    EXPECT_EQ(display->get_text(0), russian);
    EXPECT_EQ(display->get_text(1), emoji);
}

// Test: clear_line() clears specific line
TEST_F(FourLineDisplayTest, ClearLineWorks) {
    display->puts(0, "Line 0");
    display->puts(1, "Line 1");
    display->puts(2, "Line 2");
    
    display->clear_line(1);
    
    EXPECT_EQ(display->get_text(0), "Line 0");
    EXPECT_EQ(display->get_text(1), "");
    EXPECT_EQ(display->get_text(2), "Line 2");
}

// Test: clear_line() with invalid ID does nothing
TEST_F(FourLineDisplayTest, ClearLineWithInvalidIdDoesNothing) {
    display->puts(0, "Line 0");
    display->clear_line(4);
    EXPECT_EQ(display->get_text(0), "Line 0");
}

// Test: clear_all() clears all lines
TEST_F(FourLineDisplayTest, ClearAllWorks) {
    display->puts(0, "Line 0");
    display->puts(1, "Line 1");
    display->puts(2, "Line 2");
    display->puts(3, "Line 3");
    
    display->clear_all();
    
    EXPECT_EQ(display->get_text(0), "");
    EXPECT_EQ(display->get_text(1), "");
    EXPECT_EQ(display->get_text(2), "");
    EXPECT_EQ(display->get_text(3), "");
}

// Test: get_framebuffer() returns correct size
TEST_F(FourLineDisplayTest, GetFramebufferReturnsCorrectSize) {
    const auto& fb = display->get_framebuffer();
    // For 128x64 display, framebuffer should be 128 * 64 / 8 = 1024 bytes
    EXPECT_EQ(fb.size(), 1024u);
}

// Test: render() without initialization returns framebuffer
TEST_F(FourLineDisplayTest, RenderWithoutInitializationWorks) {
    const auto& fb = display->render();
    EXPECT_EQ(fb.size(), 1024u);
}

// Test: render() with initialization and font creates framebuffer
TEST_F(FourLineDisplayTest, RenderWithInitializationWorks) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    // Skip test if font doesn't exist
    std::ifstream font_file(font_path);
    if (!font_file.good()) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    ASSERT_TRUE(display->initialize(font_path));
    
    display->puts(0, "Test");
    const auto& fb = display->render();
    
    EXPECT_EQ(fb.size(), 1024u);
    
    // Framebuffer should have some non-zero bytes (rendered text)
    bool has_nonzero = false;
    for (auto byte : fb) {
        if (byte != 0) {
            has_nonzero = true;
            break;
        }
    }
    EXPECT_TRUE(has_nonzero);
}

// Test: Custom dimensions work correctly
TEST(FourLineDisplayCustomTest, CustomDimensionsWork) {
    FourLineDisplay custom_display(64, 32, 8, 16);
    
    EXPECT_EQ(custom_display.get_width(), 64);
    EXPECT_EQ(custom_display.get_height(), 32);
    EXPECT_EQ(custom_display.get_small_font_size(), 8);
    EXPECT_EQ(custom_display.get_large_font_size(), 16);
    
    // Framebuffer size should be 64 * 32 / 8 = 256 bytes
    EXPECT_EQ(custom_display.get_framebuffer().size(), 256u);
}

// Test: Multiple initialize/uninitialize cycles work
TEST_F(FourLineDisplayTest, MultipleInitUninitCyclesWork) {
    const std::string font_path = "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf";
    
    // Skip test if font doesn't exist
    std::ifstream font_file(font_path);
    if (!font_file.good()) {
        GTEST_SKIP() << "Font file not available: " << font_path;
    }
    
    // First cycle
    EXPECT_TRUE(display->initialize(font_path));
    EXPECT_TRUE(display->is_initialized());
    display->uninitialize();
    EXPECT_FALSE(display->is_initialized());
    
    // Second cycle
    EXPECT_TRUE(display->initialize(font_path));
    EXPECT_TRUE(display->is_initialized());
    display->uninitialize();
    EXPECT_FALSE(display->is_initialized());
}

// Test: Lines are independent
TEST_F(FourLineDisplayTest, LinesAreIndependent) {
    display->puts(0, "A");
    display->puts(1, "B");
    display->puts(2, "C");
    display->puts(3, "D");
    
    display->clear_line(1);
    
    EXPECT_EQ(display->get_text(0), "A");
    EXPECT_EQ(display->get_text(1), "");
    EXPECT_EQ(display->get_text(2), "C");
    EXPECT_EQ(display->get_text(3), "D");
    
    display->puts(1, "E");
    
    EXPECT_EQ(display->get_text(0), "A");
    EXPECT_EQ(display->get_text(1), "E");
    EXPECT_EQ(display->get_text(2), "C");
    EXPECT_EQ(display->get_text(3), "D");
}

// Main function for running tests
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
