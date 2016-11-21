## System states
##   state can change to next state in 2 ways:
##   - a process emits a GotoSystemState signal with state name to goto
##   - objects specified in EXIT_STATE_DEPEND have started
SYSTEM_STATES = [
    'BASE_APPS',
    'BMC_STARTING',
    'BMC_READY',
    'HOST_POWERING_ON',
    'HOST_POWERED_ON',
    'HOST_BOOTING',
    'HOST_BOOTED',
    'HOST_POWERED_OFF',
]

EXIT_STATE_DEPEND = {
    'BASE_APPS' : {
        '/org/openbmc/sensors': 0,
    },
    'BMC_STARTING' : {
        '/org/openbmc/control/chassis0': 0,
        '/org/openbmc/control/power0' : 0,
        '/org/openbmc/control/flash/bios' : 0,
    },
}

FRU_INSTANCES = {
    '<inventory_root>/system' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },
    '<inventory_root>/system/bios' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },
    '<inventory_root>/system/misc' : { 'fru_type' : 'SYSTEM','is_fru' : False, },

    '<inventory_root>/system/chassis' : { 'fru_type' : 'SYSTEM','is_fru' : True, 'present' : "True" },

    '<inventory_root>/system/chassis/motherboard' : { 'fru_type' : 'MAIN_PLANAR','is_fru' : True, },

    '<inventory_root>/system/systemevent'                  : { 'fru_type' : 'SYSTEM_EVENT', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/refclock' : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/pcieclock': { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/todclock' : { 'fru_type' : 'MAIN_PLANAR', 'is_fru' : False, },

    '<inventory_root>/system/chassis/fan0' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan1' : { 'fru_type' : 'FAN','is_fru' : True, },
    '<inventory_root>/system/chassis/fan2' : { 'fru_type' : 'FAN','is_fru' : True, },

    '<inventory_root>/system/chassis/motherboard/bmc' : { 'fru_type' : 'BMC','is_fru' : False, 'manufacturer' : 'ASPEED' },

    '<inventory_root>/system/chassis/motherboard/cpu0' : { 'fru_type' : 'CPU', 'is_fru' : True, },
    '<inventory_root>/system/chassis/motherboard/cpu1' : { 'fru_type' : 'CPU', 'is_fru' : True, },

    '<inventory_root>/system/chassis/motherboard/cpu0/core0' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core1' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core2' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core3' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core4' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core5' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core6' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core7' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core8' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core9' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core10': { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu0/core11': { 'fru_type' : 'CORE', 'is_fru' : False, },

    '<inventory_root>/system/chassis/motherboard/cpu1/core0' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core1' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core2' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core3' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core4' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core5' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core6' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core7' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core8' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core9' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core10' : { 'fru_type' : 'CORE', 'is_fru' : False, },
    '<inventory_root>/system/chassis/motherboard/cpu1/core11' : { 'fru_type' : 'CORE', 'is_fru' : False, },

    '<inventory_root>/system/chassis/motherboard/dimm0' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm1' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm2' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm3' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm4' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm5' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm6' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm7' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm8' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm9' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm10' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm11' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm12' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm13' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm14' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm15' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm16' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm17' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm18' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm19' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm20' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm21' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm22' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm23' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm24' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm25' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm26' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm27' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm28' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm29' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm30' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
    '<inventory_root>/system/chassis/motherboard/dimm31' : { 'fru_type' : 'DIMM', 'is_fru' : True,},
}

ID_LOOKUP = {
    'FRU' : {
        0x01 : '<inventory_root>/system/chassis/motherboard/cpu0',
        0x02 : '<inventory_root>/system/chassis/motherboard/cpu1',
        0x03 : '<inventory_root>/system/chassis/motherboard',
        0x04 : '<inventory_root>/system/chassis/motherboard/membuf0',
        0x05 : '<inventory_root>/system/chassis/motherboard/membuf1',
        0x06 : '<inventory_root>/system/chassis/motherboard/membuf2',
        0x07 : '<inventory_root>/system/chassis/motherboard/membuf3',
        0x08 : '<inventory_root>/system/chassis/motherboard/membuf4',
        0x09 : '<inventory_root>/system/chassis/motherboard/membuf5',
        0x0c : '<inventory_root>/system/chassis/motherboard/dimm0',
        0x0d : '<inventory_root>/system/chassis/motherboard/dimm1',
        0x0e : '<inventory_root>/system/chassis/motherboard/dimm2',
        0x0f : '<inventory_root>/system/chassis/motherboard/dimm3',
        0x10 : '<inventory_root>/system/chassis/motherboard/dimm4',
        0x11 : '<inventory_root>/system/chassis/motherboard/dimm5',
        0x12 : '<inventory_root>/system/chassis/motherboard/dimm6',
        0x13 : '<inventory_root>/system/chassis/motherboard/dimm7',
        0x14 : '<inventory_root>/system/chassis/motherboard/dimm8',
        0x15 : '<inventory_root>/system/chassis/motherboard/dimm9',
        0x16 : '<inventory_root>/system/chassis/motherboard/dimm10',
        0x17 : '<inventory_root>/system/chassis/motherboard/dimm11',
        0x18 : '<inventory_root>/system/chassis/motherboard/dimm12',
        0x19 : '<inventory_root>/system/chassis/motherboard/dimm13',
        0x1a : '<inventory_root>/system/chassis/motherboard/dimm14',
        0x1b : '<inventory_root>/system/chassis/motherboard/dimm15',
        0x1c : '<inventory_root>/system/chassis/motherboard/dimm16',
        0x1d : '<inventory_root>/system/chassis/motherboard/dimm17',
        0x1e : '<inventory_root>/system/chassis/motherboard/dimm18',
        0x1f : '<inventory_root>/system/chassis/motherboard/dimm19',
        0x20 : '<inventory_root>/system/chassis/motherboard/dimm20',
        0x21 : '<inventory_root>/system/chassis/motherboard/dimm21',
        0x22 : '<inventory_root>/system/chassis/motherboard/dimm22',
        0x23 : '<inventory_root>/system/chassis/motherboard/dimm23',
        0x24 : '<inventory_root>/system/chassis/motherboard/dimm24',
        0x25 : '<inventory_root>/system/chassis/motherboard/dimm25',
        0x26 : '<inventory_root>/system/chassis/motherboard/dimm26',
        0x27 : '<inventory_root>/system/chassis/motherboard/dimm27',
        0x28 : '<inventory_root>/system/chassis/motherboard/dimm28',
        0x29 : '<inventory_root>/system/chassis/motherboard/dimm29',
        0x2a : '<inventory_root>/system/chassis/motherboard/dimm30',
        0x2b : '<inventory_root>/system/chassis/motherboard/dimm31',
    },
    'FRU_STR' : {
        'PRODUCT_0'  : '<inventory_root>/system/bios',
        'BOARD_1'    : '<inventory_root>/system/chassis/motherboard/cpu0',
        'BOARD_2'    : '<inventory_root>/system/chassis/motherboard/cpu1',
        'CHASSIS_3'  : '<inventory_root>/system/chassis/motherboard',
        'BOARD_3'    : '<inventory_root>/system/misc',
        'PRODUCT_12'   : '<inventory_root>/system/chassis/motherboard/dimm0',
        'PRODUCT_13'   : '<inventory_root>/system/chassis/motherboard/dimm1',
        'PRODUCT_14'   : '<inventory_root>/system/chassis/motherboard/dimm2',
        'PRODUCT_15'   : '<inventory_root>/system/chassis/motherboard/dimm3',
        'PRODUCT_16'   : '<inventory_root>/system/chassis/motherboard/dimm4',
        'PRODUCT_17'   : '<inventory_root>/system/chassis/motherboard/dimm5',
        'PRODUCT_18'   : '<inventory_root>/system/chassis/motherboard/dimm6',
        'PRODUCT_19'   : '<inventory_root>/system/chassis/motherboard/dimm7',
        'PRODUCT_20'   : '<inventory_root>/system/chassis/motherboard/dimm8',
        'PRODUCT_21'   : '<inventory_root>/system/chassis/motherboard/dimm9',
        'PRODUCT_22'   : '<inventory_root>/system/chassis/motherboard/dimm10',
        'PRODUCT_23'   : '<inventory_root>/system/chassis/motherboard/dimm11',
        'PRODUCT_24'   : '<inventory_root>/system/chassis/motherboard/dimm12',
        'PRODUCT_25'   : '<inventory_root>/system/chassis/motherboard/dimm13',
        'PRODUCT_26'   : '<inventory_root>/system/chassis/motherboard/dimm14',
        'PRODUCT_27'   : '<inventory_root>/system/chassis/motherboard/dimm15',
        'PRODUCT_28'   : '<inventory_root>/system/chassis/motherboard/dimm16',
        'PRODUCT_29'   : '<inventory_root>/system/chassis/motherboard/dimm17',
        'PRODUCT_30'   : '<inventory_root>/system/chassis/motherboard/dimm18',
        'PRODUCT_31'   : '<inventory_root>/system/chassis/motherboard/dimm19',
        'PRODUCT_32'   : '<inventory_root>/system/chassis/motherboard/dimm20',
        'PRODUCT_33'   : '<inventory_root>/system/chassis/motherboard/dimm21',
        'PRODUCT_34'   : '<inventory_root>/system/chassis/motherboard/dimm22',
        'PRODUCT_35'   : '<inventory_root>/system/chassis/motherboard/dimm23',
        'PRODUCT_36'   : '<inventory_root>/system/chassis/motherboard/dimm24',
        'PRODUCT_37'   : '<inventory_root>/system/chassis/motherboard/dimm25',
        'PRODUCT_38'   : '<inventory_root>/system/chassis/motherboard/dimm26',
        'PRODUCT_39'   : '<inventory_root>/system/chassis/motherboard/dimm27',
        'PRODUCT_40'   : '<inventory_root>/system/chassis/motherboard/dimm28',
        'PRODUCT_41'   : '<inventory_root>/system/chassis/motherboard/dimm29',
        'PRODUCT_42'   : '<inventory_root>/system/chassis/motherboard/dimm30',
        'PRODUCT_43'   : '<inventory_root>/system/chassis/motherboard/dimm31',
        'PRODUCT_47'   : '<inventory_root>/system/misc',
    },
    'SENSOR' : {
        0x04 : '/org/openbmc/sensors/host/HostStatus',
        0x05 : '/org/openbmc/sensors/host/BootProgress',
        0x0c : '<inventory_root>/system/chassis/motherboard/cpu0',
        0x0e : '<inventory_root>/system/chassis/motherboard/cpu1',
        0x1e : '<inventory_root>/system/chassis/motherboard/dimm3',
        0x1f : '<inventory_root>/system/chassis/motherboard/dimm2',
        0x20 : '<inventory_root>/system/chassis/motherboard/dimm1',
        0x21 : '<inventory_root>/system/chassis/motherboard/dimm0',
        0x22 : '<inventory_root>/system/chassis/motherboard/dimm7',
        0x23 : '<inventory_root>/system/chassis/motherboard/dimm6',
        0x24 : '<inventory_root>/system/chassis/motherboard/dimm5',
        0x25 : '<inventory_root>/system/chassis/motherboard/dimm4',
        0x26 : '<inventory_root>/system/chassis/motherboard/dimm11',
        0x27 : '<inventory_root>/system/chassis/motherboard/dimm10',
        0x28 : '<inventory_root>/system/chassis/motherboard/dimm9',
        0x29 : '<inventory_root>/system/chassis/motherboard/dimm8',
        0x2a : '<inventory_root>/system/chassis/motherboard/dimm15',
        0x2b : '<inventory_root>/system/chassis/motherboard/dimm14',
        0x2c : '<inventory_root>/system/chassis/motherboard/dimm13',
        0x2d : '<inventory_root>/system/chassis/motherboard/dimm12',
        0x2e : '<inventory_root>/system/chassis/motherboard/dimm19',
        0x2f : '<inventory_root>/system/chassis/motherboard/dimm18',
        0x30 : '<inventory_root>/system/chassis/motherboard/dimm17',
        0x31 : '<inventory_root>/system/chassis/motherboard/dimm16',
        0x32 : '<inventory_root>/system/chassis/motherboard/dimm23',
        0x33 : '<inventory_root>/system/chassis/motherboard/dimm22',
        0x34 : '<inventory_root>/system/chassis/motherboard/dimm21',
        0x35 : '<inventory_root>/system/chassis/motherboard/dimm20',
        0x36 : '<inventory_root>/system/chassis/motherboard/dimm27',
        0x37 : '<inventory_root>/system/chassis/motherboard/dimm26',
        0x38 : '<inventory_root>/system/chassis/motherboard/dimm25',
        0x39 : '<inventory_root>/system/chassis/motherboard/dimm24',
        0x3a : '<inventory_root>/system/chassis/motherboard/dimm31',
        0x3b : '<inventory_root>/system/chassis/motherboard/dimm30',
        0x3c : '<inventory_root>/system/chassis/motherboard/dimm29',
        0x3d : '<inventory_root>/system/chassis/motherboard/dimm28',
        0x3e : '<inventory_root>/system/chassis/motherboard/cpu0/core0',
        0x3f : '<inventory_root>/system/chassis/motherboard/cpu0/core1',
        0x40 : '<inventory_root>/system/chassis/motherboard/cpu0/core2',
        0x41 : '<inventory_root>/system/chassis/motherboard/cpu0/core3',
        0x42 : '<inventory_root>/system/chassis/motherboard/cpu0/core4',
        0x43 : '<inventory_root>/system/chassis/motherboard/cpu0/core5',
        0x44 : '<inventory_root>/system/chassis/motherboard/cpu0/core6',
        0x45 : '<inventory_root>/system/chassis/motherboard/cpu0/core7',
        0x46 : '<inventory_root>/system/chassis/motherboard/cpu0/core8',
        0x47 : '<inventory_root>/system/chassis/motherboard/cpu0/core9',
        0x48 : '<inventory_root>/system/chassis/motherboard/cpu0/core10',
        0x49 : '<inventory_root>/system/chassis/motherboard/cpu0/core11',
        0x4a : '<inventory_root>/system/chassis/motherboard/cpu1/core0',
        0x4b : '<inventory_root>/system/chassis/motherboard/cpu1/core1',
        0x4c : '<inventory_root>/system/chassis/motherboard/cpu1/core2',
        0x4d : '<inventory_root>/system/chassis/motherboard/cpu1/core3',
        0x4e : '<inventory_root>/system/chassis/motherboard/cpu1/core4',
        0x4f : '<inventory_root>/system/chassis/motherboard/cpu1/core5',
        0x50 : '<inventory_root>/system/chassis/motherboard/cpu1/core6',
        0x51 : '<inventory_root>/system/chassis/motherboard/cpu1/core7',
        0x52 : '<inventory_root>/system/chassis/motherboard/cpu1/core8',
        0x53 : '<inventory_root>/system/chassis/motherboard/cpu1/core9',
        0x54 : '<inventory_root>/system/chassis/motherboard/cpu1/core10',
        0x55 : '<inventory_root>/system/chassis/motherboard/cpu1/core11',
        0x5f : '/org/openbmc/sensors/host/BootCount',
        0x60 : '<inventory_root>/system/chassis/motherboard',
        0x61 : '<inventory_root>/system/systemevent',
        0x63 : '<inventory_root>/system/chassis/motherboard/refclock',
        0x64 : '<inventory_root>/system/chassis/motherboard/pcieclock',
        0xb1 : '<inventory_root>/system/chassis/motherboard/todclock',
        0xb5 : '/org/openbmc/sensors/host/OperatingSystemStatus',
        0xb6 : '<inventory_root>/system/chassis/motherboard/pcielink',
    },
    'GPIO_PRESENT' : {}
}

GPIO_CONFIG = {}
GPIO_CONFIG['SOFTWARE_PGOOD'] = \
        {'gpio_pin': 'R1', 'direction': 'out'}
GPIO_CONFIG['BMC_POWER_UP'] = \
        {'gpio_pin': 'D1', 'direction': 'out'}
GPIO_CONFIG['SYS_PWROK_BUFF'] = \
        {'gpio_pin': 'D2', 'direction': 'in'}
GPIO_CONFIG['PHY_RST_N'] = \
        {'gpio_pin': 'D3', 'direction': 'out'}
GPIO_CONFIG['HDD_PWR_EN'] = \
        {'gpio_pin': 'D4', 'direction': 'out'}
GPIO_CONFIG['CP0_DEVICES_RESET_N'] = \
        {'gpio_pin': 'A1', 'direction': 'out'}
GPIO_CONFIG['BMC_CP0_PERST_ENABLE'] = \
        {'gpio_pin': 'A3', 'direction': 'out'}
GPIO_CONFIG['BMC_UCD_LATCH_LE'] = \
        {'gpio_pin': 'B4', 'direction': 'out'}
GPIO_CONFIG['FSI_DATA'] = \
        {'gpio_pin': 'E0', 'direction': 'out'}
GPIO_CONFIG['FSI_CLK'] = \
        {'gpio_pin': 'AA0', 'direction': 'out'}
GPIO_CONFIG['FSI_ENABLE'] = \
        {'gpio_pin': 'D0', 'direction': 'out'}
GPIO_CONFIG['CRONUS_SEL'] = \
        {'gpio_pin': 'P6', 'direction': 'out'}
# FIXME: net name is FP_PWR_BTN_N in schematic
GPIO_CONFIG['POWER_BUTTON'] = \
        {'gpio_pin': 'I3', 'direction': 'both'}
# FIXME: net name is FP_RST_BTN_N in schematic
GPIO_CONFIG['RESET_BUTTON'] = \
        {'gpio_pin': 'G3', 'direction': 'both'}
GPIO_CONFIG['PE_MEZZB_PRSNT_N'] = \
        {'gpio_pin': 'P7', 'direction': 'in'}

HWMON_CONFIG = {
    '0-0064': {
        'names': {
        },
        'labels': {
        },
    },
}

GPIO_CONFIGS = {
    'power_config' : {
        'latch_out': 'BMC_UCD_LATCH_LE',
        'power_good_in' : 'SYS_PWROK_BUFF',
        'power_up_outs' : [
            ('SOFTWARE_PGOOD', True),
            ('BMC_POWER_UP', True),
        ],
        'reset_outs' : [
        ],
    },
    'hostctl_config' : {
        'fsi_data' : 'FSI_DATA',
        'fsi_clk' : 'FSI_CLK',
        'fsi_enable' : 'FSI_ENABLE',
        'cronus_sel' : 'CRONUS_SEL',
        'optionals' : [
        ],
    },
}

# Miscellaneous non-poll sensor with system specific properties.
# The sensor id is the same as those defined in ID_LOOKUP['SENSOR'].
MISC_SENSORS = {
    0x5f : { 'class' : 'BootCountSensor' },
    0x05 : { 'class' : 'BootProgressSensor' },
    0xb5 : { 'class' : 'OperatingSystemStatusSensor' },
}

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
