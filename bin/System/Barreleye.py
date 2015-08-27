#! /usr/bin/python

def BarreleyeProcesses():
	config = {}

	config['org.openbmc.sensors.Temperature.Ambient'] = {
		'exe_name' : 'bin/sensor_ambient.exe',
		'watchdog' : "yes",
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
		'watchdog' : 'no',
		'instances' : [	
			{
				'name' : 'PowerButton1',
				'user_label': 'Main Power Button',
			}
		]
	}
	config['org.openbmc.leds.ChassisIdentify'] = {
		'exe_name' : 'bin/chassis_identify.exe',
		'watchdog' : "no",
		'instances' : [	
			{
				'name' : 'ChassisIdentify',
				'user_label': 'Chassis Identify LED',
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


	return gpio

