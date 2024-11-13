#include <string.h>
#include <stdio.h>
#include <memory>
#include "agitation_process_interpreter.hpp"
#include "motor_controller.hpp"

#include "debug.hpp"

void agitation_process_interpreter_init(
    AgitationProcessInterpreterState* state,
    const AgitationProcessStatic* process,
    MotorController*,
    void (*motor_cw)(bool),
    void (*motor_ccw)(bool)) {
    // Clear state
    memset(state, 0, sizeof(AgitationProcessInterpreterState));

    // Set process
    state->process = process;
    state->current_step_index = 0;
    state->process_state = AgitationProcessStateIdle;

    // Set motor callbacks
    state->motor_cw_callback = motor_cw;
    state->motor_ccw_callback = motor_ccw;

    // Set initial temperature tracking
    state->current_temperature = 20.0f;
    state->target_temperature = process->temperature;

    // Initialize user interaction state
    state->waiting_for_user = false;
    state->user_message = nullptr;

    DEBUG_PRINT("Process Interpreter Initialized:\n");
    DEBUG_PRINT("  Process Name: %s\n", process->process_name);
    DEBUG_PRINT("  Film Type: %s\n", process->film_type);
    DEBUG_PRINT("  Total Steps: %zu\n", process->steps_length);
    DEBUG_PRINT("  Initial Temperature: %.1f\n", static_cast<double>(state->target_temperature));
}

bool agitation_process_interpreter_tick(AgitationProcessInterpreterState* state) {
    // Check if process is complete
    if(state->current_step_index >= state->process->steps_length) {
        DEBUG_PRINT("Process Completed: All steps processed\n");
        state->process_state = AgitationProcessStateComplete;
        return false;
    }

    // Handle user interaction state
    if(state->waiting_for_user) {
        return true;
    }

    // Get current step
    const AgitationStepStatic* current_step = &state->process->steps[state->current_step_index];
    state->target_temperature = current_step->temperature;

    // Initialize or reinitialize movement interpreter if needed
    if(state->process_state == AgitationProcessStateIdle ||
       state->process_state == AgitationProcessStateComplete) {
        DEBUG_PRINT(
            "Initializing Movement Interpreter for Step %zu: %s\n",
            state->current_step_index,
            current_step->name ? current_step->name : "Unnamed Step");
        DEBUG_PRINT(
            "  Description: %s\n",
            current_step->description ? current_step->description : "No description");
        DEBUG_PRINT("  Step Temperature: %.1f\n", static_cast<double>(current_step->temperature));

        agitation_interpreter_init(
            &state->movement_interpreter,
            current_step->sequence,
            current_step->sequence_length,
            NULL,
            state->motor_cw_callback,
            state->motor_ccw_callback);
        state->process_state = AgitationProcessStateRunning;
    }

    // Process movement
    bool movement_active = agitation_interpreter_tick(&state->movement_interpreter);

    // Check for user interaction requirements
    if(movement_active &&
       state->movement_interpreter.current_index < state->movement_interpreter.sequence_length) {
        const AgitationMovement* current_movement =
            state->movement_interpreter.current_sequence[state->movement_interpreter.current_index];

        if(current_movement->getType() == AgitationMovement::Type::WaitUser) {
            state->waiting_for_user = true;
            // state->user_message = current_movement->getMessage();
            state->user_message = "STANDIN MESSAGE";
            return true;
        }
    }

    // Handle step completion
    if(!movement_active) {
        DEBUG_PRINT("Movement completed, advancing to next step\n");
        state->current_step_index++;
        state->process_state = AgitationProcessStateIdle;

        if(state->current_step_index >= state->process->steps_length) {
            state->process_state = AgitationProcessStateComplete;
        }
    }

    return movement_active || state->current_step_index < state->process->steps_length;
}

void agitation_process_interpreter_reset(AgitationProcessInterpreterState* state) {
    agitation_process_interpreter_init(
        state, state->process, NULL, state->motor_cw_callback, state->motor_ccw_callback);
}

void agitation_process_interpreter_confirm(AgitationProcessInterpreterState* state) {
    if(state->waiting_for_user) {
        state->waiting_for_user = false;
        state->user_message = nullptr;
        state->movement_interpreter.current_index++;
    }
}
