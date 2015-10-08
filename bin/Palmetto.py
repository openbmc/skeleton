#! /usr/bin/python

import dbus
import Openbmc

HOME_PATH = './'
CACHE_PATH = HOME_PATH+'cache/'
FRU_PATH = CACHE_PATH+'frus/'
FLASH_DOWNLOAD_PATH = "/tmp"

SYSTEM_NAME = "Palmetto"

SYSTEM_STATES = [
	'INIT',
	'STANDBY',
	'POWERING_ON',
	'POWERED_ON',
	'BOOTING',
	'HOST_UP',
	'SHUTTING_DOWN',
	'POWERING_DOWN'
]

ENTER_STATE_CALLBACK = {
	'POWERED_ON' : { 
		'bus_name'    : 'org.openbmc.control.Host',
		'obj_name'    : '/org/openbmc/control/Host_0',
		'interface_name' : 'org.openbmc.control.Host',
		'method_name' : 'boot'
	}
}

SYSTEM_CONFIG = {}

SYSTEM_CONFIG['org.openbmc.control.Bmc'] = {
		'system_state' : 'INIT',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'control_bmc.exe',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : 'Bmc_0' } ]
	}

SYSTEM_CONFIG['org.openbmc.managers.Inventory'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'inventory_items.py',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : SYSTEM_NAME } ]
	}
SYSTEM_CONFIG['org.openbmc.control.PciePresent'] = {
		'system_state' : 'POWERED_ON',
		'start_process' : True,
		'monitor_process' : False,
		'process_name' : 'pcie_slot_present.exe',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : 'Slots_0' } ]
	}


SYSTEM_CONFIG['org.openbmc.managers.Sensors'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_manager.py',
		'heartbeat' : 'no',
		'instances' : [ { 'name' : SYSTEM_NAME } ]
	}

SYSTEM_CONFIG['org.openbmc.watchdog.Host'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'host_watchdog.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'HostWatchdog_0',
				'properties' : { 
					'org.openbmc.Watchdog' : {
						'poll_interval': 3000,
					}
				}
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.control.Power'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'power_control.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'SystemPower_0',
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

SYSTEM_CONFIG['org.openbmc.sensors.Temperature.Ambient'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_ambient.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'FrontChassis',
				'user_label': 'Ambient Temperature 1',
				'properties' : { 
					'org.openbmc.SensorValue': {
						'poll_interval' : 5000,
					},
					'org.openbmc.SensorThreshold' : {
						'lower_critical': 5,
						'lower_warning' : 10,
						'upper_warning' : 15,
						'upper_critical': 20
					},
					'org.openbmc.SensorI2c' : {
						'dev_path' : '/dev/i2c/i2c0',
						'address' : '0xA0'
					}
				}
			},
		]
	}
SYSTEM_CONFIG['org.openbmc.buttons.Power'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'button_power.exe',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : 'PowerButton_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.sensors.HostStatus'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_host_status.exe',
		'heartbeat' : "no",
		'instances' : [ { 'name' : 'HostStatus_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.leds.ChassisIdentify'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'chassis_identify.exe',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : 'ChassisIdentify_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.flash.Bios'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'flash_bios.exe',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : 'Bios_0' } ]
	}

SYSTEM_CONFIG['org.openbmc.manager.Download'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'download_manager.py',
		'heartbeat' : 'no',
		'instances' : [	{ 'name' : SYSTEM_NAME } ]
	}

SYSTEM_CONFIG['org.openbmc.control.Host'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'control_host.exe',
		'heartbeat' : 'no',
		'instances' : [ { 'name' : 'Host_0' } ]
	}
SYSTEM_CONFIG['org.openbmc.control.Chassis'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'chassis_control.py',
		'heartbeat' : 'no',
		'instances' : [ { 'name' : 'Chassis' } ]
	}

SYSTEM_CONFIG['org.openbmc.vpd'] = {
		'system_state' : 'POWERING_ON',
		'start_process' : True,
		'monitor_process' : False,
		'process_name' : 'board_vpd.exe',
		'heartbeat' : 'no',
		'instances' : [ { 'name' : 'MBVPD_0' } ]
	}

SYSTEM_CONFIG['org.openbmc.sensors.Occ'] = {
		'system_state' : 'BOOTED',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_occ.exe',
		'heartbeat' : 'no',
		'instances' : [
			{
				'name' : 'Occ_0',
				'properties' : { 
					'org.openbmc.Occ' : {
						'poll_interval' : 3000,
					}
				}
			},

		]
	}

SYSTEM_CONFIG['org.openbmc.sensors.Fan'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'fan.exe',
		'heartbeat' : 'no',
		'instances' : [	{'name' : 'Fan_0' }, {'name' : 'Fan_1'}, {'name' : 'Fan_2'} ]
	}

NON_CACHABLE_PROPERTIES = {
	'name'       : True,
	'user_label' : True,
	'location'   : True,
	'state'      : True,
	'cache'      : True
}
INVENTORY_ROOT = '/org/openbmc/inventory/items'

FRU_INSTANCES = {
	'<inventory_root>/system' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['SYSTEM'],
		'is_fru'       : True,
	},
	'<inventory_root>/system/io_board' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['MAIN_PLANAR'],
		'manufacturer' : 'FOXCONN',
		'is_fru'       : True,
		'location'     : 'C1',
	},

	'<inventory_root>/system/motherboard' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['MAIN_PLANAR'],
		'manufacturer' : 'FOXCONN',
		'is_fru'       : True,
		'location'     : 'C0',
	},
	'<inventory_root>/system/fan0' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['FAN'],
		'manufacturer' : 'DELTA',
		'is_fru'       : True,
		'location'     : 'F0',
	},
	'<inventory_root>/system/fan1' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['FAN'],
		'manufacturer' : 'DELTA',
		'is_fru'       : True,
		'location'     : 'F1',
	},
	'<inventory_root>/system/io_board/bmc' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['BMC'],
		'manufacturer' : 'ASPEED',
		'is_fru'       : False,
	},
	'<inventory_root>/system/motherboard/cpu0' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['CPU'],
		'manufacturer' : 'IBM',
		'is_fru'       : True,
		'location'     : 'P0',
	},
	'<inventory_root>/system/motherboard/cpu0/core0' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['CORE'],
		'is_fru'       : False,
	},
	'<inventory_root>/system/motherboard/cpu0/core1' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['CORE'],
		'is_fru'       : False,
	},
	'<inventory_root>/system/motherboard/dimm0' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['DIMM'],
		'is_fru'       : True,
	},
	'<inventory_root>/system/motherboard/dimm1' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['DIMM'],
		'is_fru'       : True,
	},
	'<inventory_root>/system/motherboard/dimm2' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['DIMM'],
		'is_fru'       : True,
	},
	'<inventory_root>/system/motherboard/dimm3' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['DIMM'],
		'is_fru'       : True,
	},
	'<inventory_root>/system/io_board/pcie_slot0' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['PCIE_CARD'],
		'user_label'      : 'PCIe card 0',
		'is_fru'       : True,
	},
	'<inventory_root>/system/io_board/pcie_slot1' :
	{
		'fru_type'        : Openbmc.FRU_TYPES['PCIE_CARD'],
		'user_label'      : 'PCIe card 1',
		'is_fru'       : True,
	},
}

ID_LOOKUP = {
	'FRU' : {
		'14' : '<inventory_root>/system/motherboard/dimm0',
	},
	'SENSOR' : {
		'21' : '<inventory_root>/system/motherboard/dimm0',
		'14' : '/org/openbmc/sensors/HostStatus_0',
	},
	'GPIO_PRESENT' : {
		'SLOT0_RISER_PRESENT' : '<inventory_root>/system/io_board/pcie_left_slot_riser', 
		'SLOT1_RISER_PRESENT' : '<inventory_root>/system/io_board/pcie_right_slot_riser', 
		'SLOT2_RISER_PRESENT' : '<inventory_root>/system/io_board/pcie_x16_slot_riser',
		'SLOT0_PRESENT' : '<inventory_root>/system/io_board/pcie_slot0', 
		'SLOT1_PRESENT' : '<inventory_root>/system/io_board/pcie_slot1', 
		'SLOT2_PRESENT' : '<inventory_root>/system/io_board/pcie_slot2',
		'MEZZ0_PRESENT' : '<inventory_root>/system/io_board/pcie_mezz0', 
		'MEZZ1_PRESENT' : '<inventory_root>/system/io_board/pcie_mezz1',
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
GPIO_CONFIG['POWER_BUTTON'] =  { 'gpio_num': 448, 'direction': 'falling' }
GPIO_CONFIG['SLOT0_RISER_PRESENT'] =   { 'gpio_num': 104, 'direction': 'in' }
GPIO_CONFIG['SLOT1_RISER_PRESENT'] =   { 'gpio_num': 105, 'direction': 'in' }
GPIO_CONFIG['SLOT2_RISER_PRESENT'] =   { 'gpio_num': 106, 'direction': 'in' }
GPIO_CONFIG['SLOT0_PRESENT'] =  { 'gpio_num': 107, 'direction': 'in' }
GPIO_CONFIG['SLOT1_PRESENT'] =  { 'gpio_num': 108, 'direction': 'in' }
GPIO_CONFIG['SLOT2_PRESENT'] =  { 'gpio_num': 109, 'direction': 'in' }
GPIO_CONFIG['MEZZ0_PRESENT'] =  { 'gpio_num': 112, 'direction': 'in' }
GPIO_CONFIG['MEZZ1_PRESENT'] =  { 'gpio_num': 113, 'direction': 'in' }


