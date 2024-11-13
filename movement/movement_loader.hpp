#pragma once
#include "../agitation_sequence.hpp"
#include "movement.hpp"
#include "movement_factory.hpp"
#include <array>

class MovementLoader {
public:
  // Maximum number of movements in a sequence
  static constexpr size_t MAX_SEQUENCE_LENGTH = 32;

  /**
   * @brief Construct a MovementLoader with a movement factory
   * @param factory The factory to use for creating movements
   */
  explicit MovementLoader(MovementFactory& factory) : factory_(factory) {}

  /**
   * @brief Load a sequence of movements from static declarations
   * @param static_sequence Array of static movement declarations
   * @param sequence_length Length of the static sequence
   * @param sequence Array to store the created movements
   * @return Actual length of the loaded sequence
   */
  size_t loadSequence(const AgitationMovementStatic *static_sequence,
                     size_t sequence_length,
                     AgitationMovement* sequence[]) {
    DEBUG_PRINT("Loading sequence with length: %zu", sequence_length);

    size_t loaded_length = 0;

    for (size_t i = 0; i < sequence_length && i < MAX_SEQUENCE_LENGTH; i++) {
      DEBUG_PRINT("Loading movement %zu", i);
      sequence[loaded_length] = loadMovement(static_sequence[i]);
      if (sequence[loaded_length]) {
        DEBUG_PRINT("Loaded movement %zu", loaded_length);
        loaded_length++;
      }
    }

    DEBUG_PRINT("Loaded sequence length: %zu", loaded_length);
    DEBUG_PRINT("");

    return loaded_length;
  }

private:
  MovementFactory& factory_;

  /**
   * @brief Load a single movement from static declaration
   */
  AgitationMovement *
  loadMovement(const AgitationMovementStatic &static_movement) {
    switch (static_movement.type) {
    case AgitationMovementTypeCW:
      DEBUG_PRINT("Creating CW movement with duration: %u",
                  static_movement.duration);
      return factory_.createCW(static_movement.duration);

    case AgitationMovementTypeCCW:
      DEBUG_PRINT("Creating CCW movement with duration: %u",
                  static_movement.duration);
      return factory_.createCCW(static_movement.duration);

    case AgitationMovementTypePause:
      DEBUG_PRINT("Creating pause movement with duration: %u",
                  static_movement.duration);
      return factory_.createPause(static_movement.duration);

    case AgitationMovementTypeLoop: {
      // Create temporary array for inner sequence
      AgitationMovement* inner_sequence[MAX_SEQUENCE_LENGTH];
      
      // Load the inner sequence
      DEBUG_PRINT("Loading loop sequence with length: %lu",
                  static_movement.loop.sequence_length);
      size_t inner_length = loadSequence(
          static_movement.loop.sequence, 
          static_movement.loop.sequence_length,
          inner_sequence);

      // Print inner sequence details
      DEBUG_PRINT("---");
      DEBUG_PRINT("Inner sequence details:");
      for (size_t i = 0; i < inner_length; i++) {
        inner_sequence[i]->print();
      }

      // Create the loop movement
      DEBUG_PRINT("Creating loop movement with count: %u, max_duration: %u",
                  static_movement.loop.count,
                  static_movement.loop.max_duration);

      auto result = factory_.createLoop(
          const_cast<const AgitationMovement**>(inner_sequence),
          inner_length,
          static_movement.loop.count,
          static_movement.loop.max_duration);
      
      DEBUG_PRINT("***");
      result->print();
      DEBUG_PRINT("----");
      DEBUG_PRINT("");

      return result;
    }

    case AgitationMovementTypeWaitUser:
      return factory_.createWaitUser();

    default:
      return nullptr;
    }
  }
}; 