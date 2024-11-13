#pragma once
#include <cstdint>
#include "../motor_controller.hpp"

class AgitationMovement {
public:
    enum class Type {
        CW,
        CCW,
        Pause,
        Loop,
        WaitUser
    };

    explicit AgitationMovement(Type type, uint32_t duration = 0)
        : type(type), duration(duration) {}
    
    virtual ~AgitationMovement() = default;

    virtual bool execute(MotorController& motor) = 0;
    virtual bool isComplete() const = 0;
    virtual void reset() = 0;

    Type getType() const { return type; }
    uint32_t getDuration() const { return duration; }

protected:
    Type type;
    uint32_t duration;
    uint32_t elapsed_time{0};
}; 