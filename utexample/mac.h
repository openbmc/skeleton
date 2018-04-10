#pragma once

#include <cstdint>
#include <sdbusplus/bus.hpp>
#include <string>

#define MAC_ADDR_SIZE 6

struct MacAddr {
  uint8_t octet[MAC_ADDR_SIZE];  // network order
};

bool GetMacAddr(
  MacAddr* mac,
  const std::string& iface_path,
  sdbusplus::bus::bus &dbus);
