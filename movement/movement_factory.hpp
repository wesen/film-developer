#pragma once
#include "loop_movement.hpp"
#include "motor_movement.hpp"
#include "movement.hpp"
#include "pause_movement.hpp"
#include "wait_user_movement.hpp"
#include <array>
#include <new>

class MovementFactory {
public:
  static constexpr size_t MAX_MOVEMENTS = 64;
  static constexpr size_t MAX_SEQUENCE_LENGTH = 12;

  static size_t getAvailableSpace() {
    return movement_pool.size() - current_pool_index;
  }

  static bool canAllocate(size_t size) {
    return (current_pool_index + size <= movement_pool.size());
  }

  static AgitationMovement *createCW(uint32_t duration) {
    if (!canAllocate(sizeof(MotorMovement))) {
      DEBUG_PRINT("Cannot allocate CW movement, need %zu bytes, have %zu",
                  sizeof(MotorMovement), getAvailableSpace());
      return nullptr;
    }
    void *ptr = allocateMovement(sizeof(MotorMovement));
    if (!ptr)
      return nullptr;
    return new (ptr) MotorMovement(AgitationMovement::Type::CW, duration);
  }

  static AgitationMovement *createCCW(uint32_t duration) {
    if (!canAllocate(sizeof(MotorMovement))) {
      DEBUG_PRINT("Cannot allocate CCW movement, need %zu bytes, have %zu",
                  sizeof(MotorMovement), getAvailableSpace());
      return nullptr;
    }
    void *ptr = allocateMovement(sizeof(MotorMovement));
    if (!ptr)
      return nullptr;
    return new (ptr) MotorMovement(AgitationMovement::Type::CCW, duration);
  }

  static AgitationMovement *createPause(uint32_t duration) {
    if (!canAllocate(sizeof(PauseMovement))) {
      DEBUG_PRINT("Cannot allocate Pause movement, need %zu bytes, have %zu",
                  sizeof(PauseMovement), getAvailableSpace());
      return nullptr;
    }
    void *ptr = allocateMovement(sizeof(PauseMovement));
    if (!ptr)
      return nullptr;
    return new (ptr) PauseMovement(duration);
  }

  static AgitationMovement *createLoop(const AgitationMovement **sequence,
                                       size_t sequence_length,
                                       uint32_t iterations,
                                       uint32_t max_duration) {
    size_t sequence_storage_size =
        sizeof(AgitationMovement *) * sequence_length;
    size_t loop_movement_size = sizeof(LoopMovement);
    size_t total_size = sequence_storage_size + loop_movement_size;

    if (!canAllocate(total_size)) {
      DEBUG_PRINT("Cannot allocate Loop movement, need %zu bytes, have %zu",
                  total_size, getAvailableSpace());
      return nullptr;
    }

    auto sequence_storage = reinterpret_cast<AgitationMovement **>(
        allocateMovement(sequence_storage_size));

    if (!sequence_storage) {
      DEBUG_PRINT("Failed to allocate sequence storage");
      return nullptr;
    }

    for (size_t i = 0; i < sequence_length; i++) {
      sequence_storage[i] = const_cast<AgitationMovement *>(sequence[i]);
    }

    void *loop_ptr = allocateMovement(loop_movement_size);
    if (!loop_ptr) {
      DEBUG_PRINT("Failed to allocate loop movement");
      return nullptr;
    }

    return new (loop_ptr) LoopMovement(sequence_storage, sequence_length,
                                       iterations, max_duration);
  }

  static AgitationMovement *createWaitUser() {
    if (!canAllocate(sizeof(WaitUserMovement))) {
      DEBUG_PRINT("Cannot allocate WaitUser movement, need %zu bytes, have %zu",
                  sizeof(WaitUserMovement), getAvailableSpace());
      return nullptr;
    }
    void *ptr = allocateMovement(sizeof(WaitUserMovement));
    if (!ptr)
      return nullptr;
    return new (ptr) WaitUserMovement();
  }

  static void reset() {
    current_pool_index = 0;
    DEBUG_PRINT("Movement factory reset, %zu bytes available",
                movement_pool.size());
  }

  static void printPoolStats() {
    DEBUG_PRINT("Movement pool: %zu/%zu bytes used (%zu%% full)",
                current_pool_index, movement_pool.size(),
                (current_pool_index * 100) / movement_pool.size());
  }

private:
  static void *allocateMovement(size_t size) {
    if (current_pool_index + size > movement_pool.size()) {
      DEBUG_PRINT("Movement pool overflow: needed %zu bytes, %zu available",
                  size, movement_pool.size() - current_pool_index);
      return nullptr;
    }
    void *ptr = &movement_pool[current_pool_index];
    current_pool_index += size;
    return ptr;
  }

  static inline std::array<uint8_t, MAX_MOVEMENTS * sizeof(AgitationMovement)>
      movement_pool;
  static inline size_t current_pool_index = 0;
};
