#pragma once
#include "../motor_controller.hpp"
#include <furi.h>
#include <furi_hal_gpio.h>

class MotorControllerEmbedded final : public MotorController {
public:
    MotorControllerEmbedded();
    ~MotorControllerEmbedded();

    void clockwise(bool enable) override;
    void counterClockwise(bool enable) override;
    void stop() override;
    bool isRunning() const override { return running; }

private:
    static constexpr uint32_t SAFETY_DELAY_US = 1000; // 1ms safety delay

    const GpioPin* pin_cw;
    const GpioPin* pin_ccw;
    bool running{false};

    void initGpio();
    void deinitGpio();
}; 