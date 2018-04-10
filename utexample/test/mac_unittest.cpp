#include "mac.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstring>
#include <sdbusplus/test/bus_mock.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <sdbusplus/message.hpp>

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
    sdbusplus::bus::BusMock bus_mock;
    EXPECT_FALSE(GetMacAddr(nullptr, interface, bus_mock));
}

TEST(NemoraTest, DbusCallErrors) {
    std::string interface = "eth1";
    sdbusplus::bus::BusMock bus_mock;
    auto sdbus_mock = std::make_unique<sdbusplus::SdBusMock>();
    auto sdbus_mock2 = std::make_unique<sdbusplus::SdBusMock>();
    MacAddr mac;

    // This is called, and it calls the right constructor, however, the destructor fires twice...
    EXPECT_CALL(
        bus_mock, new_method_call(
            NETWORK_INTERFACE, interface.c_str(), PROP_INTERFACE, "Get"))
        .WillOnce(
            Invoke([&](const char*, const char*, const char*, const char*) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock));
            })
        );

    // This is called by the invoke'd constructor above.
    EXPECT_CALL(*sdbus_mock, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p; // since p is nullptr, we could just .WillOnce(Return(nullptr));
            })
        );

    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', MAC_INTERFACE));
    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', "MACAddress"));

    EXPECT_CALL(*sdbus_mock2, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p; // since p is nullptr, we could just .WillOnce(Return(nullptr));
            })
        );

    EXPECT_CALL(bus_mock, call(_, _))
        .WillOnce(
            Invoke([&](sdbusplus::message::message&, uint64_t) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock2));
            })
        );

    EXPECT_CALL(*sdbus_mock2, sd_bus_message_is_method_error(_, nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message *m, const char *p) {
                return 1;  // Return true, there was an error.
            })
        );

    EXPECT_FALSE(GetMacAddr(&mac, interface, bus_mock));
}

TEST(NemoraTest, DbusCallInvalidMac) {
    std::string interface = "eth1";
    sdbusplus::bus::BusMock bus_mock;
    auto sdbus_mock = std::make_unique<sdbusplus::SdBusMock>();
    auto sdbus_mock2 = std::make_unique<sdbusplus::SdBusMock>();
    MacAddr mac;

    // This is called, and it calls the right constructor, however, the destructor fires twice...
    EXPECT_CALL(
        bus_mock, new_method_call(
            NETWORK_INTERFACE, interface.c_str(), PROP_INTERFACE, "Get"))
        .WillOnce(
            Invoke([&](const char*, const char*, const char*, const char*) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock));
            })
        );

    // This is called by the invoke'd constructor above.
    EXPECT_CALL(*sdbus_mock, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p; // since p is nullptr, we could just .WillOnce(Return(nullptr));
            })
        );

    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', MAC_INTERFACE));
    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', "MACAddress"));

    EXPECT_CALL(*sdbus_mock2, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p; // since p is nullptr, we could just .WillOnce(Return(nullptr));
            })
        );

    EXPECT_CALL(bus_mock, call(_, _))
        .WillOnce(
            Invoke([&](sdbusplus::message::message&, uint64_t) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock2));
            })
        );

    EXPECT_CALL(*sdbus_mock2, sd_bus_message_is_method_error(_, nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message *m, const char *p) {
                return 0;  // Return false there was no error.
            })
        );

    const char *invalidMac = "123:134:8945:12";

    // Verify that it accepts read through a variant<std::string>
    // Above I didn't need to use StrEq, but here I do...
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_verify_type(IsNull(), 'v', StrEq("s"))).WillOnce(Return(1));
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_enter_container(IsNull(), 'v', StrEq("s"))).WillOnce(Return(0));
    // https://www.freedesktop.org/software/systemd/man/sd_bus_message_read_basic.html
    // the pointer is only borrowed and the contents must be copied if they are to be used after the end of the messages lifetime.
    // https://gbmc-internal.git.corp.google.com/gbmc-sdbusplus/+/master/sdbusplus/message/read.hpp#146
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_read_basic(IsNull(), 's', NotNull()))
        .WillOnce(
            Invoke([&](sd_bus_message *m, char type, void *p) {
                const char **s = static_cast<const char **>(p);
                *s = invalidMac;
                return 0;
            })
        );
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_exit_container(IsNull()));

    EXPECT_FALSE(GetMacAddr(&mac, interface, bus_mock));
}

TEST(NemoraTest, DbusCallAllHappy) {

    std::string interface = "eth1";
    sdbusplus::bus::BusMock bus_mock;
    auto sdbus_mock = std::make_unique<sdbusplus::SdBusMock>();
    auto sdbus_mock2 = std::make_unique<sdbusplus::SdBusMock>();
    MacAddr mac;

    // This is called, and it calls the right constructor, however, the destructor fires twice...
    EXPECT_CALL(
        bus_mock, new_method_call(
            NETWORK_INTERFACE, interface.c_str(), PROP_INTERFACE, "Get"))
        .WillOnce(
            Invoke([&](const char*, const char*, const char*, const char*) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock));
            })
        );

    // This is called by the invoke'd constructor above.
    EXPECT_CALL(*sdbus_mock, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p; // since p is nullptr, we could just .WillOnce(Return(nullptr));
            })
        );

    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', MAC_INTERFACE));
    EXPECT_CALL(*sdbus_mock, sd_bus_message_append_basic(_, 's', "MACAddress"));

    EXPECT_CALL(*sdbus_mock2, sd_bus_message_ref(nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message* p) {
                return p; // since p is nullptr, we could just .WillOnce(Return(nullptr));
            })
        );

    EXPECT_CALL(bus_mock, call(_, _))
        .WillOnce(
            Invoke([&](sdbusplus::message::message&, uint64_t) {
                return sdbusplus::message::message(nullptr, std::move(sdbus_mock2));
            })
        );

    EXPECT_CALL(*sdbus_mock2, sd_bus_message_is_method_error(_, nullptr))
        .WillOnce(
            Invoke([&](sd_bus_message *m, const char *p) {
                return 0;  // Return false there was no error.
            })
        );

    const char *validMac = "03:34:89:12:a4:09";

    // Verify that it accepts read through a variant<std::string>
    // Above I didn't need to use StrEq, but here I do...
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_verify_type(IsNull(), 'v', StrEq("s"))).WillOnce(Return(1));
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_enter_container(IsNull(), 'v', StrEq("s"))).WillOnce(Return(0));
    // https://www.freedesktop.org/software/systemd/man/sd_bus_message_read_basic.html
    // the pointer is only borrowed and the contents must be copied if they are to be used after the end of the messages lifetime.
    // https://gbmc-internal.git.corp.google.com/gbmc-sdbusplus/+/master/sdbusplus/message/read.hpp#146
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_read_basic(IsNull(), 's', NotNull()))
        .WillOnce(
            Invoke([&](sd_bus_message *m, char type, void *p) {
                const char **s = static_cast<const char **>(p);
                *s = validMac;
                return 0;
            })
        );
    EXPECT_CALL(*sdbus_mock2, sd_bus_message_exit_container(IsNull()));

    EXPECT_TRUE(GetMacAddr(&mac, interface, bus_mock));
    // Done as a strict memory comparison, therefore the mac address can't
    // change from formatting.  Keeping in mind that this code is testing the
    // dbus call and handling and not ParseMac().
    MacAddr expected{ .octet = {0x03, 0x34, 0x89, 0x12, 0xa4, 0x09}};
    EXPECT_EQ(0, std::memcmp(&expected, &mac, sizeof(mac)));
}
