# Romulus.py
#

SYSTEM_STATES = [
    "BASE_APPS",
    "BMC_STARTING",
    "BMC_READY",
    "HOST_POWERING_ON",
    "HOST_POWERED_ON",
    "HOST_BOOTING",
    "HOST_BOOTED",
    "HOST_POWERED_OFF",
]

EXIT_STATE_DEPEND = {
    "BASE_APPS": {
        "/org/openbmc/sensors": 0,
    },
    "BMC_STARTING": {
        "/org/openbmc/control/chassis0": 0,
        "/org/openbmc/control/power0": 0,
        "/org/openbmc/control/flash/bios": 0,
    },
}

FRU_INSTANCES = {
    "<inventory_root>/system": {
        "fru_type": "SYSTEM",
        "is_fru": True,
        "present": "True",
    },
    "<inventory_root>/system/bios": {
        "fru_type": "SYSTEM",
        "is_fru": True,
        "present": "True",
    },
    "<inventory_root>/system/misc": {
        "fru_type": "SYSTEM",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis": {
        "fru_type": "SYSTEM",
        "is_fru": True,
        "present": "True",
    },
    "<inventory_root>/system/chassis/motherboard": {
        "fru_type": "MAIN_PLANAR",
        "is_fru": True,
    },
    "<inventory_root>/system/systemevent": {
        "fru_type": "SYSTEM_EVENT",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/refclock": {
        "fru_type": "MAIN_PLANAR",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/pcieclock": {
        "fru_type": "MAIN_PLANAR",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/todclock": {
        "fru_type": "MAIN_PLANAR",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/apss": {
        "fru_type": "MAIN_PLANAR",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/fan0": {
        "fru_type": "FAN",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/fan1": {
        "fru_type": "FAN",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/fan2": {
        "fru_type": "FAN",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/fan3": {
        "fru_type": "FAN",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/bmc": {
        "fru_type": "BMC",
        "is_fru": False,
        "manufacturer": "ASPEED",
    },
    "<inventory_root>/system/chassis/motherboard/cpu0": {
        "fru_type": "CPU",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1": {
        "fru_type": "CPU",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core0": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core1": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core2": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core3": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core4": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core5": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core6": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core7": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core8": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core9": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core10": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core11": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core12": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core13": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core14": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core15": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core16": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core17": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core18": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core19": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core20": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core21": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core22": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu0/core23": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core0": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core1": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core2": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core3": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core4": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core5": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core6": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core7": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core8": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core9": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core10": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core11": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core12": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core13": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core14": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core15": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core16": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core17": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core18": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core19": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core20": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core21": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core22": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/cpu1/core23": {
        "fru_type": "CORE",
        "is_fru": False,
    },
    "<inventory_root>/system/chassis/motherboard/dimm0": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm1": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm2": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm3": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm4": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm5": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm6": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm7": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm8": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm9": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm10": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm11": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm12": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm13": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm14": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm15": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
}

ID_LOOKUP = {
    "FRU": {
        0x01: "<inventory_root>/system/chassis/motherboard/cpu0",
        0x02: "<inventory_root>/system/chassis/motherboard/cpu1",
        0x03: "<inventory_root>/system/chassis/motherboard",
        0x04: "<inventory_root>/system/chassis/motherboard/dimm0",
        0x05: "<inventory_root>/system/chassis/motherboard/dimm1",
        0x06: "<inventory_root>/system/chassis/motherboard/dimm2",
        0x07: "<inventory_root>/system/chassis/motherboard/dimm3",
        0x08: "<inventory_root>/system/chassis/motherboard/dimm4",
        0x09: "<inventory_root>/system/chassis/motherboard/dimm5",
        0x0A: "<inventory_root>/system/chassis/motherboard/dimm6",
        0x0B: "<inventory_root>/system/chassis/motherboard/dimm7",
        0x0C: "<inventory_root>/system/chassis/motherboard/dimm8",
        0x0D: "<inventory_root>/system/chassis/motherboard/dimm9",
        0x0E: "<inventory_root>/system/chassis/motherboard/dimm10",
        0x0F: "<inventory_root>/system/chassis/motherboard/dimm11",
        0x10: "<inventory_root>/system/chassis/motherboard/dimm12",
        0x11: "<inventory_root>/system/chassis/motherboard/dimm13",
        0x12: "<inventory_root>/system/chassis/motherboard/dimm14",
        0x13: "<inventory_root>/system/chassis/motherboard/dimm15",
    },
    "FRU_STR": {
        "PRODUCT_0": "<inventory_root>/system/bios",
        "BOARD_1": "<inventory_root>/system/chassis/motherboard/cpu0",
        "BOARD_2": "<inventory_root>/system/chassis/motherboard/cpu1",
        "CHASSIS_3": "<inventory_root>/system/chassis/motherboard",
        "BOARD_3": "<inventory_root>/system/misc",
        "PRODUCT_12": "<inventory_root>/system/chassis/motherboard/dimm0",
        "PRODUCT_13": "<inventory_root>/system/chassis/motherboard/dimm1",
        "PRODUCT_14": "<inventory_root>/system/chassis/motherboard/dimm2",
        "PRODUCT_15": "<inventory_root>/system/chassis/motherboard/dimm3",
        "PRODUCT_16": "<inventory_root>/system/chassis/motherboard/dimm4",
        "PRODUCT_17": "<inventory_root>/system/chassis/motherboard/dimm5",
        "PRODUCT_18": "<inventory_root>/system/chassis/motherboard/dimm6",
        "PRODUCT_19": "<inventory_root>/system/chassis/motherboard/dimm7",
        "PRODUCT_20": "<inventory_root>/system/chassis/motherboard/dimm8",
        "PRODUCT_21": "<inventory_root>/system/chassis/motherboard/dimm9",
        "PRODUCT_22": "<inventory_root>/system/chassis/motherboard/dimm10",
        "PRODUCT_23": "<inventory_root>/system/chassis/motherboard/dimm11",
        "PRODUCT_24": "<inventory_root>/system/chassis/motherboard/dimm12",
        "PRODUCT_25": "<inventory_root>/system/chassis/motherboard/dimm13",
        "PRODUCT_26": "<inventory_root>/system/chassis/motherboard/dimm14",
        "PRODUCT_27": "<inventory_root>/system/chassis/motherboard/dimm15",
        "PRODUCT_47": "<inventory_root>/system/misc",
    },
    "SENSOR": {
        0x01: "/org/openbmc/sensors/host/HostStatus",
        0x02: "/org/openbmc/sensors/host/BootProgress",
        0x08: "<inventory_root>/system/chassis/motherboard/cpu0",
        0x09: "<inventory_root>/system/chassis/motherboard/cpu1",
        0x0B: "<inventory_root>/system/chassis/motherboard/dimm0",
        0x0C: "<inventory_root>/system/chassis/motherboard/dimm1",
        0x0D: "<inventory_root>/system/chassis/motherboard/dimm2",
        0x0E: "<inventory_root>/system/chassis/motherboard/dimm3",
        0x0F: "<inventory_root>/system/chassis/motherboard/dimm4",
        0x10: "<inventory_root>/system/chassis/motherboard/dimm5",
        0x11: "<inventory_root>/system/chassis/motherboard/dimm6",
        0x12: "<inventory_root>/system/chassis/motherboard/dimm7",
        0x13: "<inventory_root>/system/chassis/motherboard/dimm8",
        0x14: "<inventory_root>/system/chassis/motherboard/dimm9",
        0x15: "<inventory_root>/system/chassis/motherboard/dimm10",
        0x16: "<inventory_root>/system/chassis/motherboard/dimm11",
        0x17: "<inventory_root>/system/chassis/motherboard/dimm12",
        0x18: "<inventory_root>/system/chassis/motherboard/dimm13",
        0x19: "<inventory_root>/system/chassis/motherboard/dimm14",
        0x1A: "<inventory_root>/system/chassis/motherboard/dimm15",
        0x2B: "<inventory_root>/system/chassis/motherboard/cpu0/core0",
        0x2C: "<inventory_root>/system/chassis/motherboard/cpu0/core1",
        0x2D: "<inventory_root>/system/chassis/motherboard/cpu0/core2",
        0x2E: "<inventory_root>/system/chassis/motherboard/cpu0/core3",
        0x2F: "<inventory_root>/system/chassis/motherboard/cpu0/core4",
        0x30: "<inventory_root>/system/chassis/motherboard/cpu0/core5",
        0x31: "<inventory_root>/system/chassis/motherboard/cpu0/core6",
        0x32: "<inventory_root>/system/chassis/motherboard/cpu0/core7",
        0x33: "<inventory_root>/system/chassis/motherboard/cpu0/core8",
        0x34: "<inventory_root>/system/chassis/motherboard/cpu0/core9",
        0x35: "<inventory_root>/system/chassis/motherboard/cpu0/core10",
        0x36: "<inventory_root>/system/chassis/motherboard/cpu0/core11",
        0x37: "<inventory_root>/system/chassis/motherboard/cpu0/core12",
        0x38: "<inventory_root>/system/chassis/motherboard/cpu0/core13",
        0x39: "<inventory_root>/system/chassis/motherboard/cpu0/core14",
        0x3A: "<inventory_root>/system/chassis/motherboard/cpu0/core15",
        0x3B: "<inventory_root>/system/chassis/motherboard/cpu0/core16",
        0x3C: "<inventory_root>/system/chassis/motherboard/cpu0/core17",
        0x3D: "<inventory_root>/system/chassis/motherboard/cpu0/core18",
        0x3E: "<inventory_root>/system/chassis/motherboard/cpu0/core19",
        0x3F: "<inventory_root>/system/chassis/motherboard/cpu0/core20",
        0x40: "<inventory_root>/system/chassis/motherboard/cpu0/core21",
        0x41: "<inventory_root>/system/chassis/motherboard/cpu0/core22",
        0x42: "<inventory_root>/system/chassis/motherboard/cpu0/core23",
        0x43: "<inventory_root>/system/chassis/motherboard/cpu1/core0",
        0x44: "<inventory_root>/system/chassis/motherboard/cpu1/core1",
        0x45: "<inventory_root>/system/chassis/motherboard/cpu1/core2",
        0x46: "<inventory_root>/system/chassis/motherboard/cpu1/core3",
        0x47: "<inventory_root>/system/chassis/motherboard/cpu1/core4",
        0x48: "<inventory_root>/system/chassis/motherboard/cpu1/core5",
        0x49: "<inventory_root>/system/chassis/motherboard/cpu1/core6",
        0x4A: "<inventory_root>/system/chassis/motherboard/cpu1/core7",
        0x4B: "<inventory_root>/system/chassis/motherboard/cpu1/core8",
        0x4C: "<inventory_root>/system/chassis/motherboard/cpu1/core9",
        0x4D: "<inventory_root>/system/chassis/motherboard/cpu1/core10",
        0x4E: "<inventory_root>/system/chassis/motherboard/cpu1/core11",
        0x4F: "<inventory_root>/system/chassis/motherboard/cpu1/core12",
        0x50: "<inventory_root>/system/chassis/motherboard/cpu1/core13",
        0x51: "<inventory_root>/system/chassis/motherboard/cpu1/core14",
        0x52: "<inventory_root>/system/chassis/motherboard/cpu1/core15",
        0x53: "<inventory_root>/system/chassis/motherboard/cpu1/core16",
        0x54: "<inventory_root>/system/chassis/motherboard/cpu1/core17",
        0x55: "<inventory_root>/system/chassis/motherboard/cpu1/core18",
        0x56: "<inventory_root>/system/chassis/motherboard/cpu1/core19",
        0x57: "<inventory_root>/system/chassis/motherboard/cpu1/core20",
        0x58: "<inventory_root>/system/chassis/motherboard/cpu1/core21",
        0x59: "<inventory_root>/system/chassis/motherboard/cpu1/core22",
        0x5A: "<inventory_root>/system/chassis/motherboard/cpu1/core23",
        0x8B: "/org/openbmc/sensors/host/BootCount",
        0x8C: "<inventory_root>/system/chassis/motherboard",
        0x8D: "<inventory_root>/system/chassis/motherboard/refclock",
        0x8E: "<inventory_root>/system/chassis/motherboard/pcieclock",
        0x8F: "<inventory_root>/system/chassis/motherboard/todclock",
        0x90: "<inventory_root>/system/systemevent",
        0x91: "/org/openbmc/sensors/host/OperatingSystemStatus",
        0x92: "<inventory_root>/system/chassis/motherboard/pcielink",
        #        0x08 : '<inventory_root>/system/powerlimit',
        #        0x10 : '<inventory_root>/system/chassis/motherboard/apss',
    },
    "GPIO_PRESENT": {},
}

# Miscellaneous non-poll sensor with system specific properties.
# The sensor id is the same as those defined in ID_LOOKUP['SENSOR'].
MISC_SENSORS = {}
