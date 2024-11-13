#pragma once
#include "movement.hpp"
#include "motor_movement.hpp"
#include "pause_movement.hpp"
#include "loop_movement.hpp"
#include "wait_user_movement.hpp"
#include <array>

class MovementFactory {
public:
    static constexpr size_t MAX_MOVEMENTS = 64;

    static AgitationMovement* createCW(uint32_t duration) {
        return new(allocateMovement(sizeof(MotorMovement)))
            MotorMovement(AgitationMovement::Type::CW, duration);
    }

    static AgitationMovement* createCCW(uint32_t duration) {
        return new(allocateMovement(sizeof(MotorMovement)))
            MotorMovement(AgitationMovement::Type::CCW, duration);
    }

    static AgitationMovement* createPause(uint32_t duration) {
        return new(allocateMovement(sizeof(PauseMovement))) PauseMovement(duration);
    }

    static AgitationMovement* createLoop(
        AgitationMovement** sequence,
        size_t sequence_length,
        uint32_t iterations,
        uint32_t max_duration) {
        return new(allocateMovement(sizeof(LoopMovement)))
            LoopMovement(sequence, sequence_length, iterations, max_duration);
    }

    static AgitationMovement* createWaitUser() {
        return new(allocateMovement(sizeof(WaitUserMovement))) WaitUserMovement();
    }

    static void reset() {
        current_pool_index = 0;
    }

private:
    static void* allocateMovement(size_t size) {
        if(current_pool_index + size > movement_pool.size()) {
            return nullptr;
        }
        void* ptr = &movement_pool[current_pool_index];
        current_pool_index += size;
        return ptr;
    }

    static inline std::array<uint8_t, MAX_MOVEMENTS * sizeof(AgitationMovement)> movement_pool;
    static inline size_t current_pool_index = 0;
};
