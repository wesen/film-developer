#pragma once

class MotorController {
public:
  virtual void clockwise(bool enable) = 0;
  virtual void counterClockwise(bool enable) = 0;
  virtual void stop() = 0;
  virtual bool isRunning() const = 0;
  virtual ~MotorController() = default;

  // Prevent copying for all derived classes
  MotorController(const MotorController &) = delete;
  MotorController &operator=(const MotorController &) = delete;

protected:
  MotorController() = default; // Only derived classes can construct
};