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
#include "checkstop.hpp"
namespace phosphor
{
namespace checkstop
{

using namespace phosphor::logging;

// Gets the file descriptor for passed in device
void Handler::getFD()
{
    // TODO: For now, this just opens the device and returns the FD.
    // When 4.9 kernel is merged, this will be changed to use some IOCTL to
    // associate a FD to passed in GPIO device.
    fd = open(device.c_str(), O_RDONLY | O_NONBLOCK);
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
                             EPOLLIN, callbackHandler, this);
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
    // TODO. This calls into starting quiesce target
    log<level::INFO>("Callback handler called");
    return 0;
}

} // namespace checkstop
} // namespace phosphor
