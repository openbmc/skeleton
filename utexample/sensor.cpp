#include "sensor.h"

uint64_t FanSpeed::target(uint64_t value)
{
    auto curValue = FanSpeedObject::target();

    if (curValue != value)
    {
        // Do something, we can easily drop in our own IoAccess object and
        // capture this, but my goal is to firstly test the sd-bus stuff which
        // should now be exposed.
    }

    return FanSpeedObject::target(value);
}

uint64_t FanSpeed::target() const
{
    return FanSpeedObject::target();
}
