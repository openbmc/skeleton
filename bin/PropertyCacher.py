import os
import cPickle
import json
import Openbmc

CACHE_PATH = '/var/cache/obmc/'

def getCacheFilename(obj_path, iface_name):
	name = obj_path.replace('/','.')
	filename = CACHE_PATH+name[1:]+"@"+iface_name+".props"
	return filename

def save(obj_path, iface_name, properties):
	print "Caching: "+obj_path
	try:
		
		filename = getCacheFilename(obj_path, iface_name)
		output = open(filename, 'wb')
		try:
			## use json module to convert dbus datatypes
			props = json.dumps(properties[iface_name])
			prop_obj = json.loads(props)
			cPickle.dump(prop_obj,output)
		except Exception as e:
			print "ERROR: "+str(e)
		finally:
			output.close()
	except:
		print "ERROR opening cache file: "+filename


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


