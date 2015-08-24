#! /usr/bin/python

def Barreleye():
	config = {}

	config['/org/openbmc/Sensors/Temperature/Ambient/0'] = {
		'user_label': 'Ambient Temperature',
		'bus_name': 'org.openbmc.Sensors.Temperature.Ambient',
		'parameters': ['/dev/i2c0','0xA0'],
		'poll_interval': 5000,          
		'lower_critical': 5,
		'lower_warning' : 10,
		'upper_warning' : 15,
		'upper_critical': 20
	}
	config['/org/openbmc/Sensors/HostStatus/0'] = {
		'user_label': 'Host Status',
		'bus_name': 'org.openbmc.Sensors.HostStatus',
		'lower_critical': 5,
		'lower_warning' : 10,
		'upper_warning' : 15,
		'upper_critical': 20
	}


	return config

