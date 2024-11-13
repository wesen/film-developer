#pragma once

#include "movement/movement.hpp"
#include "movement/movement_loader.hpp"
#include <stddef.h>
#include <stdint.h>
#include "agitation_sequence.hpp"
#include "motor_controller.hpp"

#define MAX_LOOP_DEPTH 3
#define LOOP_CONTINUOUS 0

typedef struct LoopState {
    AgitationMovement** sequence;
    size_t sequence_length;
    uint32_t remaining_iterations;
    size_t start_index;
    uint32_t original_count;
    uint32_t elapsed_duration;
    uint32_t max_duration;
    bool stop;
} LoopState;

typedef struct AgitationInterpreterState {
    // Current execution context
    AgitationMovement** current_sequence;
    size_t sequence_length;
    size_t current_index;

    // Loop tracking
    LoopState loop_stack[MAX_LOOP_DEPTH];
    uint8_t loop_depth;

    // Current movement tracking
    AgitationMovement::Type current_movement;
    uint32_t time_remaining;

    // Motor controller instance
    MotorController* motor_controller;

    // Legacy callback support for backwards compatibility
    void (*motor_cw_callback)(bool);
    void (*motor_ccw_callback)(bool);
} AgitationInterpreterState;

// Initialize the interpreter state
void agitation_interpreter_init(
    AgitationInterpreterState* state,
    const struct AgitationMovementStatic* sequence,
    size_t sequence_length,
    MotorController* motor_controller,
    void (*motor_cw)(bool),
    void (*motor_ccw)(bool));

// Step the interpreter forward by 1 second
bool agitation_interpreter_tick(AgitationInterpreterState* state);

// Reset the interpreter to its initial state
void agitation_interpreter_reset(AgitationInterpreterState* state);
