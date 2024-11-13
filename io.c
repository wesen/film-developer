#include <furi.h>
#include <furi_hal_gpio.h>
#include <furi_hal_resources.h>

#include "io.h"

// Define our GPIO pins (active low)
const GpioPin* const pin_cw = &gpio_ext_pa7;
const GpioPin* const pin_ccw = &gpio_ext_pa6;

void gpio_init() {
    furi_hal_gpio_init_simple(pin_cw, GpioModeOutputPushPull);
    furi_hal_gpio_init_simple(pin_ccw, GpioModeOutputPushPull);
    motor_stop(); // Initialize both pins to inactive state
}

void gpio_deinit() {
    motor_stop(); // Ensure motors are stopped
    furi_hal_gpio_init_simple(pin_cw, GpioModeAnalog);
    furi_hal_gpio_init_simple(pin_ccw, GpioModeAnalog);
}

void motor_stop() {
    // Set both pins high (inactive) for active low
    furi_hal_gpio_write(pin_cw, true);
    furi_hal_gpio_write(pin_ccw, true);
}

void motor_cw_callback(bool enable) {
    if(enable) {
        // Safety check - ensure other motor is stopped first
        furi_hal_gpio_write(pin_ccw, true); // Disable CCW
        furi_delay_us(1000); // 1ms delay for safety
        furi_hal_gpio_write(pin_cw, false); // Enable CW (active low)
    } else {
        furi_hal_gpio_write(pin_cw, true); // Disable CW
    }
}

void motor_ccw_callback(bool enable) {
    if(enable) {
        // Safety check - ensure other motor is stopped first
        furi_hal_gpio_write(pin_cw, true); // Disable CW
        furi_delay_us(1000); // 1ms delay for safety
        furi_hal_gpio_write(pin_ccw, false); // Enable CCW (active low)
    } else {
        furi_hal_gpio_write(pin_ccw, true); // Disable CCW
    }
}
