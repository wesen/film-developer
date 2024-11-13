#pragma once

#include <furi_hal_gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

void motor_stop();
void motor_cw_callback(bool enable);
void motor_ccw_callback(bool enable);

void gpio_init();
void gpio_deinit();

extern const GpioPin* const pin_cw;
extern const GpioPin* const pin_ccw;

#ifdef __cplusplus
}
#endif
