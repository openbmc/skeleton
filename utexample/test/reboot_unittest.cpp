#include "reboot.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <sdbusplus/message.hpp>
#include <sdbusplus/test/bus_mock.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;

// Since these are referenced in a couple places, we should promote them to a header.
static constexpr auto SYSTEMD_SERVICE    = "org.freedesktop.systemd1";
static constexpr auto SYSTEMD_ROOT       = "/org/freedesktop/systemd1";
static constexpr auto SYSTEMD_INTERFACE  = "org.freedesktop.systemd1.Manager";
static constexpr auto RESET_BUTTON_TARGET    = "reset-button.target";

TEST(RebootTest, VerifyBoardResetButtonCycleTarget) {

    sdbusplus::bus::BusMock bus_mock;

    auto sdbus_mock = std::make_unique<sdbusplus::SdBusMock>();

    // This is called, and it calls the right constructor.
    EXPECT_CALL(
        bus_mock, new_method_call(
            SYSTEMD_SERVICE, SYSTEMD_ROOT, SYSTEMD_INTERFACE, "StartUnit"))
        .WillOnce(
            Invoke([&](const char*, const char*, const char*, const char*) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock));
            })
        );

    // This is called by the invoke'd constructor above.
    EXPECT_CALL(*sdbus_mock, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p;
            })
        );

    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', RESET_BUTTON_TARGET));
    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', "replace"));

    EXPECT_CALL(bus_mock, call_noreply(_, _)).Times(1);

    BoardResetButtonCycle(bus_mock);
}
