#pragma once
#include "movement.hpp"
#include <cstddef>

class LoopMovement final : public AgitationMovement {
public:
    LoopMovement(AgitationMovement** sequence,
                 size_t sequence_length,
                 uint32_t iterations,
                 uint32_t max_duration)
        : AgitationMovement(Type::Loop, max_duration)
        , sequence(sequence)
        , sequence_length(sequence_length)
        , iterations(iterations)
        , current_iteration(0)
        , current_index(0) {}

    bool execute(MotorController& motor) override {
        if (isComplete()) {
            return false;
        }

        if (current_index >= sequence_length) {
            current_index = 0;
            current_iteration++;
            if (current_iteration >= iterations) {
                return false;
            }
        }

        bool result = sequence[current_index]->execute(motor);
        if (!result) {
            current_index++;
        }
        
        elapsed_time++;
        return true;
    }

    bool isComplete() const override {
        return (iterations > 0 && current_iteration >= iterations) ||
               (duration > 0 && elapsed_time >= duration);
    }

    void reset() override {
        elapsed_time = 0;
        current_iteration = 0;
        current_index = 0;
        for (size_t i = 0; i < sequence_length; i++) {
            sequence[i]->reset();
        }
    }

private:
    AgitationMovement** sequence;
    size_t sequence_length;
    uint32_t iterations;
    uint32_t current_iteration;
    size_t current_index;
}; 