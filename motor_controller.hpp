#pragma once
#include <furi_hal_gpio.h>

class MotorController {
public:
    MotorController();
    ~MotorController();

    // Prevent copying
    MotorController(const MotorController&) = delete;
    MotorController& operator=(const MotorController&) = delete;

    void clockwise(bool enable);
    void counterClockwise(bool enable);
    void stop();
    bool isRunning() const { return running; }

private:
    static constexpr uint32_t SAFETY_DELAY_US = 1000; // 1ms safety delay

    const GpioPin* pin_cw;
    const GpioPin* pin_ccw;
    bool running{false};

    void initGpio();
    void deinitGpio();
}; 