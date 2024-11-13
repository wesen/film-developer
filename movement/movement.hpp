#pragma once
#include "../debug.hpp"
#include "../motor_controller.hpp"
#include <cstdint>

class AgitationMovement {
public:
  enum class Type { CW, CCW, Pause, Loop, WaitUser };

  explicit AgitationMovement(Type type, uint32_t duration = 0)
      : type(type), duration(duration) {}

  virtual ~AgitationMovement() = default;

  virtual bool execute(MotorController &motor) = 0;
  virtual bool isComplete() const = 0;
  virtual void reset() = 0;
  virtual void print() const = 0;

  Type getType() const { return type; }
  virtual uint32_t getDuration() const { return duration; }

  virtual uint32_t timeElapsed() const { return elapsed_time; }
  virtual uint32_t timeRemaining() const {
    return duration > elapsed_time ? duration - elapsed_time : 0;
  }

protected:
  Type type;
  uint32_t duration;
  uint32_t elapsed_time{0};
};