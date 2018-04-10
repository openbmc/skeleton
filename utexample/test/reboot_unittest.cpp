#include "reboot.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::Return;

// Since these are referenced in a couple places, we should promote them to a header.
static constexpr auto SYSTEMD_SERVICE    = "org.freedesktop.systemd1";
static constexpr auto SYSTEMD_ROOT       = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMD_INTERFACE  = "org.freedesktop.systemd1.Manager";
static constexpr auto RESET_BUTTON_TARGET    = "reset-button.target";

TEST(RebootTest, VerifyBoardResetButtonCycleTarget) {
    sdbusplus::SdBusMock sdbus_mock;
    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    EXPECT_CALL(
        sdbus_mock, sd_bus_message_new_method_call(
            IsNull(), _, SYSTEMD_SERVICE, SYSTEMD_ROOT, SYSTEMD_INTERFACE, "StartUnit"));

    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', RESET_BUTTON_TARGET));
    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', "replace"));

    EXPECT_CALL(sdbus_mock, sd_bus_call(IsNull(), _, _, IsNull(), IsNull())).Times(1);

    BoardResetButtonCycle(bus_mock);
}
