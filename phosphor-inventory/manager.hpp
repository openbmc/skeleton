#pragma once

#include <sdbusplus/message.hpp>
#include <sdbusplus/bus.hpp>

namespace phosphor
{

namespace inventory
{

struct manager
{
    manager() = delete;
    manager(const manager&) = delete;
    manager& operator=(const manager&) = delete;
    manager(manager&&) = default;
    manager& operator=(manager&&) = default;
    ~manager() = default;
    manager(const char *, const char*, const char*);

    void run();
    void shutdown();
    void notify(sdbusplus::message::message &);

    private:
    sdbusplus::bus::bus _bus;
    bool _shutdown;
};

} // namespace inventory

} // namespace phosphor

// vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
