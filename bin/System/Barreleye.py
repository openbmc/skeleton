#! /usr/bin/python

def BarreleyeSensors():
	config = {}

	config['org.openbmc.sensors.Temperature.Ambient'] = {
		'user_label': 'Ambient Temperature',
		'parameters': ['/dev/i2c0','0xA0'],
		'poll_interval': 5000,          
		'lower_critical': 5,
		'lower_warning' : 10,
		'upper_warning' : 15,
		'upper_critical': 20
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

