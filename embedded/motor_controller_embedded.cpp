#include "motor_controller_embedded.hpp"
#include <furi_hal_resources.h>

MotorControllerEmbedded::MotorControllerEmbedded() {}

MotorControllerEmbedded::~MotorControllerEmbedded() {}

void MotorControllerEmbedded::clockwise(bool enable) {
  if (enable) {
    furi_hal_gpio_write(pin_ccw, true); // Ensure CCW is off
    furi_delay_us(SAFETY_DELAY_US);
    furi_hal_gpio_write(pin_cw, false); // Active low
    cw_active = true;
    ccw_active = false;
  } else {
    furi_hal_gpio_write(pin_cw, true);
    cw_active = false;
  }
}

void MotorControllerEmbedded::counterClockwise(bool enable) {
  if (enable) {
    furi_hal_gpio_write(pin_cw, true); // Ensure CW is off
    furi_delay_us(SAFETY_DELAY_US);
    furi_hal_gpio_write(pin_ccw, false); // Active low
    ccw_active = true;
    cw_active = false;
  } else {
    furi_hal_gpio_write(pin_ccw, true);
    ccw_active = false;
  }
}

void MotorControllerEmbedded::stop() {
  furi_hal_gpio_write(pin_cw, true);
  furi_hal_gpio_write(pin_ccw, true);
  cw_active = false;
  ccw_active = false;
}

void MotorControllerEmbedded::initGpio() {
  // Initialize GPIO pins
  pin_cw = &gpio_ext_pa7;
  pin_ccw = &gpio_ext_pa6;

  furi_hal_gpio_init(pin_cw, GpioModeOutputPushPull, GpioPullNo,
                     GpioSpeedVeryHigh);
  furi_hal_gpio_init(pin_ccw, GpioModeOutputPushPull, GpioPullNo,
                     GpioSpeedVeryHigh);

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

const char *MotorControllerEmbedded::getDirectionString() const {
  if (cw_active)
    return "CW";
  if (ccw_active)
    return "CCW";
  return "Idle";
}