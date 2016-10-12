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
#include "config.h"
#include <cstdlib>
#include <iostream>
#include <exception>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/vtable.hpp>
#include <sdbusplus/message.hpp>

constexpr auto busname = BUSNAME;
constexpr auto root = INVENTORY_ROOT;
constexpr auto iface = IFACE;

using object = std::map<std::string, std::map<std::string,
      sdbusplus::message::variant<std::string, int>>>;

auto notify(sd_bus_message *m, void *data, sd_bus_error *e)
{
    try {
        std::string path;
        object obj;
        auto msg = sdbusplus::message::message(m);

        sd_bus_message_ref(m);

        msg.read(path, obj);

        // TODO unstub
        std::cout << __FUNCTION__ << ": " << path << std::endl;
        sd_bus_reply_method_return(m, nullptr);
    }
    catch (std::exception &e) {
        sd_bus_reply_method_errorf(m, SD_BUS_ERROR_FAILED, e.what());
    }

    return 0;
}

constexpr sdbusplus::vtable::vtable_t manager_vtable[] =
{
    sdbusplus::vtable::start(),

    // TODO replace path type with 'o'
    sdbusplus::vtable::method(
            "Notify", "sa{sa{sv}}", nullptr, notify),
    sdbusplus::vtable::end()
};

int main(int argc, char *argv[])
{
    auto bus = [](){
        try {
            auto b = sdbusplus::bus::new_system();
            b.add_object_manager(root);
            b.add_object_vtable(root,
                    iface,
                    *manager_vtable);
            b.request_name(busname);
            return b;
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
            exit(EXIT_FAILURE);
        }
    }();

    while(true) {
        try {
            bus.process();
            bus.wait();
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }

    exit(EXIT_SUCCESS);
}

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
