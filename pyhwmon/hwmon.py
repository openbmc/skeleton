#!/usr/bin/env python

import os
import gobject
import glob
import dbus
import dbus.service
import dbus.mainloop.glib
import re
from obmc.dbuslib.bindings import get_dbus

from obmc.sensors import SensorValue as SensorValue
from obmc.sensors import HwmonSensor as HwmonSensor
from obmc.sensors import SensorThresholds as SensorThresholds

try:
    import obmc_system_config as System
    have_system = True
except ImportError:
    have_system = False

SENSOR_BUS = 'org.openbmc.Sensors'
SENSOR_PATH = '/org/openbmc/sensors'
DIR_POLL_INTERVAL = 30000
HWMON_PATH = '/sys/class/hwmon'

## static define which interface each property is under
## need a better way that is not slow
IFACE_LOOKUP = {
    'units': SensorValue.IFACE_NAME,
    'adjust': HwmonSensor.IFACE_NAME,
    'scale': HwmonSensor.IFACE_NAME,
    'offset': HwmonSensor.IFACE_NAME,
    'critical_upper': SensorThresholds.IFACE_NAME,
    'warning_upper': SensorThresholds.IFACE_NAME,
    'critical_lower': SensorThresholds.IFACE_NAME,
    'warning_lower': SensorThresholds.IFACE_NAME,
    'emergency_enabled': SensorThresholds.IFACE_NAME,
}


class Hwmons():
    def __init__(self, bus):
        self.sensors = {}
        self.hwmon_root = {}

	if have_system:
            self.scanDirectory()
            gobject.timeout_add(DIR_POLL_INTERVAL, self.scanDirectory)

    def readAttribute(self, filename):
        val = "-1"
        try:
            with open(filename, 'r') as f:
                for line in f:
                    val = line.rstrip('\n')
        except (OSError, IOError):
            print "Cannot read attributes:", filename
        return val

    def writeAttribute(self, filename, value):
        with open(filename, 'w') as f:
            f.write(str(value)+'\n')

    def poll(self, objpath, attribute):
        try:
            raw_value = int(self.readAttribute(attribute))
            obj = bus.get_object(SENSOR_BUS, objpath, introspect=False)
            intf = dbus.Interface(obj, HwmonSensor.IFACE_NAME)
            rtn = intf.setByPoll(raw_value)
            if rtn[0]:
                self.writeAttribute(attribute, rtn[1])
        except:
            print "HWMON: Attibute no longer exists: "+attribute
            self.sensors.pop(objpath, None)
            return False

        return True

    def addObject(self, dpath, hwmon_path, hwmon):
        objsuf = hwmon['object_path']
        objpath = SENSOR_PATH+'/'+objsuf

        if objpath not in self.sensors:
            print "HWMON add: "+objpath+" : "+hwmon_path

            ## register object with sensor manager
            obj = bus.get_object(SENSOR_BUS, SENSOR_PATH, introspect=False)
            intf = dbus.Interface(obj, SENSOR_BUS)
            intf.register("HwmonSensor", objpath)

            ## set some properties in dbus object
            obj = bus.get_object(SENSOR_BUS, objpath, introspect=False)
            intf = dbus.Interface(obj, dbus.PROPERTIES_IFACE)
            intf.Set(HwmonSensor.IFACE_NAME, 'filename', hwmon_path)

            ## check if one of thresholds is defined to know
            ## whether to enable thresholds or not
            if 'critical_upper' in hwmon:
                intf.Set(
                    SensorThresholds.IFACE_NAME, 'thresholds_enabled', True)

            for prop in hwmon.keys():
                if prop in IFACE_LOOKUP:
                    intf.Set(IFACE_LOOKUP[prop], prop, hwmon[prop])
                    print "Setting: "+prop+" = "+str(hwmon[prop])

            self.sensors[objpath] = True
            self.hwmon_root[dpath].append(objpath)
            gobject.timeout_add(
                hwmon['poll_interval'], self.poll, objpath, hwmon_path)

    def scanDirectory(self):
        devices = os.listdir(HWMON_PATH)
        found_hwmon = {}
        regx = re.compile('([a-z]+)\d+\_')
        for d in devices:
            dpath = HWMON_PATH+'/'+d+'/'
            found_hwmon[dpath] = True
            if dpath not in self.hwmon_root:
                self.hwmon_root[dpath] = []
            ## the instance name is a soft link
            instance_name = os.path.realpath(dpath+'device').split('/').pop()

            if instance_name in System.HWMON_CONFIG:
                hwmon = System.HWMON_CONFIG[instance_name]

                if 'labels' in hwmon:
                    label_files = glob.glob(dpath+'/*_label')
                    for f in label_files:
                        label_key = self.readAttribute(f)
                        if label_key in hwmon['labels']:
                            namef = f.replace('_label', '_input')
                            self.addObject(
                                dpath, namef, hwmon['labels'][label_key])
                        else:
                            pass

                if 'names' in hwmon:
                    for attribute in hwmon['names'].keys():
                        self.addObject(
                            dpath, dpath+attribute, hwmon['names'][attribute])

            else:
                print "WARNING - hwmon: Unhandled hwmon: "+dpath

        for k in self.hwmon_root.keys():
            if k not in found_hwmon:
                ## need to remove all objects associated with this path
                print "Removing: "+k
                for objpath in self.hwmon_root[k]:
                    if objpath in self.sensors:
                        print "HWMON remove: "+objpath
                        self.sensors.pop(objpath, None)
                        obj = bus.get_object(
                            SENSOR_BUS, SENSOR_PATH, introspect=False)
                        intf = dbus.Interface(obj, SENSOR_BUS)
                        intf.delete(objpath)

                self.hwmon_root.pop(k, None)

        return True


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = get_dbus()
    root_sensor = Hwmons(bus)
    mainloop = gobject.MainLoop()

    print "Starting HWMON sensors"
    mainloop.run()
