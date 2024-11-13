#include "movement_loader.hpp"
#include "../processes/c41_process.hpp"
#include <cassert>

void test_load_c41_sequence() {
    // Reset the movement factory
    MovementFactory::reset();

    // Load the C41 color developer sequence
    auto [sequence, length] = MovementLoader::loadSequence(
        C41_COLOR_DEVELOPER,
        C41_COLOR_DEVELOPER_LENGTH);

    // Verify sequence was loaded
    assert(sequence != nullptr);
    assert(length == C41_COLOR_DEVELOPER_LENGTH);

    // Create motor controller for testing
    MotorController motor;

    // Execute first movement (should be a loop)
    assert(sequence[0]->getType() == AgitationMovement::Type::Loop);
    
    // Execute sequence
    while(!sequence[0]->isComplete()) {
        sequence[0]->execute(motor);
        // In real code we'd wait 1 second between ticks
    }

    // Clean up
    MovementFactory::reset();
} 