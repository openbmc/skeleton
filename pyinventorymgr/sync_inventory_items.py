#!/usr/bin/python -u
#
# Copyright 2016 IBM Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import dbus
import argparse
import subprocess
import obmc.mapper
import obmc.utils.dtree
import obmc.utils.pathtree
import shutil

INV_DBUS_NAME = 'xyz.openbmc_project.Inventory.Manager'
INV_INTF_NAME = 'xyz.openbmc_project.Inventory.Item.NetworkInterface'
NET_DBUS_NAME = 'org.openbmc.NetworkManager'
NET_OBJ_NAME = '/org/openbmc/NetworkManager/Interface'
CHS_DBUS_NAME = 'org.openbmc.control.Chassis'
CHS_OBJ_NAME = 'xyz.openbmc_project.Inventory.Item.Bmc'
PROP_INTF_NAME = 'org.freedesktop.DBus.Properties'

FRUS = {}

# IEEE 802 MAC address mask for locally administered.
# This means the admin has set the MAC and is no longer
# the unique number set by the device manufacturer.
MAC_LOCALLY_ADMIN_MASK = 0x20000000000

# Get the inventory dbus path based on the requested fru
def get_inv_obj_path(bus, interface):
    obj_path = ''
    mapper = obmc.mapper.Mapper(bus)
    for path, connection in \
        mapper.get_subtree(
            path='/xyz/openbmc_project/inventory').iteritems():
        if interface in str(connection):
            obj_path = path

    return obj_path

# Get the inventory property value
def get_inv_value(obj, prop_name):
    value = ''
    dbus_method = obj.get_dbus_method("Get", PROP_INTF_NAME)
    value = dbus_method(INV_INTF_NAME, prop_name)
    return value

# Get the value of the mac on the system (from u-boot) without ':' separators
def get_sys_mac(obj):
    sys_mac = ''
    try:
        sys_mac = subprocess.check_output(["fw_printenv", "-n", "ethaddr"])
    except:
        # Handle when mac does not exist in u-boot
        return sys_mac
    sys_mac = sys_mac.replace(":", "")
    return sys_mac

# Replace the value of the system mac with the value of the inventory
# MAC if the system MAC is not locally administered because this means
# the system admin has purposely set the MAC
def sync_mac(obj, inv_mac, sys_mac):
    if sys_mac:
        # Convert sys MAC to int to perform bitwise '&'
        int_sys_mac = int(sys_mac, 16)
    else:
        # Set mac to 0 for when u-boot mac is not present
        int_sys_mac = 0
    if not int_sys_mac & MAC_LOCALLY_ADMIN_MASK:
        # Sys MAC is not locally administered, go replace it with inv value
        # Add the ':' separators
        if inv_mac:
            mac_str = ':'.join([inv_mac[i]+inv_mac[i+1] for i in range(0, 12, 2)])
            # The Set HW Method already has checking for mac format
            dbus_method = obj.get_dbus_method("SetHwAddress", NET_DBUS_NAME)
            dbus_method("eth0", mac_str)

# Get sys uuid
def get_sys_uuid(obj):
    inv_uuid = ''
    inv_uuid = obj.Get("xyz.openbmc_project.Inventory.Decorator.UUID", 
        "uuid")
    print 'inv_uuid : ' + inv_uuid
    return inv_uuid

if __name__ == '__main__':
    arg = argparse.ArgumentParser()
    arg.add_argument('-p')
    arg.add_argument('-s')

    opt = arg.parse_args()
    prop_name = opt.p
    sync_type = opt.s

    inventory = os.path.join(
        sys.prefix, 'share', 'inventory', 'inventory.json')
    if os.path.exists(inventory):
        import json
        with open(inventory, 'r') as f:
            try:
                inv = json.load(f)
            except ValueError:
                print "Invalid JSON detected in " + inventory
            else:
                FRUS = inv
    else:
        import obmc_system_config as System
        FRUS = System.FRU_INSTANCES

    bus = dbus.SystemBus()
    if sync_type == "mac":
        inv_obj_path = get_inv_obj_path(bus, INV_INTF_NAME)
        if inv_obj_path:
            inv_obj = bus.get_object(INV_DBUS_NAME, inv_obj_path)
            net_obj = bus.get_object(NET_DBUS_NAME, NET_OBJ_NAME)
    
            # Get the value of the requested inventory property
            inv_value = get_inv_value(inv_obj, prop_name)
            sys_mac = get_sys_mac(net_obj)
            if inv_value != sys_mac:
                sync_mac(net_obj, inv_value, sys_mac)
    elif sync_type == "uuid":
        inv_obj_path = get_inv_obj_path(bus, CHS_OBJ_NAME)
        if inv_obj_path:
            chs_obj = bus.get_object(CHS_DBUS_NAME, inv_obj_path)
            inv_uuid = get_sys_uuid(chs_obj)
            if inv_uuid:
                set_sys_uuid(inv_uuid)
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
