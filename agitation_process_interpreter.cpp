#include <string.h>
#include <stdio.h>
#include <memory>
#include "agitation_process_interpreter.hpp"
#include "motor_controller.hpp"
#include "debug.hpp"

AgitationProcessInterpreter::AgitationProcessInterpreter()
    : process(nullptr)
    , current_step_index(0)
    , process_state(AgitationProcessState::Idle)
    , current_temperature(20.0f)
    , target_temperature(20.0f)
    , motor_controller(nullptr)
    , waiting_for_user(false)
    , user_message(nullptr)
    , movement_loader(movement_factory)
    , sequence_length(0)
    , current_movement_index(0)
    , time_remaining(0) {
    memset(loaded_sequence, 0, sizeof(loaded_sequence));
}

void AgitationProcessInterpreter::init(
    const AgitationProcessStatic* process,
    MotorController* motor_controller) {
    this->process = process;
    this->motor_controller = motor_controller;
    current_step_index = 0;
    process_state = AgitationProcessState::Idle;

    current_temperature = 20.0f;
    target_temperature = process->temperature;

    waiting_for_user = false;
    user_message = nullptr;

    sequence_length = 0;
    current_movement_index = 0;

    DEBUG_PRINT("Process Interpreter Initialized:\n");
    DEBUG_PRINT("  Process Name: %s\n", process->process_name);
    DEBUG_PRINT("  Film Type: %s\n", process->film_type);
    DEBUG_PRINT("  Total Steps: %zu\n", process->steps_length);
    DEBUG_PRINT("  Initial Temperature: %.1f\n", static_cast<double>(target_temperature));
}

void AgitationProcessInterpreter::initializeMovementSequence(const AgitationStepStatic* step) {
    memset(loaded_sequence, 0, sizeof(loaded_sequence));

    sequence_length = movement_loader.loadSequence(
        step->sequence,
        step->sequence_length,
        loaded_sequence);

    current_movement_index = 0;

    if (sequence_length == 0) {
        DEBUG_PRINT("Failed to load movement sequence");
        process_state = AgitationProcessState::Error;
        return;
    }

    DEBUG_PRINT("Loaded movement sequence with %zu movements\n", sequence_length);
}

bool AgitationProcessInterpreter::tick() {
    if(current_step_index >= process->steps_length || process_state == AgitationProcessState::Error) {
        DEBUG_PRINT("Process Completed or Error: %s", 
                   process_state == AgitationProcessState::Error ? "Error" : "Completed");
        process_state = process_state == AgitationProcessState::Error ? 
                       AgitationProcessState::Error : 
                       AgitationProcessState::Complete;
        return false;
    }

    if(waiting_for_user) {
        return true;
    }

    const AgitationStepStatic* current_step = &process->steps[current_step_index];
    target_temperature = current_step->temperature;

    if(process_state == AgitationProcessState::Idle ||
       process_state == AgitationProcessState::Complete) {
        DEBUG_PRINT(
            "Initializing Movement Sequence for Step %zu: %s\n",
            current_step_index,
            current_step->name ? current_step->name : "Unnamed Step");

        initializeMovementSequence(current_step);
        process_state = AgitationProcessState::Running;
    }

    bool movement_active = false;
    if(current_movement_index < sequence_length) {
        AgitationMovement* current_movement = loaded_sequence[current_movement_index];

        if(current_movement) {
            if(current_movement->getType() == AgitationMovement::Type::WaitUser) {
                waiting_for_user = true;
                user_message = "STANDIN MESSAGE";
                return true;
            }

            movement_active = current_movement->execute(*motor_controller);

            if(!movement_active) {
                current_movement_index++;
            }
        }
    }

    if(!movement_active && current_movement_index >= sequence_length) {
        DEBUG_PRINT("Movement sequence completed, advancing to next step\n");
        current_step_index++;
        process_state = AgitationProcessState::Idle;

        if(current_step_index >= process->steps_length) {
            process_state = AgitationProcessState::Complete;
        }
    }

    return movement_active || current_step_index < process->steps_length;
}

void AgitationProcessInterpreter::reset() {
    init(process, motor_controller);
}

void AgitationProcessInterpreter::confirm() {
    if(waiting_for_user) {
        waiting_for_user = false;
        user_message = nullptr;
        current_movement_index++;
    }
}

void AgitationProcessInterpreter::skipToNextStep() {
    if(current_step_index < process->steps_length) {
        current_step_index++;
        process_state = AgitationProcessState::Idle;
        waiting_for_user = false;
        user_message = nullptr;
        sequence_length = 0;
        current_movement_index = 0;
    }
}
