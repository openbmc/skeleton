#! /usr/bin/python

## todos: event logger, 
import dbus
import Openbmc

HOME_PATH = '/media/sf_vbox/openbmc/'
BIN_PATH = HOME_PATH+'bin/'
CACHE_PATH = HOME_PATH+'cache/'
FRU_PATH = CACHE_PATH+'frus/'

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
		'instances' : [	
			{
				'name' : 'Bmc_0',
				'user_label': 'Master Bmc',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.managers.Frus'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'fru_manager.py',
		'heartbeat' : 'no',
		'rest_name' : 'frus',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'Fru Manager',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.managers.Ipmi'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'ipmi_manager.py',
		'heartbeat' : 'no',
		'rest_name' : 'frus',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'Fru Manager',
			}
		]
	}


SYSTEM_CONFIG['org.openbmc.managers.Sensors'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_manager.py',
		'heartbeat' : 'no',
		'rest_name' : 'sensors',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'Sensor Manager',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.loggers.EventLogger'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'eventlogger.py',
		'heartbeat' : 'no',
		'rest_name' : 'events',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'Event Logger',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.watchdog.Host'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'host_watchdog.exe',
		'heartbeat' : 'no',
		'rest_name' : 'watchdog',
		'instances' : [	
			{
				'name' : 'HostWatchdog_0',
				'user_label': 'Host Watchdog',
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
		'heartbeat' : 'yes',
		'instances' : [	
			{
				'name' : 'SystemPower_0',
				'user_label': 'Power control',
				'properties' : { 
					'org.openbmc.Control': {
						'poll_interval' : 3000
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
		'heartbeat' : 'yes',
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
		'instances' : [	
			{
				'name' : 'PowerButton_0',
				'user_label': 'Main Power Button',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.sensors.HostStatus'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_host_status.exe',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'HostStatus_0',
				'user_label': 'Host Status',
				'properties' : { 
					'org.openbmc.SensorValue': {
						'ipmi_id' : 43,
					},
				}

			}
		]
	}
SYSTEM_CONFIG['org.openbmc.leds.ChassisIdentify'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'chassis_identify.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'ChassisIdentify_0',
				'user_label': 'Chassis Identify LED',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.flash.BIOS'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'flash_bios.exe',
		'heartbeat' : 'no',
		'rest_name' : 'flash',
		'instances' : [	
			{
				'name' : 'BIOS_0',
				'user_label': 'BIOS SPI Flash',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.control.Host'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'control_host.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'Host_0',
				'user_label': 'Host Control',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.control.Chassis'] = {
		'system_state' : 'STANDBY',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'chassis_control.py',
		'heartbeat' : 'no',
		'rest_name' : 'chassis',
		'instances' : [	
			{
				'name' : 'Chassis',
				'user_label': 'Chassis Control',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.vpd'] = {
		'system_state' : 'POWERED_ON',
		'start_process' : True,
		'monitor_process' : False,
		'process_name' : 'board_vpd.exe',
		'heartbeat' : 'no',
		'instances' : [
			{
				'name' : 'MBVPD_0',
				'user_label': 'VPD',
			},

		]
	}

SYSTEM_CONFIG['org.openbmc.sensors.Occ'] = {
		'system_state' : 'HOST_UP',
		'start_process' : True,
		'monitor_process' : True,
		'process_name' : 'sensor_occ.exe',
		'heartbeat' : 'no',
		'instances' : [
			{
				'name' : 'Occ_0',
				'user_label': 'CPU0',
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
		'instances' : [
			{
				'name' : 'Fan_0',
				'user_label': 'FAN 0',
			},
			{
				'name' : 'Fan_1',
				'user_label': 'FAN 1',
			},
			{
				'name' : 'Fan_2',
				'user_label': 'FAN 2',
			},

		]
	}

NON_CACHABLE_PROPERTIES = {
	'name'       : True,
	'user_label' : True,
	'location'   : True,
	'cache'      : True
}

FRUS = {}

## key is IPMI FRU ID

FRUS[32] = {
		'name' : 'CPU0',
		'user_label' : "IBM POWER8 CPU",
		'ftype' : Openbmc.FRU_TYPES['CPU'],
		'location' : "P0",
		'manufacturer' : "IBM",
		'cache' : True,
		'state' : Openbmc.FRU_STATES['NORMAL'],
		'sensor_id' : 10,
	}

FRUS[21] = {
		'name' : 'IO_PLANAR',
		'user_label' : "BARRELEYE IO PLANAR",
		'ftype' : Openbmc.FRU_TYPES['BACKPLANE'],	
		'cache' : False,
		'state' : Openbmc.FRU_STATES['NORMAL'],
		'sensor_id' : 11,
	}


GPIO_CONFIG = {}
GPIO_CONFIG['FSI_CLK']    = { 'gpio_num': 4, 'direction': 'out' }
GPIO_CONFIG['FSI_DATA']   = { 'gpio_num': 5, 'direction': 'out' }
GPIO_CONFIG['FSI_ENABLE'] = { 'gpio_num': 24, 'direction': 'out' }
GPIO_CONFIG['POWER_PIN']  = { 'gpio_num': 33, 'direction': 'out'  }
GPIO_CONFIG['CRONUS_SEL'] = { 'gpio_num': 6, 'direction': 'out'  }
GPIO_CONFIG['PGOOD']      = { 'gpio_num': 23, 'direction': 'in'  }
GPIO_CONFIG['IDENTIFY']   = { 'gpio_num': 34, 'direction': 'out' }
GPIO_CONFIG['POWER_BUTTON'] = { 'gpio_num': 32, 'direction': 'in' }

