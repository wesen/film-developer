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
  bool isRunning() const override { return cw_active || ccw_active; }
  bool isClockwise() const override { return cw_active; }
  bool isCounterClockwise() const override { return ccw_active; }
  bool isStopped() const override { return !isRunning(); }
  const char *getDirectionString() const override;

  void initGpio();
  void deinitGpio();

private:
  static constexpr uint32_t SAFETY_DELAY_US = 1000; // 1ms safety delay

  const GpioPin *pin_cw;
  const GpioPin *pin_ccw;
  bool cw_active{false};
  bool ccw_active{false};
};
