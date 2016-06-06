#!/usr/bin/env python

# Contributors Listed Below - COPYRIGHT 2016
# [+] International Business Machines Corp.
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

import sys
import subprocess
import dbus
import dbus.service
import dbus.mainloop.glib
import gobject


class Goto(dbus.service.Object):
    def __init__(self, connection, path, loop, state, cmdline):
        super(Goto, self).__init__(connection, path)
        self.loop = loop
        self.state = state
        self.cmdline = cmdline
        gobject.idle_add(self.go)

    def go(self):
        if self.cmdline:
            subprocess.call(self.cmdline)

        if self.state:
            self.GotoSystemState(self.state)

        self.loop.quit()

    @dbus.service.signal('org.openbmc.Control', signature='s')
    def GotoSystemState(self, state):
        pass

if __name__ == '__main__':
    dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
    bus = dbus.SystemBus()
    cmdline = None
    state = None

    if len(sys.argv) > 1:
        state = sys.argv[1]
    if len(sys.argv) > 2:
        cmdline = sys.argv[2:]

    loop = gobject.MainLoop()
    o = Goto(bus, '/org/openbmc/goto', loop, state, cmdline)
    loop.run()
