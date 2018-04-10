#include "mac.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>

using ::testing::_;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::StrEq;

constexpr auto MAC_FORMAT = "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx";
constexpr auto MAC_INTERFACE = "xyz.openbmc_project.Network.MACAddress";
constexpr auto NETWORK_INTERFACE = "xyz.openbmc_project.Network";
constexpr auto PROP_INTERFACE = "org.freedesktop.DBus.Properties";
constexpr auto IFACE_ROOT = "/xyz/openbmc_project/network/";

TEST(NemoraTest, NullPointerCheck) {
    std::string interface = "eth1";
    sdbusplus::SdBusMock sdbus_mock;

    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    EXPECT_FALSE(GetMacAddr(nullptr, interface, bus_mock));
}

TEST(NemoraTest, DbusCallErrors) {
    std::string interface = "eth1";
    sdbusplus::SdBusMock sdbus_mock;
    MacAddr mac;

    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    // This is called, and it calls the right constructor
    EXPECT_CALL(sdbus_mock,
        sd_bus_message_new_method_call(
            IsNull(), _, NETWORK_INTERFACE, interface.c_str(), PROP_INTERFACE, "Get"));

    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', MAC_INTERFACE));
    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', "MACAddress"));

    EXPECT_CALL(sdbus_mock, sd_bus_call(_, _, _, IsNull(), _));

    EXPECT_CALL(sdbus_mock, sd_bus_message_is_method_error(_, nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message *m, const char *p) {
                return 1;  // Return true, there was an error.
            })
        );

    EXPECT_FALSE(GetMacAddr(&mac, interface, bus_mock));
}

TEST(NemoraTest, DbusCallInvalidMac) {
    std::string interface = "eth1";
    sdbusplus::SdBusMock sdbus_mock;
    MacAddr mac;

    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    // This is called, and it calls the right constructor
    EXPECT_CALL(
        sdbus_mock, sd_bus_message_new_method_call(
            IsNull(), _, NETWORK_INTERFACE, interface.c_str(), PROP_INTERFACE, "Get"));

    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', MAC_INTERFACE));
    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', "MACAddress"));

    EXPECT_CALL(sdbus_mock, sd_bus_call(IsNull(), _, _, IsNull(), NotNull()));

    EXPECT_CALL(sdbus_mock, sd_bus_message_is_method_error(_, IsNull()))
        .WillOnce(
            Invoke([&](sd_bus_message *m, const char *p) {
                return 0;  // Return false there was no error.
            })
        );

    const char *invalidMac = "123:134:8945:12";

    // Verify that it accepts read through a variant<std::string>
    // Above I didn't need to use StrEq, but here I do...
    EXPECT_CALL(sdbus_mock, sd_bus_message_verify_type(IsNull(), 'v', StrEq("s"))).WillOnce(Return(1));
    EXPECT_CALL(sdbus_mock, sd_bus_message_enter_container(IsNull(), 'v', StrEq("s"))).WillOnce(Return(0));
    // https://www.freedesktop.org/software/systemd/man/sd_bus_message_read_basic.html
    // the pointer is only borrowed and the contents must be copied if they are to be used after the end of the messages lifetime.
    // https://gbmc-internal.git.corp.google.com/gbmc-sdbusplus/+/master/sdbusplus/message/read.hpp#146
    EXPECT_CALL(sdbus_mock, sd_bus_message_read_basic(IsNull(), 's', NotNull()))
        .WillOnce(
            Invoke([&](sd_bus_message *m, char type, void *p) {
                const char **s = static_cast<const char **>(p);
                *s = invalidMac;
                return 0;
            })
        );
    EXPECT_CALL(sdbus_mock, sd_bus_message_exit_container(IsNull()));

    EXPECT_FALSE(GetMacAddr(&mac, interface, bus_mock));
}

TEST(NemoraTest, DbusCallAllHappy) {
    std::string interface = "eth1";
    sdbusplus::SdBusMock sdbus_mock;
    MacAddr mac;

    auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

    EXPECT_CALL(
        sdbus_mock, sd_bus_message_new_method_call(
            IsNull(), _, NETWORK_INTERFACE, interface.c_str(), PROP_INTERFACE, "Get"));

    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', MAC_INTERFACE));
    EXPECT_CALL(sdbus_mock, sd_bus_message_append_basic(_, 's', "MACAddress"));

    EXPECT_CALL(sdbus_mock, sd_bus_call(IsNull(), _, _, IsNull(), NotNull()));

    EXPECT_CALL(sdbus_mock, sd_bus_message_is_method_error(_, nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message *m, const char *p) {
                return 0;  // Return false there was no error.
            })
        );

    const char *validMac = "03:34:89:12:a4:09";

    // Verify that it accepts read through a variant<std::string>
    // Above I didn't need to use StrEq, but here I do...
    EXPECT_CALL(sdbus_mock, sd_bus_message_verify_type(IsNull(), 'v', StrEq("s"))).WillOnce(Return(1));
    EXPECT_CALL(sdbus_mock, sd_bus_message_enter_container(IsNull(), 'v', StrEq("s"))).WillOnce(Return(0));
    // https://www.freedesktop.org/software/systemd/man/sd_bus_message_read_basic.html
    // the pointer is only borrowed and the contents must be copied if they are to be used after the end of the messages lifetime.
    // https://gbmc-internal.git.corp.google.com/gbmc-sdbusplus/+/master/sdbusplus/message/read.hpp#146
    EXPECT_CALL(sdbus_mock, sd_bus_message_read_basic(IsNull(), 's', NotNull()))
        .WillOnce(
            Invoke([&](sd_bus_message *m, char type, void *p) {
                const char **s = static_cast<const char **>(p);
                *s = validMac;
                return 0;
            })
        );
    EXPECT_CALL(sdbus_mock, sd_bus_message_exit_container(IsNull()));

    EXPECT_TRUE(GetMacAddr(&mac, interface, bus_mock));
    // Done as a strict memory comparison, therefore the mac address can't
    // change from formatting.  Keeping in mind that this code is testing the
    // dbus call and handling and not ParseMac().
    MacAddr expected{ .octet = {0x03, 0x34, 0x89, 0x12, 0xa4, 0x09}};
    EXPECT_EQ(0, std::memcmp(&expected, &mac, sizeof(mac)));
}
