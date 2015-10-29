import os
import cPickle
import Openbmc

CACHE_PATH = '/var/cache/obmc/'

def getCacheFilename(obj_path, iface_name):
	name = obj_path.replace('/','.')
	filename = CACHE_PATH+name[1:]+"@"+iface_name+".props"
	return filename

def save(obj_path, iface_name, properties):
	print "Caching: "+obj_path
	try:
		output = open(getCacheFilename(obj_path, iface_name), 'wb')
		## save properties
		dbus_props = {}

		for p in properties[iface_name].keys():
			dbus_prop = Openbmc.DbusVariable(p,properties[iface_name][p])
			dbus_props[str(p)] = dbus_prop.getBaseValue()

		cPickle.dump(dbus_props,output)
	except Exception as e:
		print "ERROR: "+str(e)
	finally:
		output.close()

def load(obj_path, iface_name, properties):
	## overlay with pickled data
	filename=getCacheFilename(obj_path, iface_name)
	if (os.path.isfile(filename)):
		if (properties.has_key(iface_name) == False):
			properties[iface_name] = {}
		print "Loading from cache: "+filename
		try:			
			p = open(filename, 'rb')
			data = cPickle.load(p)
			for prop in data.keys():
				properties[iface_name][prop] = data[prop]
						
		except Exception as e:
			print "ERROR: Loading cache file: " +str(e)
		finally:
			p.close()


