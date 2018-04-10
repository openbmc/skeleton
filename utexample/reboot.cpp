#include "reboot.h"

#include <sdbusplus/base.hpp>
#include <sdbusplus/message.hpp>

static constexpr auto SYSTEMD_SERVICE    = "org.freedesktop.systemd1";
static constexpr auto SYSTEMD_ROOT       = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMD_INTERFACE  = "org.freedesktop.systemd1.Manager";
static constexpr auto RESET_BUTTON_TARGET    = "reset-button.target";

void BoardResetButtonCycle(sdbusplus::bus::BusAbc &bus)
{
  auto reset_button_method = bus.new_method_call(SYSTEMD_SERVICE,
        SYSTEMD_ROOT,
        SYSTEMD_INTERFACE,
        "StartUnit");
  reset_button_method.append(RESET_BUTTON_TARGET);
  reset_button_method.append("replace");
  bus.call_noreply(reset_button_method);
}

