#include "gpio_gpiod.h"
#include <gpiod.h>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <atomic>

struct GpioLine::Impl {
    gpiod_chip* chip{nullptr};
    gpiod_line* line{nullptr};
    bool is_output{false};
};

static void fail(const char* msg) { throw std::runtime_error(msg); }

GpioLine::GpioLine(int line_offset, bool output, bool initial_value, std::string chip_path, std::string consumer) {
    impl_ = new Impl();
    impl_->chip = gpiod_chip_open(chip_path.c_str());
    if (!impl_->chip) fail("Failed to open gpio chip");

    impl_->line = gpiod_chip_get_line(impl_->chip, line_offset);
    if (!impl_->line) fail("Failed to get gpio line");

    impl_->is_output = output;
    if (output) {
        if (gpiod_line_request_output(impl_->line, consumer.c_str(), initial_value ? 1 : 0) != 0)
            fail("Failed to request output line");
    } else {
        if (gpiod_line_request_input(impl_->line, consumer.c_str()) != 0)
            fail("Failed to request input line");
    }
}

GpioLine::~GpioLine() {
    if (!impl_) return;
    if (impl_->line) gpiod_line_release(impl_->line);
    if (impl_->chip) gpiod_chip_close(impl_->chip);
    delete impl_;
    impl_ = nullptr;
}

void GpioLine::set(bool value) {
    if (!impl_->is_output) fail("Line is not output");
    if (gpiod_line_set_value(impl_->line, value ? 1 : 0) != 0) fail("Failed to set value");
}

bool GpioLine::get() const {
    int v = gpiod_line_get_value(impl_->line);
    if (v < 0) fail("Failed to get value");
    return v != 0;
}

// Soft PWM helper
struct SoftPwmThread {
    std::thread t;
    std::atomic<bool> stop{false};
    std::atomic<int> duty{0};
    GpioLine* line{nullptr};
    int freq{1000};
};

static void pwm_loop(SoftPwmThread* th) {
    using namespace std::chrono;
    const double period = 1.0 / (th->freq > 0 ? th->freq : 500);
    while (!th->stop.load()) {
        int d = th->duty.load();
        double on = period * (double)d / 100.0;
        double off = period - on;
        if (on > 0) { th->line->set(true); std::this_thread::sleep_for(duration<double>(on)); }
        if (off > 0) { th->line->set(false); std::this_thread::sleep_for(duration<double>(off)); }
    }
    th->line->set(false);
}

SoftPwm::SoftPwm(GpioLine& line, int frequency_hz) : line_(line), freq_(frequency_hz) {}
SoftPwm::~SoftPwm() { stop(); }

void SoftPwm::clamp() {
    if (duty_ < 0) duty_ = 0;
    if (duty_ > 100) duty_ = 100;
}

void SoftPwm::start(int duty_percent) {
    if (running_) return;
    duty_ = duty_percent; clamp();
    auto* th = new SoftPwmThread();
    th->line = &line_;
    th->freq = freq_;
    th->duty.store(duty_);
    thread_ = th;
    running_ = true;
    th->t = std::thread(pwm_loop, th);
}

void SoftPwm::set_duty(int duty_percent) {
    duty_ = duty_percent; clamp();
    if (thread_) static_cast<SoftPwmThread*>(thread_)->duty.store(duty_);
}

void SoftPwm::stop() {
    if (!running_ || !thread_) return;
    auto* th = static_cast<SoftPwmThread*>(thread_);
    th->stop.store(true);
    if (th->t.joinable()) th->t.join();
    delete th;
    thread_ = nullptr;
    running_ = false;
}
