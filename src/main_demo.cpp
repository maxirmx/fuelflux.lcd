#include "four_line_display.h"
#include "spi_linux.h"
#include "gpio_gpiod.h"
#include "st7565.h"

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

    // The other suggested option is: "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf"
    std::string font = argval(argc, argv, "--font", "/usr/share/fonts/truetype/ubuntu/UbuntuMono-B.ttf");

    try {
        // Initialize hardware
        SpiLinux spi(dev);
        spi.open(8000000, 0);

        GpioLine dcLine(dc, true, false, chip, "nhd12864-dc");
        GpioLine rstLine(rst, true, true,  chip, "nhd12864-rst");

        St7565 lcd(spi, dcLine, rstLine);
        lcd.reset();
        lcd.init();

        // Initialize the Four Line Display library
        FourLineDisplay display(128, 64, 12, 28);
        
        if (!display.initialize(font)) {
            std::cerr << "Failed to initialize FourLineDisplay library\n";
            std::cerr << "  - Verify font exists: " << font << "\n";
            return 1;
        }

        std::cout << "Four Line Display Demo\n";
        std::cout << "======================\n";
        std::cout << "Line 0 (small): max " << display.length(0) << " chars\n";
        std::cout << "Line 1 (large): max " << display.length(1) << " chars\n";
        std::cout << "Line 2 (small): max " << display.length(2) << " chars\n";
        std::cout << "Line 3 (small): max " << display.length(3) << " chars\n";
        std::cout << "\nPress Ctrl+C to exit...\n\n";

        int counter = 0;
        while (true) {
            // Update display content
            display.puts(0, "Status: Running");
            display.puts(1, "Count: " + std::to_string(counter));
            display.puts(2, "FuelFlux NHD");
            display.puts(3, "Ver 2.0");

            // Render and send to LCD
            const auto& fb = display.render();
            lcd.set_framebuffer(fb);

            counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "Hints:\n";
        std::cerr << "  - Ensure SPI1 overlay is enabled and " << dev << " exists.\n";
        std::cerr << "  - Ensure font exists: " << font << "\n";
        std::cerr << "  - Verify GPIO line offsets (libgpiod) using gpio readall/gpioinfo.\n";
        return 1;
    }

    return 0;
}
