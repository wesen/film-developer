#pragma once
#include "movement.hpp"

class WaitUserMovement final : public AgitationMovement {
public:
    WaitUserMovement() : AgitationMovement(Type::WaitUser) {}

    bool execute(MotorController& motor) override {
        DEBUG_PRINT("Executing WaitUserMovement | State: %s | Elapsed: %u", 
            user_acknowledged ? "acknowledged" : "waiting",
            elapsed_time + 1);

        motor.stop();
        return !user_acknowledged;
    }

    bool isComplete() const override {
        return user_acknowledged;
    }

    void reset() override {
        user_acknowledged = false;
    }

    void acknowledgeUser() {
        user_acknowledged = true;
    }

    void print() const override {
        DEBUG_PRINT("WaitUserMovement | State: %s | Elapsed: %u", 
            user_acknowledged ? "acknowledged" : "waiting",
            elapsed_time);
    }

private:
    bool user_acknowledged{false};
}; 