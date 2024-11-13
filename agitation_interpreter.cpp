#include "agitation_interpreter.hpp"
#include "movement/movement_factory.hpp"
#include "movement/movement_loader.hpp"
#include <string.h>

void agitation_interpreter_init(
    AgitationInterpreterState* state,
    const struct AgitationMovementStatic* sequence,
    size_t sequence_length,
    MotorController* motor_controller,
    void (*motor_cw)(bool),
    void (*motor_ccw)(bool)) {
    // Reset state
    memset(state, 0, sizeof(AgitationInterpreterState));

    // Create factory and loader
    MovementFactory factory;
    MovementLoader loader(factory);

    // Create array to store the sequence
    static AgitationMovement* loaded_sequence[MovementLoader::MAX_SEQUENCE_LENGTH];
    
    // Load sequence using movement loader
    size_t loaded_length = loader.loadSequence(sequence, sequence_length, loaded_sequence);

    // Set initial sequence
    state->current_sequence = loaded_sequence;
    state->sequence_length = loaded_length;
    state->current_index = 0;

    // Store motor controller reference
    state->motor_controller = motor_controller;

    // Set legacy callbacks
    state->motor_cw_callback = motor_cw;
    state->motor_ccw_callback = motor_ccw;
}

bool agitation_interpreter_tick(AgitationInterpreterState* state) {
    if(state->current_index >= state->sequence_length) {
        return false;
    }

    // Execute current movement
    AgitationMovement* current_movement = state->current_sequence[state->current_index];
    if(current_movement) {
        bool movement_active = current_movement->execute(*state->motor_controller);

        // Update legacy state tracking
        state->current_movement = current_movement->getType();
        state->time_remaining =
            current_movement->getDuration() - (current_movement->isComplete() ? 0 : 1);

        if(!movement_active) {
            state->current_index++;
        }

        return true;
    }

    return false;
}

void agitation_interpreter_reset(AgitationInterpreterState* state) {
    const struct AgitationMovementStatic* original_sequence = nullptr;
    size_t original_length = 0;
    void (*cw_callback)(bool) = state->motor_cw_callback;
    void (*ccw_callback)(bool) = state->motor_ccw_callback;
    MotorController* motor = state->motor_controller;

    // Reinitialize
    agitation_interpreter_init(
        state, original_sequence, original_length, motor, cw_callback, ccw_callback);
}
