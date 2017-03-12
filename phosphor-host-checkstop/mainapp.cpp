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
#include "argument.hpp"
#include "checkstop.hpp"

static void ExitWithError(const char* err, char** argv)
{
    phosphor::checkstop::ArgumentParser::usage(argv);
    std::cerr << std::endl;
    std::cerr << "ERROR: " << err << std::endl;
    exit(-1);
}

int main(int argc, char** argv)
{
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

    // Create a checkstop handler object and let it do all the rest
    phosphor::checkstop::Handler handler(device, std::stoi(line));

    return 0;
}
