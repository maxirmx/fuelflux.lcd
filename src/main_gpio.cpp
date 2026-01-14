#include "gpio_gpiod.h"
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
    std::string chip = argval(argc, argv, "--chip", "/dev/gpiochip0");
    int bl = argint(argc, argv, "--bl-pin", 18);
    int heater = argint(argc, argv, "--heater-pin", 23);
    int duty = argint(argc, argv, "--bl-duty", 40);
    int heater_on = argint(argc, argv, "--heater-on", 0);
    int freq = argint(argc, argv, "--bl-freq", 1000);

    try {
        GpioLine blLine(bl, true, false, chip, "nhd12864-bl");
        GpioLine heaterLine(heater, true, false, chip, "nhd12864-heater");

        SoftPwm pwm(blLine, freq);
        pwm.start(duty);
        heaterLine.set(heater_on != 0);

        std::cout << "Backlight duty=" << duty << "% @ " << freq
                  << "Hz; heater=" << heater_on << "\\n";
        std::cout << "Ctrl+C to exit.\\n";
        while (true) std::this_thread::sleep_for(std::chrono::seconds(1));
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\\n";
        return 1;
    }
}
