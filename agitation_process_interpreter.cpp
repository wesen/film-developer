#include "agitation_process_interpreter.hpp"
#include "debug.hpp"
#include "motor_controller.hpp"
#include <memory>
#include <stdio.h>
#include <string.h>

AgitationProcessInterpreter::AgitationProcessInterpreter()
    : process(nullptr), current_step_index(0),
      process_state(AgitationProcessState::Idle), current_temperature(20.0f),
      target_temperature(20.0f), motor_controller(nullptr),
      movement_loader(movement_factory), sequence_length(0),
      current_movement_index(0), time_remaining(0) {
  memset(loaded_sequence, 0, sizeof(loaded_sequence));
}

void AgitationProcessInterpreter::init(const AgitationProcessStatic *process,
                                       MotorController *motor_controller) {
  this->process = process;
  this->motor_controller = motor_controller;
  current_step_index = 0;
  process_state = AgitationProcessState::Idle;

  current_temperature = 20.0f;
  target_temperature = process->temperature;

  sequence_length = 0;
  current_movement_index = 0;

  DEBUG_PRINT("Process Interpreter Initialized:\n");
  DEBUG_PRINT("  Process Name: %s\n", process->process_name);
  DEBUG_PRINT("  Film Type: %s\n", process->film_type);
  DEBUG_PRINT("  Total Steps: %zu\n", process->steps_length);
  DEBUG_PRINT("  Initial Temperature: %.1f\n",
              static_cast<double>(target_temperature));
}

void AgitationProcessInterpreter::initializeMovementSequence(
    const AgitationStepStatic *step) {
  memset(loaded_sequence, 0, sizeof(loaded_sequence));

  sequence_length = movement_loader.loadSequence(
      step->sequence, step->sequence_length, loaded_sequence);

  current_movement_index = 0;

  if (sequence_length == 0) {
    DEBUG_PRINT("Failed to load movement sequence");
    process_state = AgitationProcessState::Error;
    return;
  }

  for (size_t i = 0; i < sequence_length; i++) {
    if (loaded_sequence[i]) {
      loaded_sequence[i]->reset();
    }
  }

  DEBUG_PRINT("Loaded movement sequence with %zu movements\n", sequence_length);
}

bool AgitationProcessInterpreter::tick() {
  if (current_step_index >= process->steps_length ||
      process_state == AgitationProcessState::Error) {
    DEBUG_PRINT("Process Completed or Error: %s",
                process_state == AgitationProcessState::Error ? "Error"
                                                              : "Completed");
    process_state = process_state == AgitationProcessState::Error
                        ? AgitationProcessState::Error
                        : AgitationProcessState::Complete;
    return false;
  }

  const AgitationStepStatic *current_step = &process->steps[current_step_index];
  target_temperature = current_step->temperature;

  if (process_state == AgitationProcessState::Idle ||
      process_state == AgitationProcessState::Complete) {
    DEBUG_PRINT("Initializing Movement Sequence for Step %zu: %s\n",
                current_step_index,
                current_step->name ? current_step->name : "Unnamed Step");

    initializeMovementSequence(current_step);
    process_state = AgitationProcessState::Running;
  }

  bool movement_active = false;
  if (current_movement_index < sequence_length) {
    AgitationMovement *current_movement =
        loaded_sequence[current_movement_index];

    if (current_movement) {
      if (current_movement->getType() == AgitationMovement::Type::WaitUser) {
        return true;
      }

      movement_active = current_movement->execute(*motor_controller);

      if (!movement_active) {
        advanceToNextMovement();
      }
    }
  }

  if (!movement_active && current_movement_index >= sequence_length) {
    DEBUG_PRINT("Movement sequence completed, advancing to next step\n");
    advanceToNextStep();
  }

  return movement_active || current_step_index < process->steps_length;
}

void AgitationProcessInterpreter::reset() { init(process, motor_controller); }

void AgitationProcessInterpreter::confirm() {
  if (isWaitingForUser()) {
    if (current_step_index + 1 >= process->steps_length) {
      // If this is the last step, just advance the movement
      advanceToNextMovement();
    } else {
      // If there's a next step, advance to it
      advanceToNextStep();
    }
  }
}

void AgitationProcessInterpreter::advanceToNextStep() {
  if (!(current_step_index + 1 < process->steps_length)) {
    DEBUG_PRINT("Cannot advance to next step, already at last step");
    return;
  }
  DEBUG_PRINT("Advancing to next step: %s, current step index: %lu/%lu  ",
              process->steps[current_step_index + 1].name,
              current_step_index + 1, process->steps_length);
  current_step_index++;
  process_state = AgitationProcessState::Idle;
  sequence_length = 0;
  current_movement_index = 0;

  // Reset all movements in current sequence
  for (size_t i = 0; i < sequence_length; i++) {
    if (loaded_sequence[i]) {
      loaded_sequence[i]->reset();
    }
  }
}

void AgitationProcessInterpreter::skipToNextStep() { advanceToNextStep(); }

uint32_t AgitationProcessInterpreter::getCurrentMovementTimeRemaining() const {
  if (current_movement_index < sequence_length &&
      loaded_sequence[current_movement_index]) {
    return loaded_sequence[current_movement_index]->timeRemaining();
  }
  return 0;
}

uint32_t AgitationProcessInterpreter::getCurrentMovementTimeElapsed() const {
  if (current_movement_index < sequence_length &&
      loaded_sequence[current_movement_index]) {
    return loaded_sequence[current_movement_index]->timeElapsed();
  }
  return 0;
}

uint32_t AgitationProcessInterpreter::getCurrentMovementDuration() const {
  if (current_movement_index < sequence_length &&
      loaded_sequence[current_movement_index]) {
    return loaded_sequence[current_movement_index]->getDuration();
  }
  return 0;
}

bool AgitationProcessInterpreter::isWaitingForUser() const {
  if (current_movement_index < sequence_length &&
      loaded_sequence[current_movement_index]) {
    return loaded_sequence[current_movement_index]->getType() ==
           AgitationMovement::Type::WaitUser;
  }
  return false;
}

const char *AgitationProcessInterpreter::getUserMessage() const {
  static char next_step_message[32]; // Static buffer for the message

  if (current_step_index + 1 >= process->steps_length) {
    return "Finish";
  } else {
    snprintf(next_step_message, sizeof(next_step_message), "Next: %s",
             process->steps[current_step_index + 1].name);
    return next_step_message;
  }
}

void AgitationProcessInterpreter::advanceToNextMovement() {
  if (current_movement_index < sequence_length) {
    DEBUG_PRINT("Advancing to next movement: %lu/%lu",
                current_movement_index + 1, sequence_length);

    current_movement_index++;
    if (current_movement_index < sequence_length &&
        loaded_sequence[current_movement_index]) {
      loaded_sequence[current_movement_index]->reset();
    }
  }
}

const AgitationStepStatic *AgitationProcessInterpreter::getCurrentStep() const {
  if (!process || current_step_index >= process->steps_length) {
    return nullptr;
  }
  return &process->steps[current_step_index];
}

const AgitationMovement *
AgitationProcessInterpreter::getCurrentMovement() const {
  if (current_movement_index >= sequence_length) {
    return nullptr;
  }
  return loaded_sequence[current_movement_index];
}
