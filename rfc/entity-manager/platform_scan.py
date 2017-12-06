#!/usr/bin/python

import os
from fru_device import FruDeviceProbe
import json
import overlay_gen
import glob
import traceback
from utils.generic import prep_json, dict_template_replace

TEMPLATE_CHAR = '$'
MY_DIR = os.path.dirname(os.path.realpath(__file__))
OUTPUT_DIR = os.path.join(
    MY_DIR, 'out') if os.environ.get(
        'TEST', '') else '/var/configuration/'
CONFIGURATION_DIR = os.path.join(
    MY_DIR, 'configurations') if os.environ.get(
        'TEST', '') else '/usr/share/configurations'


class PlatformScan(object):
    def __init__(self):
        self.fru = None
        self.found_entities = []
        self.configuration_dir = CONFIGURATION_DIR

    def parse_fru(self):
        self.fru = FruDeviceProbe()

    @staticmethod
    def find_bind(d):
        for x in list(d):
            if x.startswith('bind'):
                return x
        return False

    @staticmethod
    def has_update(d):
        return 'update' in list(d)

    def apply_update(self, item):
        if 'update' not in list(item):
            return False
        for _, entity in self.found_entities.iteritems():
            for child in entity['exposes']:
                if child['name'] == item['update']:
                    item.pop('update', None)
                    child.update(item)
                    return True

    def apply_bind(self, item):
        bind = self.find_bind(item)
        if not bind:
            return False
        for _, entity in self.found_entities.iteritems():
            for child in entity['exposes']:
                if child['name'] == item[bind]:
                    bind_name = bind.split('_')[1]
                    item[bind_name] = child
                    # todo, should this go here?
                    item['status'] = item[bind_name]['status'] = 'okay'
                    return item

    def read_config(self):
        try:
            with open(os.path.join(OUTPUT_DIR, 'system.json')) as config:
                self.found_entities = json.load(config)
        except IOError:
            return False
        return self.found_entities

    def write_config(self):
        if not os.path.exists(OUTPUT_DIR):
            os.makedirs(OUTPUT_DIR)
        with open(os.path.join(OUTPUT_DIR, 'system.json'), 'w+') as config:
            json.dump(
                self.found_entities,
                config,
                indent=4,
                separators=(
                    ',',
                    ': '),
                sort_keys=True)
        sensors = []
        for _, value in self.found_entities.iteritems():
            if isinstance(value, dict) and value.get('exposes', None):
                sensors += value['exposes']
        # this is only for ease of parsing for apps that don't need to know the parents
        with open(os.path.join(OUTPUT_DIR, 'flattened.json'), 'w+') as sensor_config:
            json.dump(
                sensors,
                sensor_config,
                indent=4,
                separators=(
                    ',',
                    ': '),
                sort_keys=True)

        # todo make this a dbus endpoint in the fru_device
        with open(os.path.join(OUTPUT_DIR, 'frus.json'), 'w+') as fru_config:
            json.dump(
                self.fru.get_all(),
                fru_config,
                indent=4,
                separators=(
                    ',',
                    ': '),
                sort_keys=True)

    def parse_configuration(self):

        available_entity_list = []
        self.found_entities = {}

        # grab all json files and find all entities
        for json_filename in glob.glob(
            os.path.join(
                self.configuration_dir,
                "*.json")):
            with open(os.path.join(self.configuration_dir, json_filename)) as json_file:
                clean_file = prep_json(json_file)
                try:
                    entities = json.load(clean_file)
                    if isinstance(entities, list):
                        available_entity_list += entities
                    else:
                        available_entity_list.append(entities)
                except ValueError as e:
                    traceback.format_exc(e)
                    print(
                        "Failed to parse {} error was {}".format(
                            json_file, e))

        # keep looping until the number of devices didn't change (no new probes
        # passed)
        while True:
            num_devices = len(self.found_entities)
            for entity in available_entity_list[:]:
                probe_command = entity.get("probe", None)
                if not probe_command:
                    available_entity_list.remove(entity)
                    print "entity {} doesn't have a probe function".format(entity.get("name", "unknown"))
                    if not entity.get("name", False):
                        print json.dumps(entity, sort_keys=True, indent=4)

                # loop through all keys that haven't been found
                elif entity['name'] not in (key for key in list(self.found_entities)):
                    probe_devs = eval(
                        probe_command, {
                            'fru': self.fru, 'found_devices': list(
                                self.found_entities)})
                    # TODO Calling eval on a string is bad practice.  Come up with a better
                    # probing structure
                    if not probe_devs:
                        continue

                    entity['type'] = 'entity'
                    entity['status'] = 'okay'
                    exposes = entity.get('exposes', [])

                    if exposes:
                        entity['exposes'] = []

                    idx = 0
                    for probe_dev in probe_devs:
                        for item in exposes:
                            if self.find_bind(item):
                                item = self.apply_bind(item)
                                assert item
                                entity['exposes'].append(item)
                            elif self.has_update(item):
                                assert (self.apply_update(item))
                            else:
                                if TEMPLATE_CHAR in str(item):
                                    probe_dev['index'] = idx
                                    replaced = dict_template_replace(
                                        item, probe_dev)
                                    if 'status' not in replaced:
                                        replaced['status'] = 'okay'
                                    entity['exposes'].append(replaced)
                                else:
                                    replaced = item
                                    if 'status' not in replaced:
                                        replaced['status'] = 'okay'
                                    entity['exposes'].append(replaced)
                        idx += 1
                    self.found_entities[entity['name']] = entity

            if len(self.found_entities) == num_devices:
                break  # exit after looping without additions
        self.write_config()
        return self.found_entities


if __name__ == '__main__':
    platform_scan = PlatformScan()
    platform_scan.parse_fru()
    found_devices = platform_scan.parse_configuration()

    overlay_gen.unload_overlays()  # start fresh

    for pk, pvalue in found_devices.iteritems():
        for element in pvalue.get('exposes', []):
            if not isinstance(
                    element,
                    dict) or element.get(
                    'status',
                    'disabled') != 'okay':
                continue
            element['oem_name'] = element.get(
                'name', 'unknown').replace(
                ' ', '_')

            if element.get("type", "") == "TMP75":
                element["reg"] = element.get("address").lower()
                overlay_gen.load_entity(**element)

            elif element.get("type", "") == "TMP421":
                element["reg"] = element.get("address").lower()
                element["oem_name1"] = element.get("name1").replace(" ", "_")
                overlay_gen.load_entity(**element)

            elif element.get("type", "") == "ADC":
                overlay_gen.load_entity(**element)

            elif element.get("type", "") == "AspeedFan":
                connector = element['connector']
                element.update(connector)
                element["type"] = 'aspeed_pwmtacho'
                overlay_gen.load_entity(**element)

            elif element.get("type", "") == "SkylakeCPU":
                element["type"] = 'aspeed_peci_hwmon'
                overlay_gen.load_entity(**element)

            elif element.get("type", "") == "IntelFruDevice":
                element["type"] = 'eeprom'
                element["reg"] = element.get("address").lower()
                overlay_gen.load_entity(**element)
