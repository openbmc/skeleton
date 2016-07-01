#!/usr/bin/python -u

import sys
import dbus
import getopt
import subprocess
import obmc_system_config as System

INV_DBUS_NAME = 'org.openbmc.Inventory'
INV_INTF_NAME = 'org.openbmc.InventoryItem'
NET_DBUS_NAME = 'org.openbmc.NetworkManager'
NET_OBJ_NAME = '/org/openbmc/NetworkManager/Interface'
CHS_DBUS_NAME = 'org.openbmc.control.Chassis'
CHS_OBJ_NAME = '/org/openbmc/control/chassis0'
PROP_INTF_NAME = 'org.freedesktop.DBus.Properties'

FRUS = System.FRU_INSTANCES

# IEEE 802 MAC address mask for locally administered.
# This means the admin has set the MAC and is no longer
# the unique number set by the device manufacturer.
MAC_LOCALLY_ADMIN_MASK = 0x20000000000

# Pop the command name                               
sys.argv.pop(0)

# Get the inventory dbus path based on the requested fru
def get_inv_obj_path(fru_type, fru_name):
    obj_path = ''
    for f in FRUS.keys():
       if (FRUS[f]['fru_type'] == fru_type and
            f.endswith(fru_name)):
            obj_path = f.replace("<inventory_root>",System.INVENTORY_ROOT)
    return obj_path

# Get the inventory property value
def get_inv_value(obj, prop_name):
    value = ''
    dbus_method = obj.get_dbus_method("Get", PROP_INTF_NAME)
    value = dbus_method(INV_INTF_NAME, prop_name)
    return value

# Get the value of the mac on the system without ':' separators
def get_sys_mac(obj):
    sys_mac = ''
    dbus_method = obj.get_dbus_method("GetHwAddress", NET_DBUS_NAME)
    sys_mac = dbus_method("eth0")
    sys_mac = sys_mac.replace(":","")
    return sys_mac

# Replace the value of the system mac with the value of the inventory
# MAC if the system MAC is not locally administered because this means
# the system admin has purposely set the MAC
def sync_mac(obj, inv_mac, sys_mac):
    # Convert sys MAC to int to perform bitwise '&'
    int_sys_mac = int(sys_mac,16)                                                   
    if not int_sys_mac & MAC_LOCALLY_ADMIN_MASK:                                    
        # Sys MAC is not locally administered, go replace it with inv value
        # Add the ':' separators
        inv_mac_str = ':'.join([inv_mac[i]+inv_mac[i+1] for i in range(0,12,2)])
        #Set HW Method already has checking for mac format
        dbus_method = obj.get_dbus_method("SetHwAddress", NET_DBUS_NAME)
        dbus_method("eth0", inv_mac_str)

# Get sys uuid
def get_sys_uuid(obj):
    sys_uuid = ''
    dbus_method = obj.get_dbus_method("Get", PROP_INTF_NAME)
    sys_uuid = dbus_method(CHS_DBUS_NAME, "uuid")
    return sys_uuid

# Set sys uuid, this reboots the BMC for the value to take effect
def set_sys_uuid(uuid):
    rc = subprocess.call(["fw_setenv", "uuid", uuid])
    #TODO Check script that syncs u-boot to /etc/machine-id is in place
    #rc = subprocess.call(["reboot"])

if __name__ == '__main__':
    try:
        opts, args = getopt.getopt(sys.argv,"s:t:n:p:")
    except getopt.GetoptError:
        print "getopt error"

    # what type to sync, currently mac and uuid supported
    sync_type = ''
    # fru type as specified in System.FRU_INSTANCES
    fru_type = ''
    # fru name path as specified in System.FRU_INSTANCES
    fru_name = ''
    # the fru inventory property name
    prop_name = ''
    for opt, arg in opts:
        if opt in ("-t"):
            fru_type = arg
        elif opt in ("-n"):
            fru_name = arg
        elif opt in ("-p"):
            prop_name = arg
        elif opt in ("-s"):
            sync_type = arg

    bus = dbus.SystemBus()
    inv_obj_path = get_inv_obj_path(fru_type, fru_name)
    inv_obj = bus.get_object(INV_DBUS_NAME, inv_obj_path)
    net_obj = bus.get_object(NET_DBUS_NAME, NET_OBJ_NAME)
    chs_obj = bus.get_object(CHS_DBUS_NAME, CHS_OBJ_NAME)

    # Get the value of the requested inventory property
    inv_value = get_inv_value(inv_obj, prop_name)

    if sync_type == "mac":
        sys_mac = get_sys_mac(net_obj)
        if inv_value != sys_mac:
            sync_mac(net_obj, inv_value, sys_mac)
    elif sync_type == "uuid":
        sys_uuid = get_sys_uuid(chs_obj)
        if inv_value != sys_uuid:
            set_sys_uuid(inv_value)
