#pragma once
#include "movement.hpp"
#include <cstddef>

class LoopMovement final : public AgitationMovement {
public:
  LoopMovement(AgitationMovement **sequence, size_t sequence_length,
               uint32_t iterations, uint32_t max_duration)
      : AgitationMovement(Type::Loop, max_duration), sequence(sequence),
        sequence_length(sequence_length), iterations(iterations),
        current_iteration(0), current_index(0) {}

  bool execute(MotorController &motor) override {
    if (isComplete()) {
      return false;
    }

    if (current_index >= sequence_length) {
      current_index = 0;
      current_iteration++;
      if (current_iteration >= iterations) {
        return false;
      }
      if (sequence[current_index]) {
        sequence[current_index]->reset();
      }
    }

    bool result = sequence[current_index]->execute(motor);
    if (!result) {
      current_index++;
      if (current_index < sequence_length && sequence[current_index]) {
        sequence[current_index]->reset();
      }
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

  void print() const override {
    DEBUG_PRINT("LoopMovement: iteration %u/%u, sequence length %lu",
                current_iteration, iterations, sequence_length);

    DEBUG_PRINT("Sequence:");
    for (size_t i = 0; i < sequence_length; i++) {
      if (sequence[i]) {
        DEBUG_PRINTN("%s[%lu]%s ", i == current_index ? ">" : " ", i,
                     i == current_index ? "<" : " ");
        sequence[i]->print();
      }
    }
  }

  uint32_t timeElapsed() const override { return elapsed_time; }

  uint32_t timeRemaining() const override {
    if (duration > 0) {
      return duration > elapsed_time ? duration - elapsed_time : 0;
    }

    // Calculate remaining time based on current sequence position
    uint32_t remaining = 0;
    // Add time for remaining movements in current iteration
    for (size_t i = current_index; i < sequence_length; i++) {
      if (sequence[i]) {
        remaining += sequence[i]->timeRemaining();
      }
    }
    // Add time for remaining full iterations
    if (iterations > 0) {
      uint32_t remaining_iterations = iterations - current_iteration - 1;
      uint32_t iteration_duration = 0;
      for (size_t i = 0; i < sequence_length; i++) {
        if (sequence[i]) {
          iteration_duration += sequence[i]->getDuration();
        }
      }
      remaining += iteration_duration * remaining_iterations;
    }
    return remaining;
  }

  void advanceToNext() {
    current_index++;
    if (current_index >= sequence_length) {
      current_index = 0;
      current_iteration++;
    }
    // Reset the current movement
    if (current_index < sequence_length && sequence[current_index]) {
      sequence[current_index]->reset();
    }
  }

  uint32_t getDuration() const override {
    if (duration > 0) {
      return duration;
    }

    // Calculate total duration based on sequence
    uint32_t total = 0;
    for (size_t i = 0; i < sequence_length; i++) {
      if (sequence[i]) {
        total += sequence[i]->getDuration();
      }
    }
    // Multiply by number of iterations
    return total * iterations;
  }

private:
  AgitationMovement **sequence;
  size_t sequence_length;
  uint32_t iterations;
  uint32_t current_iteration;
  size_t current_index;
};
