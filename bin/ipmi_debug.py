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


class IpmiDebug(dbus.service.Object):
    def __init__(self,bus,name):
        dbus.service.Object.__init__(self,bus,name)

    @dbus.service.signal(DBUS_NAME, "yyyay")
    def ReceivedMessage(self, seq, netfn, cmd, data):
        print "IPMI packet from host:"
        print_packet(seq, netfn, cmd, data)

    @dbus.service.method(DBUS_NAME, "yyyay", "x")
    def sendMessage(self, seq, netfn, cmd, data):
        print "IPMI packet sent to host:"
        print_packet(seq, netfn, cmd, data)
        return 0

class ConsoleReader(object):
    def __init__(self, ipmi_obj):
        self.buffer = ''
        self.seq = 0
        self.ipmi_obj = ipmi_obj
        flags = fcntl.fcntl(sys.stdin.fileno(), fcntl.F_GETFL)
        flags |= os.O_NONBLOCK
        fcntl.fcntl(sys.stdin.fileno(), fcntl.F_SETFL, flags)
        glib.io_add_watch(sys.stdin, glib.IO_IN, self.io_callback)

    def io_callback(self, fd, condition):
        chunk = fd.read()
        for char in chunk:
            self.buffer += char
            if char == '\n':
                self.line(self.buffer)
                self.buffer = ''

        return True

    def line(self, data):
        s = data.split(' ')
        if len(s) < 2:
            print "Not enough bytes to form a valid IPMI packet"
            return
        try:
            data = [int(c, 16) for c in s]
        except ValueError:
            return
        self.seq += 1
        self.ipmi_obj.ReceivedMessage(self.seq, data[0], data[1], data[2:])

def main():
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SessionBus()
    name = dbus.service.BusName(DBUS_NAME, bus)
    obj = IpmiDebug(bus, OBJ_NAME)
    mainloop = gobject.MainLoop()
    r = ConsoleReader(obj)

    print ("Enter IPMI packet as hex values. First two bytes will be used"
            "as netfn and cmd")
    mainloop.run()

if __name__ == '__main__':
    sys.exit(main())
