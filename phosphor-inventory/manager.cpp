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
#include <exception>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/vtable.hpp>
#include <sdbusplus/message.hpp>
#include "manager.hpp"

namespace phosphor
{

namespace inventory
{

auto _notify(sd_bus_message *m, void *data, sd_bus_error *e)
{
    try {
        auto msg = sdbusplus::message::message(m);
        sd_bus_message_ref(m);
        auto mgr = static_cast<manager *>(data);
        mgr->notify(msg);
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
            "Notify", "sa{sa{sv}}", nullptr, _notify),
    sdbusplus::vtable::end()
};

manager::manager(const char *busname, const char *root, const char *iface) :
    _bus(sdbusplus::bus::new_system()),
    _shutdown(false)
{
    _bus.add_object_manager(root);
    _bus.add_object_vtable(root,
            iface,
            *manager_vtable);
    _bus.request_name(busname);
}

void manager::shutdown()
{
    _shutdown = true;
}

void manager::run()
{
    while(!_shutdown) {
        try {
            _bus.process();
            _bus.wait(5000000);
        }
        catch (std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
    }
}

void manager::notify(sdbusplus::message::message &msg)
{
    using object = std::map<std::string, std::map<std::string,
          sdbusplus::message::variant<std::string, int>>>;

    std::string path;
    object obj;
    msg.read(path, obj);

    // TODO unstub
    std::cout << __FUNCTION__ << ": " << path << std::endl;
}

} // namespace inventory

} // namespace phosphor

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
