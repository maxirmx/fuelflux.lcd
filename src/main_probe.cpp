#include "gpio_gpiod.h"
#include <iostream>
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
    std::string chip = argval(argc, argv, "--chip", "/dev/gpiochip0");
    int line = argint(argc, argv, "--line", 25);

    try {
        GpioLine l(line, true, false, chip, "nhd12864-probe");
        std::cout << "Requested OUTPUT OK: " << chip << " line " << line << "\\n";
        l.set(true);
        std::cout << "Set HIGH OK\\n";
        l.set(false);
        std::cout << "Set LOW OK\\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Probe failed: " << e.what() << "\\n";
        return 2;
    }
}
