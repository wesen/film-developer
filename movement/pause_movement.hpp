#pragma once
#include "movement.hpp"

class PauseMovement final : public AgitationMovement {
public:
    explicit PauseMovement(uint32_t duration)
        : AgitationMovement(Type::Pause, duration) {
    }

    bool execute(MotorController& motor) override {
        DEBUG_PRINT("Executing PauseMovement | Elapsed: %u/%u", elapsed_time + 1, duration);

        if(elapsed_time >= duration) {
            return false;
        }

        motor.stop();
        elapsed_time++;

        if(elapsed_time >= duration) {
            DEBUG_PRINT("PauseMovement completed");
            return false;
        }
        return true;
    }

    bool isComplete() const override {
        return elapsed_time >= duration;
    }

    void reset() override {
        DEBUG_PRINT("Resetting PauseMovement");
        elapsed_time = 0;
    }

    void print() const override {
        DEBUG_PRINT(
            "PauseMovement | Duration: %u ticks | Elapsed: %u | Remaining: %u",
            duration,
            elapsed_time,
            duration > elapsed_time ? duration - elapsed_time : 0);
    }
};
