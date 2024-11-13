#pragma once
#include "movement.hpp"
#include "movement_factory.hpp"
#include "../agitation_sequence.hpp"
#include <array>

class MovementLoader {
public:
    // Maximum number of movements in a sequence
    static constexpr size_t MAX_SEQUENCE_LENGTH = 32;

    /**
     * @brief Load a sequence of movements from static declarations
     * @param static_sequence Array of static movement declarations
     * @param sequence_length Length of the static sequence
     * @return Array of AgitationMovement pointers and actual length
     */
    static std::pair<AgitationMovement**, size_t> loadSequence(
        const AgitationMovementStatic* static_sequence,
        size_t sequence_length) {
        
        // Allocate array of pointers
        static AgitationMovement* sequence[MAX_SEQUENCE_LENGTH];
        size_t loaded_length = 0;

        for(size_t i = 0; i < sequence_length && i < MAX_SEQUENCE_LENGTH; i++) {
            sequence[loaded_length] = loadMovement(static_sequence[i]);
            if(sequence[loaded_length]) {
                loaded_length++;
            }
        }

        return {sequence, loaded_length};
    }

private:
    /**
     * @brief Load a single movement from static declaration
     */
    static AgitationMovement* loadMovement(const AgitationMovementStatic& static_movement) {
        switch(static_movement.type) {
        case AgitationMovementTypeCW:
            return MovementFactory::createCW(static_movement.duration);

        case AgitationMovementTypeCCW:
            return MovementFactory::createCCW(static_movement.duration);

        case AgitationMovementTypePause:
            return MovementFactory::createPause(static_movement.duration);

        case AgitationMovementTypeLoop: {
            // First load the inner sequence
            auto [inner_sequence, inner_length] = loadSequence(
                static_movement.loop.sequence,
                static_movement.loop.sequence_length);

            // Then create the loop movement
            return MovementFactory::createLoop(
                inner_sequence,
                inner_length,
                static_movement.loop.count,
                static_movement.loop.max_duration);
        }

        case AgitationMovementTypeWaitUser:
            return MovementFactory::createWaitUser();

        default:
            return nullptr;
        }
    }
}; 