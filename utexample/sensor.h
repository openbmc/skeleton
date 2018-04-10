#pragma once

#include <sdbusplus/server.hpp>

#include "xyz/openbmc_project/Control/FanSpeed/server.hpp"
#include "xyz/openbmc_project/Sensor/Value/server.hpp"

template <typename... T>
using ServerObject = typename sdbusplus::server::object::object<T...>;

using ValueInterface = sdbusplus::xyz::openbmc_project::Sensor::server::Value;
using ValueObject = ServerObject<ValueInterface>;

// We'll use a ValueObject directly instead of inheriting from it, and we'll
// catch its sd-bus calls.
//

using FanSpeedInterface =
    sdbusplus::xyz::openbmc_project::Control::server::FanSpeed;
using FanSpeedObject = ServerObject<FanSpeedInterface>;

class FanSpeed : public FanSpeedObject
{
    public:
        FanSpeed(const std::string& devPath,
                 const std::string& id,
                 sdbusplus::bus::bus& bus,
                 const char* objPath,
                 bool defer,
                 uint64_t target) : FanSpeedObject(bus, objPath, defer),
                    id(id),
                    devPath(devPath)
        {
            FanSpeedObject::target(target);
        }

        uint64_t target(uint64_t value) override;
        uint64_t target() const override;

    private:
        std::string id;
        std::string devPath;
};

