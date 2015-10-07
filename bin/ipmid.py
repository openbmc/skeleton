#!/usr/bin/env python

import sys
import subprocess
import dbus
import string
import os
import fcntl
import glib
import gobject
import dbus.service
import dbus.mainloop.glib

DBUS_NAME = 'org.openbmc.HostIpmi'
OBJ_NAME = '/org/openbmc/HostIpmi/1'

def print_packet(seq, netfn, cmd, data):
    print 'seq:   0x%02x\nnetfn: 0x%02x\ncmd:   0x%02x\ndata:  [%s]' % (
            seq, netfn, cmd,
            ", ".join(['0x%02x' % x for x in data]))


class Ipmid(dbus.service.Object):
    def __init__(self, bus, name):
        dbus.service.Object.__init__(self,bus,name)

    def setReader(self, reader):
        self.reader = reader

    @dbus.service.signal(DBUS_NAME, "yyyay")
    def ReceivedMessage(self, seq, netfn, cmd, data):
        print("IPMI packet from host. Seq = 0x%x Netfn = 0x%x Cmd = 0x%x" %
              (ord(seq), ord(netfn), ord(cmd)))

    @dbus.service.method(DBUS_NAME, "", "")
    def test(self):
        print("TEST")

    @dbus.service.method(DBUS_NAME, "yyyay", "x")
    def sendMessage(self, seq, netfn, cmd, data):
        print("IPMI packet sent to host. Seq = 0x%x Netfn = 0x%x Cmd = 0x%x" %
              (int(seq), int(netfn), int(cmd)))

        self.reader.write(seq, netfn, cmd, data)

        return 0

class BtReader(object):
    def __init__(self, ipmi_obj):
        self.ipmi_obj = ipmi_obj
        flags = os.O_NONBLOCK | os.O_RDWR
        self.bt = os.open("/dev/bt", flags)
        glib.io_add_watch(self.bt, glib.IO_IN, self.io_callback)
        ipmi_obj.setReader(self)

    def write(self, seq, netfn, cmd, data):
        # Untested
        val = chr(netfn) + chr(seq) + chr(cmd)
        val += reduce(lambda a, b: a + chr(b), data, "")
        val = chr(len(val)) + val
        os.write(self.bt, val)

    def io_callback(self, fd, condition):
        data = os.read(self.bt, 128)
        self.ipmi_obj.ReceivedMessage(data[2], data[1], data[3], data[4:])
        return True

def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = Ipmid(bus, OBJ_NAME)
    r = BtReader(obj)
    mainloop = gobject.MainLoop()
    print("Started")
    mainloop.run()

if __name__ == '__main__':
    sys.exit(main())
