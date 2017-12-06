#!/usr/bin/python

import subprocess
import os
import re
import warnings
import time
import glob
import struct
import datetime

INTEL_EPOCH = datetime.datetime(year=1996, month=1, day=1)

BASEBOARD_FRU = r'/etc/fru/baseboard.fru.bin'
TIMEOUT = 2.0


class FruDeviceProbe(object):
    def __init__(self):
        if not os.environ.get('TEST', ''):
            self.children = find_i2c_frus()
            self.append_baseboard_fru(BASEBOARD_FRU)
            for fru in self.children:
                fru['formatted'] = read_fru(fru['fru'])

    def append_baseboard_fru(self, fru_path):
        try:
            with open(fru_path, 'rb') as fru:
                fru_bytes = bytearray(fru.read())
            baseboard_fru = {'fru': fru_bytes, 'device': 0, 'bus': 0}
            self.children.append(baseboard_fru)
        except IOError:
            warnings.warn('Cannot find baseboard FRU.')

    def probe(self, match_dict, fru_address='any'):
        found = self.children[:]
        for key, value in match_dict.iteritems():
            match = re.compile(value)
            for child in found[:]:
                m = match.search(child['formatted'].get(key, ""))
                if not m:
                    found.remove(child)
        if fru_address != 'any':
            for f in found[::]:
                if f['device'] != fru_address:
                    found.remove(f)
        # return needed template fill parameters
        return [self.template_resp(x) for x in found]

    @staticmethod
    def template_resp(fru):
        return {'bus': fru['bus'], 'fruaddress': hex(fru['device'])}

    def get_all(self):
        return [x['formatted'] for x in self.children]


def run_command(command):
    p = subprocess.Popen(command, shell=True, stdout=subprocess.PIPE)
    start = time.time()
    while p.poll() is None and (time.time() - start) < TIMEOUT:
        pass
    if p.poll() is None:
        p.terminate()
    if p.returncode == 0:
        return p.communicate()[0]
    raise subprocess.CalledProcessError(command, p.returncode)


def i2c_cmd_to_bytes(cmd):
    data = re.split(':', cmd)[1]
    data = data.replace('0x', '').strip()
    return bytearray.fromhex(data)


def find_i2c_frus():
    fru_devices = []
    unknown_i2c_devices = []
    i2c_devices = glob.glob('/dev/i2c*')
    i2c_indexes = (idx.split('-')[1] for idx in i2c_devices)
    for bus_index in i2c_indexes:
        try:
            result = run_command("i2cdetect -y -r {}".format(bus_index))
            print result
            device_index = 0
            for line in result.split("\n")[1:]:
                line = line[4:]
                for device in [line[i:i + 3] for i in range(0, len(line), 3)]:

                    if device == "-- ":
                        pass
                    elif device == "   ":
                        pass
                    else:
                        # print("found {:02X}".format(device_index))
                        unknown_i2c_devices.append((bus_index, device_index))
                    device_index += 1
        except subprocess.CalledProcessError:
            pass
    for i2c_bus_index, i2c_device_index in unknown_i2c_devices:
        try:
            # print "querying bus: {} device {:02X}".format(i2c_bus_index, i2c_device_index)
            result = run_command("i2cget -f -y {} 0x{:02X} 0x00 i 0x8".format(i2c_bus_index, i2c_device_index))
            # print "got {}".format(result)
            result_bytes = i2c_cmd_to_bytes(result)
            csum = 256 - sum(result_bytes[0:7]) & 0xFF
            # print "result_bytes {}".format(result_bytes)

            if csum == result_bytes[7]:
                # print "found valid fru bus: {} device {:02X}".format(i2c_bus_index, i2c_device_index)
                for index, area in enumerate(("INTERNAL", "CHASSIS", "BOARD", "PRODUCT", "MULTIRECORD")):
                    area_offset = result_bytes[index + 1]
                    if area_offset != 0:
                        area_offset = area_offset * 8
                        # print "offset 0x{:02X}".format(area_offset)
                        result = run_command(
                            "i2cget -f -y {} 0x{:02X} 0x{:02X} i 0x8".format(i2c_bus_index, i2c_device_index,
                                                                             area_offset))
                        # print "got {}".format(result)
                        area_bytes = i2c_cmd_to_bytes(result)
                        format = area_bytes[0]
                        length = area_bytes[1] * 8
                        if length != 0:
                            area_bytes = []
                            while length > 0:
                                to_get = min(0x20, length)
                                result = run_command(
                                    "i2cget -f -y {} 0x{:02X} 0x{:02X} i 0x{:02X}".format(i2c_bus_index,
                                                                                          i2c_device_index, area_offset,
                                                                                          to_get))
                                area_offset += to_get
                                area_bytes.extend(i2c_cmd_to_bytes(result))
                                length -= to_get
                        result_bytes.extend(area_bytes)

                fru_devices.append({'bus': i2c_bus_index, 'device': i2c_device_index, 'fru': result_bytes})
            # print ""

        except subprocess.CalledProcessError:
            pass
    return fru_devices


def read_fru(fru_bytes):
    # check the header checksum
    if 256 - sum(fru_bytes[0:7]) & 0xFF != fru_bytes[7]:
        return None

    out = {"Common Format Version": str(fru_bytes[0])}

    for index, area in enumerate(("INTERNAL", "CHASSIS", "BOARD", "PRODUCT", "MULTIRECORD")):
        offset = fru_bytes[index + 1] * 8

        if offset and offset != 1:
            format = fru_bytes[offset]
            offset += 1
            length = fru_bytes[offset] * 8
            offset += 1

            if area == "CHASSIS":
                field_names = ("PART_NUMBER", "SERIAL_NUMBER", "CHASSIS_INFO_AM1", "CHASSIS_INFO_AM2")
                out["CHASSIS_TYPE"] = str(fru_bytes[offset])
                offset += 1
            elif area == "BOARD":
                field_names = ("MANUFACTURER", "PRODUCT_NAME", "SERIAL_NUMBER", "PART_NUMBER", "VERSION_ID")
                out["BOARD_LANGUAGE_CODE"] = str(fru_bytes[offset])
                offset += 1
                minutes = struct.unpack("<I", str(fru_bytes[offset:offset + 3] + bytearray(1)))[0]
                out["BOARD_MANUFACTURE_DATE"] = (INTEL_EPOCH + datetime.timedelta(minutes=minutes)).isoformat()
                offset += 3
            elif area == "PRODUCT":
                field_names = ("MANUFACTURER", "PRODUCT_NAME", "PART_NUMBER", "PRODUCT_VERSION",
                               "PRODUCT_SERIAL_NUMBER", "ASSET_TAG")
                out["PRODUCT_LANGUAGE_CODE"] = str(fru_bytes[offset])
                offset += 1
            else:
                field_names = ()

            for field in field_names:
                length = fru_bytes[offset] & 0x3f
                out[area + "_" + field] = str(fru_bytes[offset + 1:offset + 1 + length])
                offset += length + 1
                if offset > len(fru_bytes):
                    warnings.warn('FRU Length Mismatch {}'.format(fru_bytes))
                    break

    return out
