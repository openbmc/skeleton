#include "mac.h"

#include <cstdio>
#include <iostream>
#include <sdbusplus/base.hpp>
#include <sdbusplus/bus.hpp>

constexpr auto MAC_FORMAT = "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx";
constexpr auto MAC_INTERFACE = "xyz.openbmc_project.Network.MACAddress";
constexpr auto NETWORK_INTERFACE = "xyz.openbmc_project.Network";
constexpr auto PROP_INTERFACE = "org.freedesktop.DBus.Properties";
constexpr auto IFACE_ROOT = "/xyz/openbmc_project/network/";

// Not yet tested.
bool ParseMac(const std::string& mac_addr, MacAddr* mac)
{
  int ret = std::sscanf(mac_addr.c_str(), MAC_FORMAT,
                     mac->octet,
                     mac->octet + 1,
                     mac->octet + 2,
                     mac->octet + 3,
                     mac->octet + 4,
                     mac->octet + 5);
  return (ret == 6);
}

// Now fully unit-tested.
bool GetMacAddr(MacAddr* mac, const std::string& iface_path, sdbusplus::bus::BusAbc &dbus)
{
  if (mac == nullptr)
  {
    std::cerr << "Nemora::GetMacAddr MAC Address is nullptr" << std::endl;
    return false;
  }

  auto networkd_call = dbus.new_method_call(
                             NETWORK_INTERFACE,
                             iface_path.c_str(),
                             PROP_INTERFACE,
                             "Get");

  // this calls the tuple with ss
  networkd_call.append(MAC_INTERFACE, "MACAddress");

  auto reply = dbus.call(networkd_call);
  if (reply.is_method_error())
  {
    std::cerr << "Nemora::GetMacAddr Failed to get MAC Address" << std::endl;
    return false;
  }
  sdbusplus::message::variant<std::string> result;
  reply.read(result);
  auto mac_addr = result.get<std::string>();
  if (!ParseMac(mac_addr, mac))
  {
    std::cerr << "Nemora::GetMacAddr Failed to parse MAC Address: "
              << mac_addr
              << std::endl;
    return false;
  }
  return true;
}
