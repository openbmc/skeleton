/*
// Copyright (c) 2018 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#pragma once
#include <boost/asio.hpp>
#include <experimental/filesystem>
#include <phosphor-logging/log.hpp>
#include <sdbusplus/asio/object_server.hpp>

class GpioEn
{
    std::string name;
    uint16_t number;
    bool enabled;

  public:
    GpioEn(const std::string& name_, uint16_t number_, bool enabled_) :
        name(name_), number(number_), enabled(enabled_){};

    std::string getName() const
    {
        return name;
    }
    uint16_t getNumber() const
    {
        return number;
    }
    bool getEnabled() const
    {
        return enabled;
    }
    void setEnabled(bool value)
    {
        enabled = value;
    }
};

class GpioManager
{
    std::vector<GpioEn> gpioEnableList;
    std::vector<std::experimental::filesystem::path> gpioPaths;
    sdbusplus::asio::object_server& server;
    std::shared_ptr<sdbusplus::asio::connection> conn;
    std::shared_ptr<sdbusplus::asio::dbus_interface> iface;
    std::vector<std::string> paths;

    GpioEn* findGpioObj(const std::string& gpioName);
    bool findGpioDirs(
        const std::experimental::filesystem::path& dirPath,
        const std::string& matchString,
        std::vector<std::experimental::filesystem::path>& foundPaths);

  public:
    GpioManager(sdbusplus::asio::object_server& srv,
                std::shared_ptr<sdbusplus::asio::connection>& conn);
};
