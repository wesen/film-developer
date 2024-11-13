#include <furi_hal_resources.h>
#include "motor_controller_embedded.hpp"

MotorControllerEmbedded::MotorControllerEmbedded() {
    initGpio();
}

MotorControllerEmbedded::~MotorControllerEmbedded() {
    deinitGpio();
}

void MotorControllerEmbedded::clockwise(bool enable) {
    if (enable) {
        furi_hal_gpio_write(pin_ccw, true);  // Ensure CCW is off
        furi_delay_us(SAFETY_DELAY_US);
        furi_hal_gpio_write(pin_cw, false);  // Active low
        running = true;
    } else {
        furi_hal_gpio_write(pin_cw, true);
        running = false;
    }
}

void MotorControllerEmbedded::counterClockwise(bool enable) {
    if (enable) {
        furi_hal_gpio_write(pin_cw, true);   // Ensure CW is off
        furi_delay_us(SAFETY_DELAY_US);
        furi_hal_gpio_write(pin_ccw, false); // Active low
        running = true;
    } else {
        furi_hal_gpio_write(pin_ccw, true);
        running = false;
    }
}

void MotorControllerEmbedded::stop() {
    furi_hal_gpio_write(pin_cw, true);
    furi_hal_gpio_write(pin_ccw, true);
    running = false;
}

void MotorControllerEmbedded::initGpio() {
    // Initialize GPIO pins
    pin_cw = &gpio_ext_pc0;
    pin_ccw = &gpio_ext_pc1;

    furi_hal_gpio_init(pin_cw, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
    furi_hal_gpio_init(pin_ccw, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);

    // Set both pins high (motor off) initially
    furi_hal_gpio_write(pin_cw, true);
    furi_hal_gpio_write(pin_ccw, true);
}

void MotorControllerEmbedded::deinitGpio() {
    stop();
    // Reset GPIO pins to default state
    furi_hal_gpio_init(pin_cw, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
    furi_hal_gpio_init(pin_ccw, GpioModeAnalog, GpioPullNo, GpioSpeedLow);
} 