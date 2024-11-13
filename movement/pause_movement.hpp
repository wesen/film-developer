#pragma once
#include "movement.hpp"

class PauseMovement final : public AgitationMovement {
public:
    explicit PauseMovement(uint32_t duration)
        : AgitationMovement(Type::Pause, duration) {}

    bool execute(MotorController& motor) override {
        if (elapsed_time >= duration) {
            return false;
        }

        motor.stop();
        elapsed_time++;
        return true;
    }

    bool isComplete() const override {
        return elapsed_time >= duration;
    }

    void reset() override {
        elapsed_time = 0;
    }
}; 