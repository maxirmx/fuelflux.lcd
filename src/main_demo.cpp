#include "spi_linux.h"
#include "gpio_gpiod.h"
#include "st7565.h"
#include "graphics.h"
#include "ft_text.h"

#include <iostream>
#include <thread>
#include <chrono>
#include <string>

static const char* argval(int argc, char** argv, const char* key, const char* defv) {
    for (int i=1;i<argc;i++) {
        if (std::string(argv[i]) == key && i+1<argc) return argv[i+1];
    }
    return defv;
}
static int argint(int argc, char** argv, const char* key, int defv) {
    const char* v = argval(argc, argv, key, nullptr);
    return v ? std::stoi(v) : defv;
}

int main(int argc, char** argv) {
    std::string dev  = argval(argc, argv, "--spidev", "/dev/spidev1.0");
    std::string chip = argval(argc, argv, "--chip", "/dev/gpiochip0");
    int dc  = argint(argc, argv, "--dc", 262);
    int rst = argint(argc, argv, "--rst", 226);

    std::string font = argval(argc, argv, "--font", "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf");
    int px = argint(argc, argv, "--px", 16);

    try {
        SpiLinux spi(dev);
        spi.open(8000000, 0);

        GpioLine dcLine(dc, true, false, chip, "nhd12864-dc");
        GpioLine rstLine(rst, true, true,  chip, "nhd12864-rst");

        St7565 lcd(spi, dcLine, rstLine);
        lcd.reset();
        lcd.init();

        MonoGfx g(128, 64);
        FtText ft;
        ft.load_font(font);
        ft.set_pixel_size(px);

        int t = 0;
        while (true) {
            g.clear();

            // 8x16-like layout: 4 lines visible at px=16 (64px height)
            // Demonstrate full English + Russian (incl. Ё/ё).
            std::string l1 = "Hello, World!";
            std::string l2 = "Привет, мир!";
            std::string l3 = "Ёжик, съёмка";
            std::string l4 = "t=" + std::to_string(t++);

            ft.draw_utf8(g.fb(), 128, 64, 0, 0,  l1, true);
            ft.draw_utf8(g.fb(), 128, 64, 0, 16, l2, true);
            ft.draw_utf8(g.fb(), 128, 64, 0, 32, l3, true);
            ft.draw_utf8(g.fb(), 128, 64, 0, 48, l4, true);

            lcd.set_framebuffer(g.fb());
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "Hints:\n";
        std::cerr << "  - Ensure SPI1 overlay is enabled and " << dev << " exists.\n";
        std::cerr << "  - Ensure font exists: " << font << "\n";
        std::cerr << "  - Verify GPIO line offsets (libgpiod) using gpio readall/gpioinfo.\n";
        return 1;
    }
}
