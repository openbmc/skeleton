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

#include <iostream>
#include <string>
#include <systemd/sd-event.h>
#include <phosphor-logging/log.hpp>
#include "argument.hpp"
#include "checkstop.hpp"

using namespace phosphor::logging;

static void ExitWithError(const char* err, char** argv)
{
    phosphor::checkstop::ArgumentParser::usage(argv);
    std::cerr << std::endl;
    std::cerr << "ERROR: " << err << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
    sd_event* event = nullptr;

    // Read arguments.
    auto options = phosphor::checkstop::ArgumentParser(argc, argv);

    // Parse out device argument.
    auto device = std::move((options)["device"]);
    if (device == phosphor::checkstop::ArgumentParser::empty_string)
    {
        ExitWithError("device not specified.", argv);
    }

    // Parse out line number
    auto line = std::move((options)["line"]);
    if (line == phosphor::checkstop::ArgumentParser::empty_string)
    {
        ExitWithError("line not specified.", argv);
    }

    auto r = sd_event_default(&event);
    if (r < 0)
    {
        log<level::ERR>("Error creating a default sd_event handler");
        return r;
    }

    // Create a checkstop handler object and let it do all the rest
    phosphor::checkstop::Handler handler(device, std::stoi(line), event,
                            phosphor::checkstop::Handler::processEvents);

    // Wait for events
    sd_event_loop(event);

    // Delete the event memory
    event = sd_event_unref(event);

    return 0;
}
