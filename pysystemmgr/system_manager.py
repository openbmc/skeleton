#!/usr/bin/env python

import gobject
import dbus
import dbus.service
import dbus.mainloop.glib
import os
from obmc.dbuslib.bindings import DbusProperties, DbusObjectManager, get_dbus
import obmc.enums
import obmc_system_config as System
import obmc.system

DBUS_NAME = 'org.openbmc.managers.System'
OBJ_NAME = '/org/openbmc/managers/System'


class SystemManager(DbusProperties, DbusObjectManager):
    def __init__(self, bus, obj_name):
        super(SystemManager, self).__init__(
            conn=bus,
            object_path=obj_name)
        self.bus = bus

        print "SystemManager Init Done"

    @dbus.service.method(DBUS_NAME, in_signature='s', out_signature='sis')
    def gpioInit(self, name):
        gpio_path = ''
        gpio_num = -1
        r = ['', gpio_num, '']
        if name not in System.GPIO_CONFIG:
            # TODO: Better error handling
            msg = "ERROR: "+name+" not found in GPIO config table"
            print msg
            raise Exception(msg)
        else:

            gpio_num = -1
            gpio = System.GPIO_CONFIG[name]
            if 'gpio_num' in System.GPIO_CONFIG[name]:
                gpio_num = gpio['gpio_num']
            else:
                if 'gpio_pin' in System.GPIO_CONFIG[name]:
                    gpio_num = obmc.system.convertGpio(gpio['gpio_pin'])
                else:
                    msg = "ERROR: SystemManager - GPIO lookup failed for "+name
                    print msg
                    raise Exception(msg)

            if (gpio_num != -1):
                r = [obmc.enums.GPIO_DEV, gpio_num, gpio['direction']]
        return r

    @dbus.service.method(DBUS_NAME, in_signature='',
                         out_signature='ssa(sb)a(sb)a(sbb)ssssa(sb)')
    def getGpioConfiguration(self):
        power_config = System.GPIO_CONFIGS.get('power_config', {})
        power_good_in = power_config.get('power_good_in', '')
        latch_out = power_config.get('latch_out', '')
        power_up_outs = power_config.get('power_up_outs', [])
        reset_outs = power_config.get('reset_outs', [])
        pci_reset_outs = power_config.get('pci_reset_outs', [])
        hostctl_config = System.GPIO_CONFIGS.get('hostctl_config', {})
        fsi_data = hostctl_config.get('fsi_data', '')
        fsi_clk = hostctl_config.get('fsi_clk', '')
        fsi_enable = hostctl_config.get('fsi_enable', '')
        cronus_sel = hostctl_config.get('cronus_sel', '')
        optionals = hostctl_config.get('optionals', [])
        r = [power_good_in, latch_out, power_up_outs, reset_outs,
             pci_reset_outs, fsi_data, fsi_clk, fsi_enable, cronus_sel,
             optionals]
        print "Power GPIO config: " + str(r)
        return r


if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = get_dbus()
    obj = SystemManager(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    obj.unmask_signals()
    name = dbus.service.BusName(DBUS_NAME, bus)

    print "Running SystemManager"
    mainloop.run()

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
