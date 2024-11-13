#pragma once
#include "movement.hpp"

class MotorMovement final : public AgitationMovement {
public:
    explicit MotorMovement(Type type, uint32_t duration)
        : AgitationMovement(type, duration) {
        if (type != Type::CW && type != Type::CCW) {
            // In production code, we might want to handle this error differently
            type = Type::CW;
        }
    }

    bool execute(MotorController& motor) override {
        if (elapsed_time >= duration) {
            return false;
        }

        DEBUG_PRINT("Executing MotorMovement: %s | Elapsed: %u/%u", 
            type == Type::CW ? "CW" : "CCW",
            elapsed_time + 1,
            duration);

        if (type == Type::CW) {
            motor.clockwise(true);
        } else {
            motor.counterClockwise(true);
        }

        elapsed_time++;
        return true;
    }

    bool isComplete() const override {
        return elapsed_time >= duration;
    }

    void reset() override {
        elapsed_time = 0;
    }

    void print() const override {
        DEBUG_PRINT("MotorMovement: %s | Duration: %u ticks | Elapsed: %u | Remaining: %u", 
            type == Type::CW ? "CW" : "CCW",
            duration,
            elapsed_time,
            duration > elapsed_time ? duration - elapsed_time : 0);
    }
}; 