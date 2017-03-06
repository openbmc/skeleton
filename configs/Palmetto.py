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
		'/org/openbmc/control/led/identify' : 0,
		'/org/openbmc/control/host0' : 0,
		'/org/openbmc/control/flash/bios' : 0,
	}
}

ID_LOOKUP = {
	'FRU' : {
		0x0d : '<inventory_root>/system/chassis',
		0x34 : '<inventory_root>/system/chassis/motherboard',
		0x01 : '<inventory_root>/system/chassis/motherboard/cpu',
		0x02 : '<inventory_root>/system/chassis/motherboard/membuf',
		0x03 : '<inventory_root>/system/chassis/motherboard/dimm0',
		0x04 : '<inventory_root>/system/chassis/motherboard/dimm1',
		0x05 : '<inventory_root>/system/chassis/motherboard/dimm2',
		0x06 : '<inventory_root>/system/chassis/motherboard/dimm3',
		0x35 : '<inventory_root>/system',
	},
	'FRU_STR' : {
		'PRODUCT_15' : '<inventory_root>/system',
		'CHASSIS_2' : '<inventory_root>/system/chassis',
		'BOARD_1'   : '<inventory_root>/system/chassis/motherboard/cpu',
		'BOARD_2'   : '<inventory_root>/system/chassis/motherboard/membuf',
		'BOARD_14'   : '<inventory_root>/system/chassis/motherboard',
		'PRODUCT_3'   : '<inventory_root>/system/chassis/motherboard/dimm0',
		'PRODUCT_4'   : '<inventory_root>/system/chassis/motherboard/dimm1',
		'PRODUCT_5'   : '<inventory_root>/system/chassis/motherboard/dimm2',
		'PRODUCT_6'   : '<inventory_root>/system/chassis/motherboard/dimm3',
	},
	'SENSOR' : {
		0x34 : '<inventory_root>/system/chassis/motherboard',
		0x37 : '<inventory_root>/system/chassis/motherboard/refclock',
		0x38 : '<inventory_root>/system/chassis/motherboard/pcieclock',
		0x39 : '<inventory_root>/system/chassis/motherboard/todclock',
		0x3A : '<inventory_root>/system/chassis/apss',
		0x2f : '<inventory_root>/system/chassis/motherboard/cpu',
		0x22 : '<inventory_root>/system/chassis/motherboard/cpu/core1',
		0x23 : '<inventory_root>/system/chassis/motherboard/cpu/core2',
		0x24 : '<inventory_root>/system/chassis/motherboard/cpu/core3',
		0x25 : '<inventory_root>/system/chassis/motherboard/cpu/core4',
		0x26 : '<inventory_root>/system/chassis/motherboard/cpu/core5',
		0x27 : '<inventory_root>/system/chassis/motherboard/cpu/core6',
		0x28 : '<inventory_root>/system/chassis/motherboard/cpu/core9',
		0x29 : '<inventory_root>/system/chassis/motherboard/cpu/core10',
		0x2a : '<inventory_root>/system/chassis/motherboard/cpu/core11',
		0x2b : '<inventory_root>/system/chassis/motherboard/cpu/core12',
		0x2c : '<inventory_root>/system/chassis/motherboard/cpu/core13',
		0x2d : '<inventory_root>/system/chassis/motherboard/cpu/core14',
		0x2e : '<inventory_root>/system/chassis/motherboard/membuf',
		0x1e : '<inventory_root>/system/chassis/motherboard/dimm0',
		0x1f : '<inventory_root>/system/chassis/motherboard/dimm1',
		0x20 : '<inventory_root>/system/chassis/motherboard/dimm2',
		0x21 : '<inventory_root>/system/chassis/motherboard/dimm3',
		0x09 : '/org/openbmc/sensors/host/BootCount',
		0x05 : '/org/openbmc/sensors/host/BootProgress',
		0x08 : '/org/openbmc/sensors/host/cpu0/OccStatus',
		0x32 : '/org/openbmc/sensors/host/OperatingSystemStatus',
		0x33 : '/org/openbmc/sensors/host/PowerCap',
	},
	'GPIO_PRESENT' : {
		'SLOT0_PRESENT' : '<inventory_root>/system/chassis/motherboard/pciecard_x16',
		'SLOT1_PRESENT' : '<inventory_root>/system/chassis/motherboard/pciecard_x8',
	}
}

GPIO_CONFIG = {}
GPIO_CONFIG['FSI_CLK']    =   { 'gpio_pin': 'A4', 'direction': 'out' }
GPIO_CONFIG['FSI_DATA']   =   { 'gpio_pin': 'A5', 'direction': 'out' }
GPIO_CONFIG['FSI_ENABLE'] =   { 'gpio_pin': 'D0', 'direction': 'out' }
GPIO_CONFIG['POWER_PIN']  =   { 'gpio_pin': 'E1', 'direction': 'out'  }
GPIO_CONFIG['CRONUS_SEL'] =   { 'gpio_pin': 'A6', 'direction': 'out'  }
GPIO_CONFIG['PGOOD']      =   { 'gpio_pin': 'C7', 'direction': 'in'  }
GPIO_CONFIG['BMC_THROTTLE'] = { 'gpio_pin': 'J3', 'direction': 'out' }
GPIO_CONFIG['IDBTN']       = { 'gpio_pin': 'Q7', 'direction': 'out' }
GPIO_CONFIG['POWER_BUTTON'] = { 'gpio_pin': 'E0', 'direction': 'both' }
GPIO_CONFIG['PCIE_RESET']   = { 'gpio_pin': 'B5', 'direction': 'out' }
GPIO_CONFIG['USB_RESET']    = { 'gpio_pin': 'B6', 'direction': 'out' }
GPIO_CONFIG['SLOT0_RISER_PRESENT'] =   { 'gpio_pin': 'N0', 'direction': 'in' }
GPIO_CONFIG['SLOT1_RISER_PRESENT'] =   { 'gpio_pin': 'N1', 'direction': 'in' }
GPIO_CONFIG['SLOT2_RISER_PRESENT'] =   { 'gpio_pin': 'N2', 'direction': 'in' }
GPIO_CONFIG['SLOT0_PRESENT'] =         { 'gpio_pin': 'N3', 'direction': 'in' }
GPIO_CONFIG['SLOT1_PRESENT'] =         { 'gpio_pin': 'N4', 'direction': 'in' }
GPIO_CONFIG['SLOT2_PRESENT'] =         { 'gpio_pin': 'N5', 'direction': 'in' }
GPIO_CONFIG['MEZZ0_PRESENT'] =         { 'gpio_pin': 'O0', 'direction': 'in' }
GPIO_CONFIG['MEZZ1_PRESENT'] =         { 'gpio_pin': 'O1', 'direction': 'in' }
GPIO_CONFIG['CHECKSTOP']      =   { 'gpio_pin': 'P5', 'direction': 'falling' }

HWMON_CONFIG = {
	'0-0068' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/rtc','poll_interval' : 5000,'scale' : -3,'units' : 'C' },
		}
	},
	'2-004c' :  {
		'names' : {
			'temp1_input' : { 'object_path' : 'temperature/ambient','poll_interval' : 5000,'scale' : -3,'units' : 'C' },
		}
	},
	'3-0050' : {
		'names' : {
			'caps_curr_powercap' : { 'object_path' : 'powercap/curr_cap','poll_interval' : 10000,'scale' : 1,'units' : 'W' },
			'caps_curr_powerreading' : { 'object_path' : 'powercap/system_power','poll_interval' : 10000,'scale' : 1,'units' : 'W' },
			'caps_max_powercap' : { 'object_path' : 'powercap/max_cap','poll_interval' : 10000,'scale' : 1,'units' : 'W' },
			'caps_min_powercap' : { 'object_path' : 'powercap/min_cap','poll_interval' : 10000,'scale' : 1,'units' : 'W' },
			'caps_norm_powercap' : { 'object_path' : 'powercap/n_cap','poll_interval' : 10000,'scale' : 1,'units' : 'W' },
			'caps_user_powerlimit' : { 'object_path' : 'powercap/user_cap','poll_interval' : 10000,'scale' : 1,'units' : 'W' },
		}
	}
}

GPIO_CONFIGS = {
    'power_config' : {
        'power_good_in' : 'PGOOD',
        'power_up_outs' : [
            ('POWER_PIN', False),
        ],
        'reset_outs' : [
            ('USB_RESET', False),
        ],
        'pci_reset_outs': [
            # net name, polarity, reset hold
            ('PCIE_RESET', False, False),
        ],
    },
    'hostctl_config' : {
        'fsi_data' : 'FSI_DATA',
        'fsi_clk' : 'FSI_CLK',
        'fsi_enable' : 'FSI_ENABLE',
        'cronus_sel' : 'CRONUS_SEL',
        'optionals' : [
            ('BMC_THROTTLE', True),
            ('IDBTN', False),
        ],
    },
}

# Miscellaneous non-poll sensor with system specific properties.
# The sensor id is the same as those defined in ID_LOOKUP['SENSOR'].
MISC_SENSORS = {
	0x09 : { 'class' : 'BootCountSensor' },
	0x05 : { 'class' : 'BootProgressSensor' },
	0x08 : { 'class' : 'OccStatusSensor',
		'os_path' : '/sys/bus/i2c/devices/3-0050/online' },
	0x32 : { 'class' : 'OperatingSystemStatusSensor' },
	0x33 : { 'class' : 'PowerCap',
		'os_path' : '/sys/class/hwmon/hwmon1/user_powercap' },
}

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
