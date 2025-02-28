# System states
#   state can change to next state in 2 ways:
#   - a process emits a GotoSystemState signal with state name to goto
#   - objects specified in EXIT_STATE_DEPEND have started
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
    "<inventory_root>/system/chassis/motherboard/dimm16": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm17": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm18": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm19": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm20": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm21": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm22": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm23": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm24": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm25": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm26": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm27": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm28": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm29": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm30": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
    "<inventory_root>/system/chassis/motherboard/dimm31": {
        "fru_type": "DIMM",
        "is_fru": True,
    },
}

ID_LOOKUP = {
    "FRU": {
        0x01: "<inventory_root>/system/chassis/motherboard/cpu0",
        0x02: "<inventory_root>/system/chassis/motherboard/cpu1",
        0x03: "<inventory_root>/system/chassis/motherboard",
        0x04: "<inventory_root>/system/chassis/motherboard/membuf0",
        0x05: "<inventory_root>/system/chassis/motherboard/membuf1",
        0x06: "<inventory_root>/system/chassis/motherboard/membuf2",
        0x07: "<inventory_root>/system/chassis/motherboard/membuf3",
        0x08: "<inventory_root>/system/chassis/motherboard/membuf4",
        0x09: "<inventory_root>/system/chassis/motherboard/membuf5",
        0x0C: "<inventory_root>/system/chassis/motherboard/dimm0",
        0x0D: "<inventory_root>/system/chassis/motherboard/dimm1",
        0x0E: "<inventory_root>/system/chassis/motherboard/dimm2",
        0x0F: "<inventory_root>/system/chassis/motherboard/dimm3",
        0x10: "<inventory_root>/system/chassis/motherboard/dimm4",
        0x11: "<inventory_root>/system/chassis/motherboard/dimm5",
        0x12: "<inventory_root>/system/chassis/motherboard/dimm6",
        0x13: "<inventory_root>/system/chassis/motherboard/dimm7",
        0x14: "<inventory_root>/system/chassis/motherboard/dimm8",
        0x15: "<inventory_root>/system/chassis/motherboard/dimm9",
        0x16: "<inventory_root>/system/chassis/motherboard/dimm10",
        0x17: "<inventory_root>/system/chassis/motherboard/dimm11",
        0x18: "<inventory_root>/system/chassis/motherboard/dimm12",
        0x19: "<inventory_root>/system/chassis/motherboard/dimm13",
        0x1A: "<inventory_root>/system/chassis/motherboard/dimm14",
        0x1B: "<inventory_root>/system/chassis/motherboard/dimm15",
        0x1C: "<inventory_root>/system/chassis/motherboard/dimm16",
        0x1D: "<inventory_root>/system/chassis/motherboard/dimm17",
        0x1E: "<inventory_root>/system/chassis/motherboard/dimm18",
        0x1F: "<inventory_root>/system/chassis/motherboard/dimm19",
        0x20: "<inventory_root>/system/chassis/motherboard/dimm20",
        0x21: "<inventory_root>/system/chassis/motherboard/dimm21",
        0x22: "<inventory_root>/system/chassis/motherboard/dimm22",
        0x23: "<inventory_root>/system/chassis/motherboard/dimm23",
        0x24: "<inventory_root>/system/chassis/motherboard/dimm24",
        0x25: "<inventory_root>/system/chassis/motherboard/dimm25",
        0x26: "<inventory_root>/system/chassis/motherboard/dimm26",
        0x27: "<inventory_root>/system/chassis/motherboard/dimm27",
        0x28: "<inventory_root>/system/chassis/motherboard/dimm28",
        0x29: "<inventory_root>/system/chassis/motherboard/dimm29",
        0x2A: "<inventory_root>/system/chassis/motherboard/dimm30",
        0x2B: "<inventory_root>/system/chassis/motherboard/dimm31",
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
        "PRODUCT_28": "<inventory_root>/system/chassis/motherboard/dimm16",
        "PRODUCT_29": "<inventory_root>/system/chassis/motherboard/dimm17",
        "PRODUCT_30": "<inventory_root>/system/chassis/motherboard/dimm18",
        "PRODUCT_31": "<inventory_root>/system/chassis/motherboard/dimm19",
        "PRODUCT_32": "<inventory_root>/system/chassis/motherboard/dimm20",
        "PRODUCT_33": "<inventory_root>/system/chassis/motherboard/dimm21",
        "PRODUCT_34": "<inventory_root>/system/chassis/motherboard/dimm22",
        "PRODUCT_35": "<inventory_root>/system/chassis/motherboard/dimm23",
        "PRODUCT_36": "<inventory_root>/system/chassis/motherboard/dimm24",
        "PRODUCT_37": "<inventory_root>/system/chassis/motherboard/dimm25",
        "PRODUCT_38": "<inventory_root>/system/chassis/motherboard/dimm26",
        "PRODUCT_39": "<inventory_root>/system/chassis/motherboard/dimm27",
        "PRODUCT_40": "<inventory_root>/system/chassis/motherboard/dimm28",
        "PRODUCT_41": "<inventory_root>/system/chassis/motherboard/dimm29",
        "PRODUCT_42": "<inventory_root>/system/chassis/motherboard/dimm30",
        "PRODUCT_43": "<inventory_root>/system/chassis/motherboard/dimm31",
        "PRODUCT_47": "<inventory_root>/system/misc",
    },
    "SENSOR": {
        0x02: "/org/openbmc/sensors/host/HostStatus",
        0x03: "/org/openbmc/sensors/host/BootProgress",
        0x21: "<inventory_root>/system/chassis/motherboard/cpu0",
        0x71: "<inventory_root>/system/chassis/motherboard/cpu1",
        0xC7: "<inventory_root>/system/chassis/motherboard/dimm3",
        0xC5: "<inventory_root>/system/chassis/motherboard/dimm2",
        0xC3: "<inventory_root>/system/chassis/motherboard/dimm1",
        0xC1: "<inventory_root>/system/chassis/motherboard/dimm0",
        0xCF: "<inventory_root>/system/chassis/motherboard/dimm7",
        0xCD: "<inventory_root>/system/chassis/motherboard/dimm6",
        0xCB: "<inventory_root>/system/chassis/motherboard/dimm5",
        0xC9: "<inventory_root>/system/chassis/motherboard/dimm4",
        0xD7: "<inventory_root>/system/chassis/motherboard/dimm11",
        0xD5: "<inventory_root>/system/chassis/motherboard/dimm10",
        0xD3: "<inventory_root>/system/chassis/motherboard/dimm9",
        0xD1: "<inventory_root>/system/chassis/motherboard/dimm8",
        0xDF: "<inventory_root>/system/chassis/motherboard/dimm15",
        0xDD: "<inventory_root>/system/chassis/motherboard/dimm14",
        0xDB: "<inventory_root>/system/chassis/motherboard/dimm13",
        0xD9: "<inventory_root>/system/chassis/motherboard/dimm12",
        0xE7: "<inventory_root>/system/chassis/motherboard/dimm19",
        0xE5: "<inventory_root>/system/chassis/motherboard/dimm18",
        0xE3: "<inventory_root>/system/chassis/motherboard/dimm17",
        0xE1: "<inventory_root>/system/chassis/motherboard/dimm16",
        0xEF: "<inventory_root>/system/chassis/motherboard/dimm23",
        0xED: "<inventory_root>/system/chassis/motherboard/dimm22",
        0xEB: "<inventory_root>/system/chassis/motherboard/dimm21",
        0xE9: "<inventory_root>/system/chassis/motherboard/dimm20",
        0xF7: "<inventory_root>/system/chassis/motherboard/dimm27",
        0xF5: "<inventory_root>/system/chassis/motherboard/dimm26",
        0xF3: "<inventory_root>/system/chassis/motherboard/dimm25",
        0xF1: "<inventory_root>/system/chassis/motherboard/dimm24",
        0xFF: "<inventory_root>/system/chassis/motherboard/dimm31",
        0xFD: "<inventory_root>/system/chassis/motherboard/dimm30",
        0xFB: "<inventory_root>/system/chassis/motherboard/dimm29",
        0xF9: "<inventory_root>/system/chassis/motherboard/dimm28",
        0x23: "<inventory_root>/system/chassis/motherboard/cpu0/core0",
        0x26: "<inventory_root>/system/chassis/motherboard/cpu0/core1",
        0x29: "<inventory_root>/system/chassis/motherboard/cpu0/core2",
        0x2C: "<inventory_root>/system/chassis/motherboard/cpu0/core3",
        0x2F: "<inventory_root>/system/chassis/motherboard/cpu0/core4",
        0x32: "<inventory_root>/system/chassis/motherboard/cpu0/core5",
        0x35: "<inventory_root>/system/chassis/motherboard/cpu0/core6",
        0x38: "<inventory_root>/system/chassis/motherboard/cpu0/core7",
        0x3B: "<inventory_root>/system/chassis/motherboard/cpu0/core8",
        0x3E: "<inventory_root>/system/chassis/motherboard/cpu0/core9",
        0x41: "<inventory_root>/system/chassis/motherboard/cpu0/core10",
        0x44: "<inventory_root>/system/chassis/motherboard/cpu0/core11",
        0x47: "<inventory_root>/system/chassis/motherboard/cpu0/core12",
        0x4A: "<inventory_root>/system/chassis/motherboard/cpu0/core13",
        0x4D: "<inventory_root>/system/chassis/motherboard/cpu0/core14",
        0x50: "<inventory_root>/system/chassis/motherboard/cpu0/core15",
        0x53: "<inventory_root>/system/chassis/motherboard/cpu0/core16",
        0x56: "<inventory_root>/system/chassis/motherboard/cpu0/core17",
        0x59: "<inventory_root>/system/chassis/motherboard/cpu0/core18",
        0x5C: "<inventory_root>/system/chassis/motherboard/cpu0/core19",
        0x5F: "<inventory_root>/system/chassis/motherboard/cpu0/core20",
        0x62: "<inventory_root>/system/chassis/motherboard/cpu0/core21",
        0x65: "<inventory_root>/system/chassis/motherboard/cpu1/core22",
        0x68: "<inventory_root>/system/chassis/motherboard/cpu1/core23",
        0x73: "<inventory_root>/system/chassis/motherboard/cpu1/core0",
        0x76: "<inventory_root>/system/chassis/motherboard/cpu1/core1",
        0x79: "<inventory_root>/system/chassis/motherboard/cpu1/core2",
        0x7C: "<inventory_root>/system/chassis/motherboard/cpu1/core3",
        0x7F: "<inventory_root>/system/chassis/motherboard/cpu1/core4",
        0x82: "<inventory_root>/system/chassis/motherboard/cpu1/core5",
        0x85: "<inventory_root>/system/chassis/motherboard/cpu1/core6",
        0x88: "<inventory_root>/system/chassis/motherboard/cpu1/core7",
        0x8B: "<inventory_root>/system/chassis/motherboard/cpu1/core8",
        0x8E: "<inventory_root>/system/chassis/motherboard/cpu1/core9",
        0x91: "<inventory_root>/system/chassis/motherboard/cpu1/core10",
        0x94: "<inventory_root>/system/chassis/motherboard/cpu1/core11",
        0x97: "<inventory_root>/system/chassis/motherboard/cpu1/core12",
        0x9A: "<inventory_root>/system/chassis/motherboard/cpu1/core13",
        0x9D: "<inventory_root>/system/chassis/motherboard/cpu1/core14",
        0xA0: "<inventory_root>/system/chassis/motherboard/cpu1/core15",
        0xA3: "<inventory_root>/system/chassis/motherboard/cpu1/core16",
        0xA6: "<inventory_root>/system/chassis/motherboard/cpu1/core17",
        0xA9: "<inventory_root>/system/chassis/motherboard/cpu1/core18",
        0xAC: "<inventory_root>/system/chassis/motherboard/cpu1/core19",
        0xAF: "<inventory_root>/system/chassis/motherboard/cpu1/core20",
        0xB2: "<inventory_root>/system/chassis/motherboard/cpu1/core21",
        0xB5: "<inventory_root>/system/chassis/motherboard/cpu1/core22",
        0xB8: "<inventory_root>/system/chassis/motherboard/cpu1/core23",
        0x07: "/org/openbmc/sensors/host/BootCount",
        0x10: "<inventory_root>/system/chassis/motherboard",
        0x01: "<inventory_root>/system/systemevent",
        0x11: "<inventory_root>/system/chassis/motherboard/refclock",
        0x12: "<inventory_root>/system/chassis/motherboard/pcieclock",
        0x13: "<inventory_root>/system/chassis/motherboard/todclock",
        0x02: "/org/openbmc/sensors/host/OperatingSystemStatus",
        0x04: "<inventory_root>/system/chassis/motherboard/pcielink",
    },
    "GPIO_PRESENT": {},
}

# Miscellaneous non-poll sensor with system specific properties.
# The sensor id is the same as those defined in ID_LOOKUP['SENSOR'].
MISC_SENSORS = {}
