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
#include "monitor.hpp"
#include "config.h"

namespace phosphor
{
namespace gpio
{

// systemd service to kick start a target.
constexpr auto SYSTEMD_SERVICE        = "org.freedesktop.systemd1";
constexpr auto SYSTEMD_ROOT           = "/org/freedesktop/systemd1";
constexpr auto SYSTEMD_INTERFACE      = "org.freedesktop.systemd1.Manager";

using namespace phosphor::logging;

// Populate the file descriptor for passed in device
void Monitor::openDevice()
{
    fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0)
    {
        log<level::ERR>("Failed to open device",
                entry("PATH=%s", path.c_str()));
        throw std::runtime_error("Failed to open device");
    }
}

// Attaches the FD to event loop and registers the callback handler
void Monitor::registerCallback()
{
    auto r = sd_event_add_io(event, &eventSource, fd,
                             EPOLLIN, callbackHandler, this);
    if (r < 0)
    {
        log<level::ERR>("Failed to register callback handler",
                entry("ERROR=%s", strerror(-r)));
        throw std::runtime_error("Failed to register callback handler");
    }
}

// Callback handler when there is an activity on the FD
int Monitor::processEvents(sd_event_source* es, int fd,
                           uint32_t revents, void* userData)
{
    log<level::INFO>("GPIO line altered");
    auto monitor = static_cast<Monitor*>(userData);

    // TODO : Need a way to check if the GPIO state change is what we wanted
    return monitor->analyzeEvent();
}

// Analyzes the GPIO event
int Monitor::analyzeEvent()
{
    auto method = bus.new_method_call(SYSTEMD_SERVICE,
                                      SYSTEMD_ROOT,
                                      SYSTEMD_INTERFACE,
                                      "StartUnit");
    method.append(TARGET_UNIT);
    method.append("replace");

    // If there is any error, an exception would be thrown from here.
    bus.call_noreply(method);

    // This marks the completion of handling the checkstop and app can exit
    complete = true;

    return 0;
}

} // namespace gpio
} // namespace phosphor
