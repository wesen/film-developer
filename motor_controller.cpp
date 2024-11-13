#include "motor_controller.hpp"
#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>

MotorController::MotorController()
    : pin_cw(&gpio_ext_pa7)
    , pin_ccw(&gpio_ext_pa6) {
    initGpio();
}

MotorController::~MotorController() {
    deinitGpio();
}

void MotorController::initGpio() {
    furi_hal_gpio_init_simple(pin_cw, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(pin_ccw, GpioModeOutputPushPull);
    stop(); // Initialize both pins to inactive state
}

void MotorController::deinitGpio() {
    stop(); // Ensure motors are stopped
    furi_hal_gpio_init_simple(pin_cw, GpioModeAnalog);
    furi_hal_gpio_init_simple(pin_ccw, GpioModeAnalog);
}

void MotorController::stop() {
    // Set both pins high (inactive) for active low
    furi_hal_gpio_write(pin_cw, true);
    furi_hal_gpio_write(pin_ccw, true);
    running = false;
}

void MotorController::clockwise(bool enable) {
    if(enable) {
        // Safety check - ensure other motor is stopped first
        furi_hal_gpio_write(pin_ccw, true); // Disable CCW
        furi_delay_us(SAFETY_DELAY_US); // Safety delay
        furi_hal_gpio_write(pin_cw, false); // Enable CW (active low)
        running = true;
    } else {
        furi_hal_gpio_write(pin_cw, true); // Disable CW
        running = false;
    }
}

void MotorController::counterClockwise(bool enable) {
    if(enable) {
        // Safety check - ensure other motor is stopped first
        furi_hal_gpio_write(pin_cw, true); // Disable CW
        furi_delay_us(SAFETY_DELAY_US); // Safety delay
        furi_hal_gpio_write(pin_ccw, false); // Enable CCW (active low)
        running = true;
    } else {
        furi_hal_gpio_write(pin_ccw, true); // Disable CCW
        running = false;
    }
}
