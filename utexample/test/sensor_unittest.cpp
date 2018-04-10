#include "sensor.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <sdbusplus/bus.hpp>
#include <sdbusplus/message.hpp>
#include <sdbusplus/test/sdbus_mock.hpp>
#include <string>

using ::testing::_;
using ::testing::Invoke;
using ::testing::IsNull;
using ::testing::Return;
using ::testing::StrEq;

static constexpr auto FANSPEED_INTF = "xyz.openbmc_project.Control.FanSpeed";
static constexpr auto SENSORVALUE_INTF = "xyz.openbmc_project.Sensor.Value";

TEST(SensorTest, FanSpeedRead) {
  // We only override the setting of the fanspeed, not the reading, but this
  // should be something we test.
  sdbusplus::SdBusMock sdbus_mock;
  auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

  std::string path = "the/path";
  std::string id = "the_fan_id";
  const char *obj = "/xyz/openbmc_project/sensors/fan_tach/fan1";
  bool deferSignals = true;

  // I expect this to be the same for anyone composing this object, or similar
  // objects, which means it can be moved into a help method.

  // sd_bus_ref called twice more it's captured in the object and then the interface.
  EXPECT_CALL(sdbus_mock, sd_bus_ref(IsNull())).Times(2).WillRepeatedly(Return(nullptr));
  // FanSpeedObject(bus, objPath, defer)
  //
  // This is in the interface constructor
  // sd_bus_add_object_vtable(
  //   NULL, <-- bus
  //   0x7ffc7008cfb0, <-- slot
  //   0x1edbfb0 pointing to "/xyz/openbmc_project/sensors/fan_tach/fan1",
  //   0x1edbff0 pointing to "xyz.openbmc_project.Control.FanSpeed",
  //   0x7f58ac971800, <-- vtable
  //   0x1edc250) <-- context
  EXPECT_CALL(sdbus_mock,
              sd_bus_add_object_vtable(IsNull(), _, StrEq(obj), StrEq(FANSPEED_INTF), _, _)).WillOnce(Return(0));

  auto fanspeed = std::make_unique<FanSpeed>(path, id, bus_mock, obj, deferSignals, 0);
  EXPECT_FALSE(fanspeed == nullptr);

  // The value 0 isn't passed in, it's just default, so worth noting that for
  // phosphor-hwmon, you need to read the value first, which it does now.
  EXPECT_EQ(0, fanspeed->target());
}

TEST(SensorTest, FanSpeedWrite) {
  // We don't do anything upon writing it, but we can and we can inject an
  // IoAccess interface mock to handle this in the real phosphor-hwmon.

  sdbusplus::SdBusMock sdbus_mock;
  auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);

  std::string path = "the/path";
  std::string id = "the_fan_id";
  const char *obj = "/xyz/openbmc_project/sensors/fan_tach/fan1";
  bool deferSignals = true;

  // This stuff is common between all creating objects...
  // sd_bus_ref called twice more it's captured in the object and then the interface.
  EXPECT_CALL(sdbus_mock, sd_bus_ref(IsNull())).Times(2).WillRepeatedly(Return(nullptr));
  // FanSpeedObject(bus, objPath, defer)
  //
  // This is in the interface constructor
  // sd_bus_add_object_vtable(
  //   NULL, <-- bus
  //   0x7ffc7008cfb0, <-- slot
  //   0x1edbfb0 pointing to "/xyz/openbmc_project/sensors/fan_tach/fan1",
  //   0x1edbff0 pointing to "xyz.openbmc_project.Control.FanSpeed",
  //   0x7f58ac971800, <-- vtable
  //   0x1edc250) <-- context
  EXPECT_CALL(sdbus_mock,
              sd_bus_add_object_vtable(IsNull(), _, StrEq(obj), StrEq(FANSPEED_INTF), _, _)).WillOnce(Return(0));

  auto fanspeed = std::make_unique<FanSpeed>(path, id, bus_mock, obj, deferSignals, 0);
  EXPECT_FALSE(fanspeed == nullptr);

  // The value 0 isn't passed in, it's just default, so worth noting that for
  // phosphor-hwmon, you need to read the value first, which it does now.
  EXPECT_EQ(0, fanspeed->target());

  uint64_t value = 123;

  // The dbus interface servers are only passing the property into that last
  // arg.
  //  sd_bus_emit_properties_changed_strv(
  //    NULL, <-- bus
  //    0x2302340 pointing to "/xyz/openbmc_project/sensors/fan_tach/fan1",
  //    0x2302270 pointing to "xyz.openbmc_project.Control.FanSpeed",
  //    0x2302200) <-- const char **{property}
  EXPECT_CALL(sdbus_mock,
              sd_bus_emit_properties_changed_strv(IsNull(), StrEq(obj), StrEq(FANSPEED_INTF), _))
      .WillOnce(
          Invoke([&](sd_bus *bus, const char *path, const char *interface, char **names) {
              EXPECT_THAT(names[0], StrEq("Target"));
              EXPECT_EQ(0, names[1]); // Expect a null terminated list of pointers.
              return 0; // no error
          })
      );

  fanspeed->target(value);

  // Reading the value doesn't touch dbus.
  EXPECT_EQ(value, fanspeed->target());
}

TEST(SensorTest, SensorValueUpdate) {
  // Just set the value and make sure the sd-bus calls are made, which is
  // basically identical to the fanspeedwrite but this should do slightly less.

  sdbusplus::SdBusMock sdbus_mock;
  auto bus_mock = sdbusplus::get_mocked_new(&sdbus_mock);
  EXPECT_CALL(sdbus_mock, sd_bus_ref(IsNull())).Times(2).WillRepeatedly(Return(nullptr));
  const char *obj = "/xyz/openbmc_project/sensors/fan_tach/fan1";
  bool deferSignals = true;

  EXPECT_CALL(sdbus_mock,
              sd_bus_add_object_vtable(IsNull(), _, StrEq(obj), StrEq(SENSORVALUE_INTF), _, _)).WillOnce(Return(0));

  auto fansensor = std::make_unique<ValueObject>(bus_mock, obj, deferSignals);

  int64_t value = 23452345;

  EXPECT_CALL(sdbus_mock,
              sd_bus_emit_properties_changed_strv(IsNull(), StrEq(obj), StrEq(SENSORVALUE_INTF), _))
      .WillOnce(
          Invoke([&](sd_bus *bus, const char *path, const char *interface, char **names) {
              EXPECT_THAT(names[0], StrEq("Value"));
              EXPECT_EQ(0, names[1]); // Expect a null terminated list of pointers.
              return 0; // no error
          })
      );

  fansensor->value(value);
}
