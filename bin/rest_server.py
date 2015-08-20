#!/usr/bin/env python 

from BaseHTTPServer import BaseHTTPRequestHandler
import urlparse
import dbus
import json
import re
import xml.etree.ElementTree as ET

def parseIntrospection(obj,interface,method=None):
	introspect_iface = dbus.Interface(obj,"org.freedesktop.DBus.Introspectable")
	tree = ET.ElementTree(ET.fromstring(introspect_iface.Introspect()))
	root = tree.getroot()
	r = []
	for intf in root.iter('interface'):
		if (intf.attrib['name'] == interface):
			for methd in intf.iter('method'):
				if (method == None):
					r.append(methd.attrib['name'])
				elif (method == methd.attrib['name']):
					for arg in methd.iter('arg'):
						a = [arg.attrib['name'],arg.attrib['type'],arg.attrib['direction']]
						r.append(a)
	return r

def getArgFromSignature(args,signature):
	if (signature == 'i'):
		return int(args[0])
	elif (signature == 's'):
		return args[0];

	raise Exception("Inavlid signature: "+signature)	
		

class GetHandler(BaseHTTPRequestHandler):
	def do_GET(self):
		url_data = urlparse.urlparse(self.path)
		parms = urlparse.parse_qs(url_data.query)
		self.objmap = {
			'chassis' : ['org.openbmc.ChassisControl','/org/openbmc/ChassisControl','org.openbmc.ChassisControl'],
			'flash' : ['org.openbmc.Flash.BIOS','/org/openbmc/Flash/BIOS/0','org.openbmc.Flash'],
			'power' : ['org.openbmc.PowerControl','/org/openbmc/PowerControl/0','org.openbmc.PowerControl']
		}
		self.sensors = {
			'/org/openbmc/Sensors/HostStatus/0' : ['org.openbmc.Sensors.HostStatus','org.openbmc.SensorIntegerSettable',0,""], 
			'/org/openbmc/Sensors/Temperature/Ambient/0': ['org.openbmc.Sensors.Temperature.Ambient','org.openbmc.SensorInteger',0,""]
		}


		parts = url_data.path.split('/');
		item = parts[1]
		
		method = ''
		if (len(parts) > 2):
			method = parts[2]

		code = 403
		payload = {'status': 'error'}
		bus = dbus.SessionBus()

		if (item == 'sensors'):
			try:
				for s in self.sensors.keys():
					obj = bus.get_object(self.sensors[s][0],s)
					iface = dbus.Interface(obj,self.sensors[s][1])
					self.sensors[s][2] = iface.getValue()
					self.sensors[s][3] = iface.getUnits()
				
				payload['status'] = 'ok'	
				payload['sensors'] = self.sensors
				code = 200
			except dbus.exceptions.DBusException as e:
				payload['error-message'] = e.get_dbus_name()

		elif (self.objmap.has_key(item)):
			bus_name = self.objmap[item][0]
			obj_name = self.objmap[item][1] 
			interface = self.objmap[item][2]
			try:
				obj = bus.get_object(bus_name,obj_name)
					
				if (method == ''):
					payload['available-methods'] = parseIntrospection(obj,interface)
				else:
					args = parseIntrospection(obj,interface,method)
					arg_array = []
					signature_in = ''
					signature_out = ''
					for a in args:
						if (a[2] == 'in'):
							if (parms.has_key(a[0]) == False):
								raise Exception("Method '"+method+"' requires argument '"+a[0]+"'")
							arg_array.append(getArgFromSignature(parms[a[0]],a[1]))
							signature_in = signature_in + a[1]
						else:
							signature_out = a[1]

					rtn = bus.call_blocking(bus_name,obj_name,interface,method,signature_in,arg_array)

				code = 200
				payload['status'] = 'ok'

			except dbus.exceptions.DBusException as e:
				payload['error-message'] = str(e)
			except Exception as ex:
				payload['error-message'] = str(ex)
				
		else:
			payload['status'] = 'ok'
			payload['available-commands'] = self.objmap.keys()
				
		self.send_response(code)
		self.send_header('Content-Type', 'application/json')
		self.end_headers()
		self.wfile.write(json.dumps(payload))
		return

if __name__ == '__main__':
	from BaseHTTPServer import HTTPServer
	server = HTTPServer(('', 3000), GetHandler)
	print 'Starting server, use <Ctrl-C> to stop'
	server.serve_forever()

