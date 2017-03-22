/**
 * Copyright © 2016 IBM Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fcntl.h>
#include <phosphor-logging/log.hpp>
#include <xyz/openbmc_project/State/Host/server.hpp>
#include "checkstop.hpp"
namespace phosphor
{
namespace checkstop
{

// systemd service to kick start a target.
constexpr auto SYSTEMD_SERVICE        = "org.freedesktop.systemd1";
constexpr auto SYSTEMD_ROOT           = "/org/freedesktop/systemd1";
constexpr auto SYSTEMD_INTERFACE      = "org.freedesktop.systemd1.Manager";
constexpr auto HOST_QUIESCE_TARGET    = "obmc-quiesce-host@0.target";

// To know the current state of Host.
constexpr auto HOST_STATE_MANAGER_ROOT  = "/xyz/openbmc_project/state/host0";
constexpr auto HOST_STATE_MANAGER_IFACE = "xyz.openbmc_project.State.Host";
constexpr auto DBUS_PROPERTY_IFACE      = "org.freedesktop.DBus.Properties";
constexpr auto PROPERTY                 = "CurrentHostState";

// Phosphor State manager
namespace State = sdbusplus::xyz::openbmc_project::State::server;

using namespace phosphor::logging;

// Gets the file descriptor for passed in device
void Handler::getFD()
{
    // TODO: For now, this just opens the device and returns the FD.
    // When 4.9 kernel is merged, this will be changed to use some IOCTL to
    // associate a FD to passed in GPIO device.
    fd = open(device.c_str(), O_RDONLY);
    if (fd < 0)
    {
        log<level::ERR>("Failed to open device",
                entry("PATH=%s", device.c_str()));
        throw std::runtime_error("Failed to open device");
    }
}

// Attaches the FD to event loop and registers the callback handler
void Handler::registerCallback()
{
    auto r = sd_event_add_io(event, &eventSource, fd,
                             EPOLLIN, processEvents, this);
    if (r < 0)
    {
        log<level::ERR>("Failed to register callback handler",
                entry("ERROR=%s", strerror(-r)));
        throw std::runtime_error("Failed to register callback handler");
    }
}

// Callback handler when there is an activity on the FD
int Handler::processEvents(sd_event_source* es, int fd,
                           uint32_t revents, void* userData)
{
    log<level::INFO>("GPIO line altered");
    auto handler = static_cast<Handler*>(userData);
    return handler->analyzeEvent();
}

// Analyzes the GPIO event
int Handler::analyzeEvent()
{
    // To extract the current state of Host
    sdbusplus::message::variant<std::string> currHostState {};

    // Who is hosting the State Manager service.
    auto service = getServiceName(HOST_STATE_MANAGER_ROOT,
                                  HOST_STATE_MANAGER_IFACE);
    if(service.empty())
    {
        // Do not want to get into quiesce state when we don't know for sure.
        return 0;
    }

    auto method = bus.new_method_call(service.c_str(),
                                      HOST_STATE_MANAGER_ROOT,
                                      DBUS_PROPERTY_IFACE,
                                      "Get");
    method.append(HOST_STATE_MANAGER_IFACE);
    method.append(PROPERTY);

    // If the last known HOST state is Running, then transition
    // to Quiesce state.
    auto reply = bus.call(method);
    reply.read(currHostState);

    // TODO Check if the GPIO really asserted.
    if (currHostState == State::convertForMessage(
                State::Host::HostState::Running))
    {
        auto method = bus.new_method_call(SYSTEMD_SERVICE,
                                          SYSTEMD_ROOT,
                                          SYSTEMD_INTERFACE,
                                          "StartUnit");
        method.append(HOST_QUIESCE_TARGET);
        method.append("replace");

        // If there is any error, an exception would be thrown from here.
        bus.call_noreply(method);

        // This marks the completion of handling the checkstop and app can exit
        completed = true;
    }
    else
    {
        log<level::INFO>("Host is not Running. ignoring GPIO assertion",
                entry("CURRENT_STATE=%s", currHostState));
    }
    return 0;
}

// Given the LED dbus path and interface, returns the service name
std::string Handler::getServiceName(const std::string& objPath,
                                    const std::string& interface)
{
    using namespace phosphor::logging;

    // Mapper dbus constructs
    constexpr auto MAPPER_BUSNAME   = "xyz.openbmc_project.ObjectMapper";
    constexpr auto MAPPER_OBJ_PATH  = "/xyz/openbmc_project/ObjectMapper";
    constexpr auto MAPPER_IFACE     = "xyz.openbmc_project.ObjectMapper";

    // Make a mapper call
    auto mapperCall = bus.new_method_call(MAPPER_BUSNAME, MAPPER_OBJ_PATH,
                                          MAPPER_IFACE, "GetObject");
    // Cook rest of the things.
    mapperCall.append(objPath);
    mapperCall.append(std::vector<std::string>({interface}));

    auto reply = bus.call(mapperCall);
    if (reply.is_method_error())
    {
        // Its okay if we do not see a corresponding physical LED.
        log<level::INFO>("Error looking up service",
                entry("PATH=%s",objPath.c_str()));
        return "";
    }

    // Response by mapper in the case of success
    std::map<std::string, std::vector<std::string>> serviceNames;

    // This is the service name for the passed in objpath
    reply.read(serviceNames);
    if (serviceNames.empty())
    {
        log<level::INFO>("Service lookup did not return any service",
                entry("PATH=%s",objPath.c_str()));
        return "";
    }

    return serviceNames.begin()->first;
}

} // namespace checkstop
} // namespace phosphor
