#pragma once

#include <cstdint>
#include <string>
#include <sdbusplus/base.hpp>

#define MAC_ADDR_SIZE 6

struct MacAddr {
  uint8_t octet[MAC_ADDR_SIZE];  // network order
};

bool GetMacAddr(MacAddr* mac, const std::string& iface_path, sdbusplus::bus::BusAbc &dbus);
