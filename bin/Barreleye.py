#! /usr/bin/python
import dbus
import Openbmc

CACHED_INTERFACES = {
	'org.openbmc.Fru' : True
}

SYSTEM_CONFIG = {}

SYSTEM_CONFIG['org.openbmc.managers.Sensors'] = {
		'exe_name' : 'bin/sensor_manager.py',
		'heartbeat' : 'no',
		'rest_name' : 'sensors',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'Sensor Manager',
			}
		]
	}

#SYSTEM_CONFIG['org.openbmc.managers.Frus'] = {
#		'exe_name' : 'bin/fru_manager.py',
#		'heartbeat' : 'no',
#		'rest_name' : 'frus',
#		'instances' : [	
#			{
#				'name' : 'Barreleye',
#				'user_label': 'Fru Manager',
#			}
#		]
#	}


SYSTEM_CONFIG['org.openbmc.loggers.EventLogger'] = {
		'exe_name' : 'bin/eventlogger.py',
		'heartbeat' : 'no',
		'rest_name' : 'events',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'Event Logger',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.managers.IpmiTranslator'] = {
		'exe_name' : 'bin/ipmi_translator.py',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'Barreleye',
				'user_label': 'IPMI Translator',
			}
		]
	}


SYSTEM_CONFIG['org.openbmc.control.Power'] = {
		'exe_name' : 'bin/power_control.exe',
		'heartbeat' : 'yes',
		'instances' : [	
			{
				'name' : 'PowerControl1',
				'user_label': 'Power control',
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.sensors.Temperature.Ambient'] = {
		'exe_name' : 'bin/sensor_ambient.exe',
		'heartbeat' : 'yes',
		'init_methods' : ['org.openbmc.SensorInteger'],
		'poll_interval': 5000,    
		'instances' : [	
			{
				'name' : 'AmbientTemperature1',
				'user_label': 'Ambient Temperature 1',
				'sensor_id' : 41,
				'properties' : { 
					'org.openbmc.SensorIntegerThreshold' : {
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
			{
				'name' : 'AmbientTemperature2',
				'user_label': 'Ambient Temperature 2',
 				'properties' : { 
					'org.openbmc.SensorIntegerThreshold' : {
						'lower_critical': 5,
						'lower_warning' : 10,
						'upper_warning' : 15,
						'upper_critical': 20
					},
					'org.openbmc.SensorI2c' : {
						'dev_path' : '/dev/i2c/i2c0',
						'address' : '0xA2'
					}
				}
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.buttons.ButtonPower'] = {
		'exe_name' : 'bin/button_power.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'PowerButton1',
				'user_label': 'Main Power Button',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.sensors.HostStatus'] = {
		'exe_name' : 'bin/sensor_host_status.exe',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'HostStatus1',
				'user_label': 'Host Status',
				'sensor_id' : 43,
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.leds.ChassisIdentify'] = {
		'exe_name' : 'bin/chassis_identify.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'ChassisIdentify1',
				'user_label': 'Chassis Identify LED',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.flash.BIOS'] = {
		'exe_name' : 'bin/flash_bios.exe',
		'heartbeat' : 'no',
		'rest_name' : 'flash',
		'instances' : [	
			{
				'name' : 'BIOS1',
				'user_label': 'BIOS SPI Flash',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.control.Host'] = {
		'exe_name' : 'bin/control_host.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'HostControl1',
				'user_label': 'Host Control',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.control.Chassis'] = {
		'exe_name' : 'bin/chassis_control.py',
		'heartbeat' : 'no',
		'rest_name' : 'chassis',
		'instances' : [	
			{
				'name' : 'Chassis',
				'user_label': 'Chassis Control',
			}
		]
	}
SYSTEM_CONFIG['org.openbmc.frus.Fan'] = {
		'exe_name' : 'bin/fan.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'Fan0',
				'user_label': 'Fan 0',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'FAN0',
						'location' : 'F0',
						'type' : Openbmc.FRU_TYPES['FAN'],
					}
				}

			},
			{
				'name' : 'Fan1',
				'user_label': 'Fan 1',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'FAN1',
						'location' : 'F1',
						'type' : Openbmc.FRU_TYPES['FAN'],
					}
				}

			},
			{
				'name' : 'Fan2',
				'user_label': 'Fan 2',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'FAN2',
						'location' : 'F2',
						'type' : Openbmc.FRU_TYPES['FAN'],
					}
				}

			},
			{
				'name' : 'Fan3',
				'user_label': 'Fan 3',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'FAN3',
						'location' : 'F3',
						'type' : Openbmc.FRU_TYPES['FAN'],
					}
				}

			},
			{
				'name' : 'Fan4',
				'user_label': 'Fan 4',				
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'FAN4',
						'location' : 'F4',
						'type' : Openbmc.FRU_TYPES['FAN'],
					}
				}

			},
			{
				'name' : 'Fan5',
				'user_label': 'Fan 5',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'FAN5',
						'location' : 'F5',
						'type' : Openbmc.FRU_TYPES['FAN'],
					}
				}

			}

		]
	}

SYSTEM_CONFIG['org.openbmc.frus.Board'] = {
		'exe_name' : 'bin/fru_board.exe',
		'init_methods' : ['org.openbmc.Fru'],
		'heartbeat' : 'no',
		'instances' : [
			{
				'name' : 'IO_Planer',
				'user_label': 'IO Planar',
				'fru_id' : 61,
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'IO Planar',
						'location' : 'IO_PLANAR',
						'type' : Openbmc.FRU_TYPES['BACKPLANE']
					},
					'org.openbmc.Fru.Eeprom' : {
						'i2c_address' : '0xA8',
						'i2c_dev_path' : '/dev/i2c/i2c5'
					}
				}
			}
		]
	}

SYSTEM_CONFIG['org.openbmc.frus.Fru'] = {
		'exe_name' : 'bin/fru_generic.exe',
		'heartbeat' : 'no',
		'instances' : [
			{
				'name' : 'Backplane',
				'user_label': '2S Motherboard',
				'fru_id' : 60,
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'MAIN_PLANAR',
						'location' : 'C0',
						'type' : Openbmc.FRU_TYPES['BACKPLANE'],
					}
				}
			},
			{
				'name' : 'DIMM0',
				'user_label': 'DIMM A0 Slot 0',
				'fru_id' : 12,
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'DIMM0',
						'location' : 'A0',
						'type' : Openbmc.FRU_TYPES['DIMM'],
					}
				}
			},
			{
				'name' : 'DIMM1',
				'user_label': 'DIMM A1 Slot 0',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'DIMM1',
						'location' : 'A1',
						'type' : Openbmc.FRU_TYPES['DIMM'],
					}
				}
			},
			{
				'name' : 'CPU0',
				'user_label': 'CPU0',
				'properties' : { 
					'org.openbmc.Fru' : {
						'label' : 'CPU0',
						'location' : 'CPU0',
						'type' : Openbmc.FRU_TYPES['CPU'],
					}
				}
			},

		]
	}

GPIO_CONFIG = {}
GPIO_CONFIG['FSI_CLK']    = { 'gpio_num': 23, 'direction': 'out' }
GPIO_CONFIG['FSI_DATA']   = { 'gpio_num': 24, 'direction': 'out' }
GPIO_CONFIG['FSI_ENABLE'] = { 'gpio_num': 25, 'direction': 'out' }
GPIO_CONFIG['POWER_PIN']  = { 'gpio_num': 26, 'direction': 'out'  }
GPIO_CONFIG['CRONUS_SEL'] = { 'gpio_num': 27, 'direction': 'out'  }
GPIO_CONFIG['PGOOD']      = { 'gpio_num': 28, 'direction': 'in'  }
GPIO_CONFIG['IDENTIFY']   = { 'gpio_num': 30, 'direction': 'out' }
GPIO_CONFIG['POWER_BUTTON'] = { 'gpio_num': 31, 'direction': 'in' }

