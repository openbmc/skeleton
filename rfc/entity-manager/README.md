#Entity Manager#

Entity manager is a runtime configuration application that based on system components generates a system JSON description. This JSON system configuration can then be parsed by applications for initialization data. It also can, based on the configuration, attempt to load device tree overlays to add sensors to the device tree.

## Configuration Syntax ##

Configuration JSON is meant to be highly configurable, such that individual applications can define specialized fields per the application need. The JSON is described in entities that describe pieces of hardware. In general it is preferred to define files as the smallest components that are available, as it makes platform ports less costly. However this is at the vendors discretion and a single file configuration is legal.

*Clarification point: FRU in this file refers to the actual EEPROM file contents, not a piece of hardware.*

### Keywords ###

* "name" - Field used for identification and sorting.

* "probe" - A probe is an action to determine if a configuration record should be applied. Probe can be set to “true” in the case where the record should always be applied, or set to more complex lookups, for instance a field in a FRU file. Probes may also be that another entity was added.

* "exposes" - Records that get applied when a probe passes, for instance a sensor.

* "status" - An indicator that allows for some records to be disabled by default. This can be useful if one entity "updates" another.

* "update" - A reference to another field to update its contents with the data in this record using a dictionary style update.

* "bind_*" - A reference to attach this record to another record, using the name following the underscore.

Templates may also be used such as $bus and $index when using the fru device to automatically fill in device information.

Required fields are name, probe and exposes.

## Configuration Records##

Configuration records are composed of one or more entities that can define a hardware component. One configuration file can be used to describe an entire platform, or they can define a piece of removable hardware. Entities are added to the system configuration when a probe passes. Below is a simplified baseboard.

```
{
        "exposes": [
            {
                "name": "1U System Fan connector 1",
                "pwm": 1,
                "status": "disabled",
                "tachs": [
                    1,
                    2
                ],
                "type": "IntelFanConnector"
            },
            {
                "name": "2U System Fan connector 1",
                "pwm": 1,
                "status": "disabled",
                "tachs": [
                    1
                ],
                "type": "IntelFanConnector"
            },
            {
                "address": "0x49",
                "bus": 6,
                "name": "Left Rear Temp",
                "type": "TMP75"
            },
            {
                "address": "0x48",
                "bus": 6,
                "name": "Voltage Regulator 1 Temp",
                "type": "TMP75"
            }
        ],
        "name": "WFP Baseboard",
        "probe": "fru.probe('BOARD_PRODUCT_NAME': '.*WFT')"
}
```

####Example Baseboard ####

This baseboard entity describes two TMP75 sensors, and two fan connectors of different types. When a FRU is found with the appropriate product name, this record is added to the system JSON.

```
{
        "exposes": [
            {
                "bind_connector": "1U System Fan connector 1",
                "name": "Fan 1",
                "type": "AspeedFan"
            }
        ],
        "name": "R1000 Chassis",
        "probe": "'WFP Baseboard' in found_devices and fru.probe('BOARD_PRODUCT_NAME': 'F1UL16RISER\\d')"
}
```

####Example Fans####

A separate probe could then expose the fans. As these use the bind keyword this would get inserted into the fan connector record, so that the system JSON would show the following:

```
{
        "connector": {
            "name": "1U System Fan connector 1",
            "pwm": 1,
            "status": "okay",
            "tachs": [
                1,
                2
            ],
            "type": "IntelFanConnector"
        },
        "name": "Fan 1",
        "status": "okay",
        "type": "AspeedFan"
}
```

## Enabling Sensors ##

As demons can trigger off of shared types, sometimes some handshaking will be needed to enable sensors. From Example 1, we can see the definition of a TMP75 sensor. When the entity is enabled, the device tree must be updated before scanning may begin. The device tree overlay generator has the ability to key off of different types and create device tree overlays for specific offsets. Once this is done, the baseboard temperature sensor demon can scan the sensors.

## Run Unit Tests ##

The following environment variables need to be set to run unit tests:

* TEST: 1, this disables the fru parser from scanning on init and changes the work directories.
