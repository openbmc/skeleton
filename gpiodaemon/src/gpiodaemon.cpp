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

#include "gpiodaemon.hpp"
#include "gpioutils.hpp"
#include <boost/algorithm/string/replace.hpp>
#include <regex>

static constexpr const char* entityMgrService =
    "xyz.openbmc_project.EntityManager";
static constexpr const char* gpioService = "xyz.openbmc_project.Gpio";
static constexpr const char* gpioConfigInterface =
    "xyz.openbmc_project.Configuration.Gpio";
static constexpr const char* gpioInterface =
    "xyz.openbmc_project.Control.Gpio";
static constexpr const char* gpioPath = "/xyz/openbmc_project/control/gpio/";
static constexpr const char* propInterface = "org.freedesktop.DBus.Properties";

static constexpr const char* objPath = "/xyz/openbmc_project/object_mapper";
static constexpr const char* objService = "xyz.openbmc_project.ObjectMapper";
static constexpr const char* objInterface = "xyz.openbmc_project.ObjectMapper";

const static constexpr size_t waitTime = 2; // seconds

using BasicVariantType =
    sdbusplus::message::variant<std::string, int64_t, uint64_t, double, int32_t,
                                uint32_t, int16_t, uint16_t, uint8_t, bool>;

GpioManager::GpioManager(sdbusplus::asio::object_server& srv_,
                         std::shared_ptr<sdbusplus::asio::connection>& conn_) :
    server(srv_),
    conn(conn_)
{
    using GetSubTreeType = std::vector<std::pair<
        std::string,
        std::vector<std::pair<std::string, std::vector<std::string>>>>>;

    auto mesg =
        conn->new_method_call(objService, objPath, objInterface, "GetSubTree");

    static const auto depth = 3;
    mesg.append("/xyz/openbmc_project/inventory/system", depth,
                std::vector<std::string>());

    try
    {
        auto ret = conn->call(mesg);

        GetSubTreeType subtree;
        ret.read(subtree);

        for (const auto& i : subtree)
        {
            for (const auto& j : i.second)
            {
                for (const auto& k : j.second)
                {
                    if (gpioConfigInterface == k)
                    {
                        paths.push_back(i.first);
                    }
                }
            }
        }
    }
    catch (sdbusplus::exception::SdBusError& e)
    {
        phosphor::logging::log<phosphor::logging::level::INFO>(
            "ERROR while readding ObjectMapper.");
        return;
    }

    for (const auto& path : paths)
    {
        conn->async_method_call(
            [this, &path](
                boost::system::error_code ec,
                const boost::container::flat_map<std::string, BasicVariantType>&
                    result) {
                if (ec)
                {
                    phosphor::logging::log<phosphor::logging::level::INFO>(
                        ("ERROR with async_method_call path: " + path).c_str());
                }
                else
                {
                    auto nameFind = result.find("Name");
                    auto indexFind = result.find("Index");
                    auto polarityFind = result.find("Polarity");

                    if (nameFind == result.end() || indexFind == result.end() ||
                        polarityFind == result.end())
                    {
                        phosphor::logging::log<phosphor::logging::level::INFO>(
                            "ERROR accessing entity manager.");
                        return;
                    }

                    uint64_t index = indexFind->second.get<uint64_t>();
                    std::string gpioName = nameFind->second.get<std::string>();
                    bool inverted = polarityFind->second.get<std::string>() == "Low";

                    boost::replace_all(gpioName, " ", "_");

                    // Add gpio entries to be managed by gpiodaemon
                    gpioEnableList.push_back(
                        GpioEn(gpioName, (uint16_t)index, false));

                    // Read present gpio data from /sys/class/gpio/...
                    Gpio gpio = Gpio(std::to_string(index));

                    // Add path to server object
                    iface = server.add_interface(gpioPath + gpioName,
                                                 gpioInterface);

                    /////////////////////////////
                    // Set generic properties:
                    //    - enabled - when set to false other properties like
                    //    value and direction
                    //                cannot be overriden
                    //    - value
                    //    - direction
                    //    - polarity
                    /////////////////////////////
                    iface->register_property(
                        "Enabled", true,
                        [this, gpioName](const bool& req, bool& propertyValue) {
                            auto it = findGpioObj(gpioName);
                            if (it != nullptr)
                            {
                                it->setEnabled(req);
                                propertyValue = req;
                                return 1;
                            }
                            return 0;
                        });

                    bool value = static_cast<bool>(gpio.getValue());
                    if (inverted)
                    {
                        value = !value;
                    }
                    iface->register_property(
                        "Value", value,
                        // Override set
                        [this, gpioName, inverted](const bool& req, bool& propertyValue) {
                            auto it = findGpioObj(gpioName);

                            if (it != nullptr)
                            {
                                // If Gpio enabled property is set as true then
                                // reject set request
                                if (it->getEnabled())
                                {
                                    Gpio gpio =
                                        Gpio(std::to_string(it->getNumber()));
                                    bool setVal = req;
                                    if (inverted)
                                    {
                                        setVal = !setVal;
                                    }
                                    gpio.setValue(static_cast<GpioValue>(setVal));

                                    propertyValue = req;
                                    return 1;
                                }
                                return 0;
                            }
                            return 0;
                        });

                    iface->register_property(
                        "Direction", gpio.getDirection(),
                        // Override set
                        [this, gpioName](const std::string& req,
                                         std::string& propertyValue) {
                            auto it = findGpioObj(gpioName);

                            if (it != nullptr)
                            {
                                // If Gpio enabled property is set as true than
                                // reject request
                                if (it->getEnabled())
                                {
                                    Gpio gpio =
                                        Gpio(std::to_string(it->getNumber()));
                                    gpio.setDirection(req);

                                    propertyValue = req;
                                    return 1;
                                }
                                return 0;
                            }
                            return 0;
                        });
                    iface->initialize();
                }
            },
            entityMgrService, path, propInterface, "GetAll", gpioConfigInterface);
    }
}

GpioEn* GpioManager::findGpioObj(const std::string& gpioName)
{
    auto it = std::find_if(
        gpioEnableList.begin(), gpioEnableList.end(),
        [&gpioName](const GpioEn& obj) { return obj.getName() == gpioName; });
    if (it != gpioEnableList.end())
    {
        return &(*it);
    }
    return nullptr;
}

bool GpioManager::findGpioDirs(
    const std::experimental::filesystem::path& dirPath,
    const std::string& matchString,
    std::vector<std::experimental::filesystem::path>& foundPaths)
{
    if (!std::experimental::filesystem::exists(dirPath))
    {
        return false;
    }

    std::regex search(matchString);
    std::smatch match;
    for (const auto& p :
         std::experimental::filesystem::recursive_directory_iterator(dirPath))
    {
        std::string path = p.path().string();
        if (is_directory(p))
        {
            if (std::regex_search(path, match, search))
            {
                foundPaths.emplace_back(p.path());
            }
        }
    }
    return true;
}

int main()
{
    boost::asio::io_service io;
    auto systemBus = std::make_shared<sdbusplus::asio::connection>(io);

    systemBus->request_name(gpioService);
    sdbusplus::asio::object_server server(systemBus);

    static auto match = std::make_unique<sdbusplus::bus::match::match>(
        static_cast<sdbusplus::bus::bus&>(*systemBus),
        "type='signal',member='PropertiesChanged',"
        "arg0namespace='" +
         std::string(gpioConfigInterface) + "'",
        [](sdbusplus::message::message& message) {
            phosphor::logging::log<phosphor::logging::level::INFO>(
             "New configuration detected. Restarting gpiodaemon.");

         std::this_thread::sleep_for(std::chrono::seconds(waitTime));

         // todo: restarting isn't the best approach, really we should detect
         // what changed and add the appropriate gpios, this will be changed
         // shortly
         std::exit(0);
     });

    GpioManager gpioMgr(server, systemBus);

    io.run();
}
