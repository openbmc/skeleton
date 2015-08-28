#! /usr/bin/python

def BarreleyeProcesses():
	config = {}

	config['org.openbmc.control.Power'] = {
		'exe_name' : 'bin/power_control.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'PowerControl1',
				'user_label': 'Power control',
			}
		]
	}

	config['org.openbmc.sensors.Temperature.Ambient'] = {
		'exe_name' : 'bin/sensor_ambient.exe',
		'heartbeat' : "yes",
		'instances' : [	
			{
				'name' : 'AmbientTemperature1',
				'user_label': 'Ambient Temperature 1',
				'parameters': ['/dev/i2c0','0xA0'],
				'poll_interval': 5000,          
				'lower_critical': 5,
				'lower_warning' : 10,
				'upper_warning' : 15,
				'upper_critical': 20
			},
			{
				'name' : 'AmbientTemperature2',
				'user_label': 'Ambient Temperature 2',
				'parameters': ['/dev/i2c0','0xA2'],
				'poll_interval': 5000,          
				'lower_critical': 5,
				'lower_warning' : 10,
				'upper_warning' : 15,
				'upper_critical': 20
			}
		]
	}
	config['org.openbmc.buttons.ButtonPower'] = {
		'exe_name' : 'bin/button_power.exe',
		'heartbeat' : 'no',
		'instances' : [	
			{
				'name' : 'PowerButton1',
				'user_label': 'Main Power Button',
			}
		]
	}
	config['org.openbmc.sensors.HostStatus'] = {
		'exe_name' : 'bin/sensor_host_status.exe',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'HostStatus1',
				'user_label': 'Host Status',
			}
		]
	}
	config['org.openbmc.leds.ChassisIdentify'] = {
		'exe_name' : 'bin/chassis_identify.exe',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'ChassisIdentify1',
				'user_label': 'Chassis Identify LED',
			}
		]
	}
	config['org.openbmc.flash.BIOS'] = {
		'exe_name' : 'bin/flash_bios.exe',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'BIOS1',
				'user_label': 'BIOS SPI Flash',
			}
		]
	}
	config['org.openbmc.control.Host'] = {
		'exe_name' : 'bin/control_host.exe',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'HostControl1',
				'user_label': 'Host Control',
			}
		]
	}
	config['org.openbmc.control.Chassis'] = {
		'exe_name' : 'bin/chassis_control.py',
		'heartbeat' : "no",
		'instances' : [	
			{
				'name' : 'Chassis',
				'user_label': 'Chassis Control',
			}
		]
	}


	return config

def BarreleyeGpios():
	gpio = {}
	gpio['FSI_CLK']    = { 'gpio_num': 23, 'direction': 'out' }
	gpio['FSI_DATA']   = { 'gpio_num': 24, 'direction': 'out' }
	gpio['FSI_ENABLE'] = { 'gpio_num': 25, 'direction': 'out' }
	gpio['POWER_PIN']  = { 'gpio_num': 26, 'direction': 'out'  }
	gpio['CRONUS_SEL'] = { 'gpio_num': 27, 'direction': 'out'  }
	gpio['PGOOD']      = { 'gpio_num': 28, 'direction': 'in'  }
	gpio['IDENTIFY']   = { 'gpio_num': 30, 'direction': 'out' }
	gpio['POWER_BUTTON'] = { 'gpio_num': 31, 'direction': 'in' }


	return gpio

