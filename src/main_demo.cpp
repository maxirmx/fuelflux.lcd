#include "spi_linux.h"
#include "gpio_gpiod.h"
#include "st7565.h"
#include "graphics.h"

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
    std::string dev  = argval(argc, argv, "--spidev", "/dev/spidev0.0");
    std::string chip = argval(argc, argv, "--chip", "/dev/gpiochip0");
    int dc  = argint(argc, argv, "--dc", 25);
    int rst = argint(argc, argv, "--rst", 17);

    try {
        SpiLinux spi(dev);
        spi.open(8000000, 0);

        GpioLine dcLine(dc, true, false, chip, "nhd12864-dc");
        GpioLine rstLine(rst, true, true,  chip, "nhd12864-rst");

        St7565 lcd(spi, dcLine, rstLine);
        lcd.reset();
        lcd.init();

        MonoGfx g(128, 64);
        int x = 0;

        while (true) {
            g.clear();
            g.text(2, 2, "NHD 128x64 (C++)", true);
            g.hline(0, 127, 14, true);
            g.rect(5, 20, 122, 58, true);
            g.fill_rect(8, 24, 8 + (x % 110), 54, true);

            lcd.set_framebuffer(g.fb());
            x += 3;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\\n";
        std::cerr << "Verify /dev/spidev* exists and map GPIO via gpioinfo.\\n";
        return 1;
    }
}
