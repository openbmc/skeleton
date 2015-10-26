#! /usr/bin/python

import dbus
import Openbmc

HOME_PATH = './'
CACHE_PATH = HOME_PATH+'cache/'
FLASH_DOWNLOAD_PATH = "/tmp"

SYSTEM_NAME = "Palmetto"


## System states
##   state can change to next state in 2 ways:
##   - a process emits a GotoSystemState signal with state name to goto
##   - objects specified in EXIT_STATE_DEPEND have started
SYSTEM_STATES = [
	'BASE_APPS',
	'BMC_INIT',
	'BMC_STARTING',
	'BMC_READY',
	'HOST_POWERING_ON',
	'HOST_POWERED_ON',
	'HOST_BOOTING',
	'HOST_BOOTED',
	'HOST_POWERED_DOWN',
]

EXIT_STATE_DEPEND = {
	'BASE_APPS' : {
		'/org/openbmc/managers/Property': 0,
	},
	'BMC_STARTING' : {
		'/org/openbmc/control/chassis0': 0,
		'/org/openbmc/control/power0' : 0,
		'/org/openbmc/control/led/BMC_READY' : 0,
		'/org/openbmc/control/Host_0' : 0,
	}
}

## method will be called when state is entered
ENTER_STATE_CALLBACK = {
	'HOST_POWERED_ON' : { 
		'bus_name'    : 'org.openbmc.control.Host',
		'obj_name'    : '/org/openbmc/control/Host_0',
		'interface_name' : 'org.openbmc.control.Host',
		'method_name' : 'boot'
	},
	'BMC_READY' : {
		'bus_name'   : 'org.openbmc.control.led',
		'obj_name'   : '/org/openbmc/control/led/BMC_READY',
		'interface_name' : 'org.openbmc.Led',
		'method_name' : 'setOn'
	}
}

SYSTEM_CONFIG = {}

SYSTEM_CONFIG['org.openbmc.managers.Property'] = {
		'system_state' : 'BASE_APPS',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'property_manager.py',
		'instances' : [	{ 'name' : SYSTEM_NAME } ]
	}

SYSTEM_CONFIG['org.openbmc.control.Bmc'] = {
		'system_state' : 'BMC_INIT',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'control_bmc.exe',
		'instances' : [	{ 'name' : 'Bmc_0' } ]
	}

SYSTEM_CONFIG['org.openbmc.managers.Inventory'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'inventory_items.py',
		'instances' : [	{ 'name' : SYSTEM_NAME } ]
	}
SYSTEM_CONFIG['org.openbmc.control.PciePresent'] = {
		'system_state' : 'HOST_POWERED_ON',
		'start_process' : True,
		'monitor_process' : False,
		'process_name' : 'pcie_slot_present.exe',
		'instances' : [	{ 'name' : 'Slots_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.sensor.Power8Virtual'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensors_virtual_p8.py',
		'instances' : [	{ 'name' : 'virtual' } ]
	}

SYSTEM_CONFIG['org.openbmc.managers.Sensors'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_manager.py',
		'instances' : [ { 'name' : SYSTEM_NAME } ]
	}

SYSTEM_CONFIG['org.openbmc.watchdog.Host'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'host_watchdog.exe',
		'instances' : [	
			{
				'name' : 'HostWatchdog_0',
				'properties' : { 
					'org.openbmc.Watchdog' : {
						'poll_interval': 30000,
					}
				}
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.control.Power'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'power_control.exe',
		'instances' : [	
			{
				'name' : 'power0',
				'user_label': 'Power control',
				'properties' : { 
					'org.openbmc.Control': {
						'poll_interval' : 3000
					},
					'org.openbmc.control.Power': {
						'pgood_timeout' : 10
					}
				}
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.buttons.Power'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'button_power.exe',
		'instances' : [	{ 'name' : 'PowerButton_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.control.led'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'led_controller.exe',
		'instances' : [	{ 'name' : 'Dummy' } ]
	}
SYSTEM_CONFIG['org.openbmc.control.Flash'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'flash_bios.exe',
		'instances' : [	{ 'name' : 'dummy' } ]
	}

SYSTEM_CONFIG['org.openbmc.manager.Download'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'download_manager.py',
		'instances' : [	{ 'name' : SYSTEM_NAME } ]
	}

SYSTEM_CONFIG['org.openbmc.control.Host'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'control_host.exe',
		'instances' : [ { 'name' : 'Host_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.control.Chassis'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'chassis_control.py',
		'instances' : [ { 'name' : 'chassis0' } ]
	}

SYSTEM_CONFIG['org.openbmc.vpd'] = {
		'system_state' : 'HOST_POWERED_ON',
		'start_process' : False,
		'monitor_process' : False,
		'process_name' : 'board_vpd.exe',
		'instances' : [ { 'name' : 'MBVPD_0' } ]
	}

SYSTEM_CONFIG['org.openbmc.sensors.Fan'] = {
		'system_state' : 'BMC_STARTING',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'fan.exe',
		'instances' : [	{'name' : 'Fan_0' }, {'name' : 'Fan_1'}, {'name' : 'Fan_2'} ]
	}

CACHED_INTERFACES = {
		"org.openbmc.InventoryItem" : True,
		"org.openbmc.control.Chassis" : True,
	}
INVENTORY_ROOT = '/org/openbmc/inventory'

FRU_INSTANCES = {
	'<inventory_root>/system' :
		{ 'fru_type' : 'SYSTEM','is_fru' : True, },

	'<inventory_root>/system/chassis' :
		{ 'fru_type' : 'SYSTEM','is_fru' : True, },

	'<inventory_root>/system/chassis/motherboard' :
		{ 'fru_type' : 'MAIN_PLANAR','is_fru' : True, },

	'<inventory_root>/system/chassis/fan0' :
		{ 'fru_type' : 'FAN','is_fru' : True, },
	'<inventory_root>/system/chassis/fan1' :
		{ 'fru_type' : 'FAN','is_fru' : True, },
	'<inventory_root>/system/chassis/fan2' :
		{ 'fru_type' : 'FAN','is_fru' : True, },
	'<inventory_root>/system/chassis/fan3' :
		{ 'fru_type' : 'FAN','is_fru' : True, },
	'<inventory_root>/system/chassis/fan4' :
		{ 'fru_type' : 'FAN','is_fru' : True, },

	'<inventory_root>/system/chassis/motherboard/bmc' :
		{ 'fru_type' : 'BMC','is_fru' : False, 
			'manufacturer' : 'ASPEED' },
	'<inventory_root>/system/chassis/motherboard/cpu0' :
		{ 'fru_type' : 'CPU', 'is_fru' : True, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core0' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core1' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core2' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core3' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core4' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core5' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core6' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core7' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core8' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core9' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core10' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/cpu0/core11' :
		{ 'fru_type' : 'CORE', 'is_fru' : False, },
	
	'<inventory_root>/system/chassis/motherboard/centaur0' :
		{ 'fru_type' : 'MEMORY_BUFFER', 'is_fru' : False, },

	'<inventory_root>/system/chassis/motherboard/dimm0' :
		{ 'fru_type' : 'DIMM', 'is_fru' : True,},

	'<inventory_root>/system/chassis/motherboard/dimm1' :
		{ 'fru_type' : 'DIMM', 'is_fru' : True,},

	'<inventory_root>/system/chassis/motherboard/dimm2' :
		{ 'fru_type' : 'DIMM', 'is_fru' : True,},

	'<inventory_root>/system/chassis/motherboard/dimm3' :
		{ 'fru_type' : 'DIMM', 'is_fru' : True,},

	'<inventory_root>/system/chassis/io_board/pcie_slot0' :
		{ 'fru_type' : 'PCIE_CARD', 'is_fru' : True,},

	'<inventory_root>/system/chassis/io_board/pcie_slot1' :
		{ 'fru_type' : 'PCIE_CARD', 'is_fru' : True,},

}

ID_LOOKUP = {
	'FRU' : {
		0x0d : '<inventory_root>/system/chassis',
		0x34 : '<inventory_root>/system/chassis/motherboard',
		0x01 : '<inventory_root>/system/chassis/motherboard/cpu0',
		0x02 : '<inventory_root>/system/chassis/motherboard/centaur0',
		0x03 : '<inventory_root>/system/chassis/motherboard/dimm0',
		0x04 : '<inventory_root>/system/chassis/motherboard/dimm1',
		0x05 : '<inventory_root>/system/chassis/motherboard/dimm2',
		0x06 : '<inventory_root>/system/chassis/motherboard/dimm3',
		0x35 : '<inventory_root>/system',
	},
	'FRU_STR' : {
		'PRODUCT_15' : '<inventory_root>/system',
		'CHASSIS_2' : '<inventory_root>/system/chassis',
		'BOARD_1'   : '<inventory_root>/system/chassis/motherboard/cpu0',
		'BOARD_2'   : '<inventory_root>/system/chassis/motherboard/centaur0',
		'PRODUCT_3'   : '<inventory_root>/system/chassis/motherboard/dimm0',
		'PRODUCT_4'   : '<inventory_root>/system/chassis/motherboard/dimm1',
		'PRODUCT_5'   : '<inventory_root>/system/chassis/motherboard/dimm2',
		'PRODUCT_6'   : '<inventory_root>/system/chassis/motherboard/dimm3',
	},
	'SENSOR' : {
		0x2f : '<inventory_root>/system/chassis/motherboard/cpu0',
		0x22 : '<inventory_root>/system/chassis/motherboard/cpu0/core0',
		0x23 : '<inventory_root>/system/chassis/motherboard/cpu0/core1',
		0x24 : '<inventory_root>/system/chassis/motherboard/cpu0/core2',
		0x25 : '<inventory_root>/system/chassis/motherboard/cpu0/core3',
		0x26 : '<inventory_root>/system/chassis/motherboard/cpu0/core4',
		0x27 : '<inventory_root>/system/chassis/motherboard/cpu0/core5',
		0x28 : '<inventory_root>/system/chassis/motherboard/cpu0/core6',
		0x29 : '<inventory_root>/system/chassis/motherboard/cpu0/core7',
		0x2a : '<inventory_root>/system/chassis/motherboard/cpu0/core8',
		0x2b : '<inventory_root>/system/chassis/motherboard/cpu0/core9',
		0x2c : '<inventory_root>/system/chassis/motherboard/cpu0/core10',
		0x2d : '<inventory_root>/system/chassis/motherboard/cpu0/core11',
		0x2e : '<inventory_root>/system/chassis/motherboard/centaur0',
		0x1e : '<inventory_root>/system/chassis/motherboard/dimm0',
		0x1f : '<inventory_root>/system/chassis/motherboard/dimm1',
		0x20 : '<inventory_root>/system/chassis/motherboard/dimm2',
		0x21 : '<inventory_root>/system/chassis/motherboard/dimm3',
		0x09 : '/org/openbmc/sensor/virtual/BootCount',
		0x05 : '/org/openbmc/sensor/virtual/BootProgress',
		0x04 : '/org/openbmc/sensor/virtual/HostStatus',
		0x08 : '/org/openbmc/sensor/virtual/OccStatus',
		0x32 : '/org/openbmc/sensor/virtual/OperatingSystemStatus',
	},
	'GPIO_PRESENT' : {
		'SLOT0_PRESENT' : '<inventory_root>/system/chassis/io_board/pcie_slot0', 
		'SLOT1_PRESENT' : '<inventory_root>/system/chassis/io_board/pcie_slot1', 
	}
}

GPIO_CONFIG = {}
GPIO_CONFIG['FSI_CLK']    = { 'gpio_num': 484, 'direction': 'out' }
GPIO_CONFIG['FSI_DATA']   = { 'gpio_num': 485, 'direction': 'out' }
GPIO_CONFIG['FSI_ENABLE'] = { 'gpio_num': 504, 'direction': 'out' }
GPIO_CONFIG['POWER_PIN']  = { 'gpio_num': 449, 'direction': 'out'  }
GPIO_CONFIG['CRONUS_SEL'] = { 'gpio_num': 486, 'direction': 'out'  }
GPIO_CONFIG['PGOOD']      = { 'gpio_num': 503, 'direction': 'in'  }
GPIO_CONFIG['IDENTIFY']   = { 'gpio_num': 365, 'direction': 'out' }
GPIO_CONFIG['BMC_READY']   = { 'gpio_num': 365, 'direction': 'out' }
GPIO_CONFIG['POWER_BUTTON'] =  { 'gpio_num': 448, 'direction': 'falling' }
GPIO_CONFIG['SLOT0_RISER_PRESENT'] =   { 'gpio_num': 104, 'direction': 'in' }
GPIO_CONFIG['SLOT1_RISER_PRESENT'] =   { 'gpio_num': 105, 'direction': 'in' }
GPIO_CONFIG['SLOT2_RISER_PRESENT'] =   { 'gpio_num': 106, 'direction': 'in' }
GPIO_CONFIG['SLOT0_PRESENT'] =  { 'gpio_num': 470, 'direction': 'in' }
GPIO_CONFIG['SLOT1_PRESENT'] =  { 'gpio_num': 471, 'direction': 'in' }
GPIO_CONFIG['SLOT2_PRESENT'] =  { 'gpio_num': 109, 'direction': 'in' }
GPIO_CONFIG['MEZZ0_PRESENT'] =  { 'gpio_num': 112, 'direction': 'in' }
GPIO_CONFIG['MEZZ1_PRESENT'] =  { 'gpio_num': 113, 'direction': 'in' }


