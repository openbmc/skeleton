import unittest2 as unittest
import platform_scan
import fru_device
import os

THIS_DIR = os.path.dirname(os.path.realpath(__file__))


class TestPlatformScan(unittest.TestCase):
    def test_parse_configuration_1u(self):
        fru_dev = fru_device.FruDeviceProbe()
        fru_dev.children = \
            [{'device': 80,
              'bus': '7',
              'fru': bytearray(
                  b'\x01\x00\x00\x00\x01\x0c\x00\xf2\x01\x0b\x19\xe1'
                  b'SAMSUNG ELECTRO-MECHANICS CO.,LTD\xcbPSSF132202A'
                  b'\xcaH79286-001\xc3S02\xd1CNS1322A4SG1U0112\x00\x00'
                  b'\xc1\x00\x00\xa5\x00\x02\x18\x9dI\x14\x05t\x08#\x05'
                  b'(#\xb06P')},
             {'device': 81,
              'bus': '7',
              'fru': bytearray(
                  b'\x01\x00\x00\x00\x01\x0c\x00\xf2\x01\x0b\x19\xe1'
                  b'SAMSUNG ELECTRO-MECHANICS CO.,LTD\xcbPSSF132202A'
                  b'\xcaH79286-001\xc3S02\xd1CNS1322A4SG1U0425\x00\x00\
                  xc1\x00\x00\x9e\x00\x02\x18\x9dI\x14\x05t\x08#\x05'
                  b'(#\xb06P')},
             {'device': 80,
              'bus': '2',
              'fru': bytearray(
                  b'\x01\x01\x00\x02\x00\x00\x00\xfc\x01\x00\x00\x00\x00'
                  b'\x00\x00\x01\x01\x1e\x00v\xc5\xa5\xd1Intel Corporation'
                  b'\xccF1UL16RISER1\xccBQWK63400247\xcaH88399-200\xccFRU '
                  b'Ver 0.02\xd1Field not present\xc1'
              )},
             {'device': 81,
              'bus': '2',
              'fru': bytearray(
                  b'\x01\x01\x00\x02\x00\x00\x00\xfc\x01\x00\x00\x00\x00'
                  b'\x00\x00\x01\x01\x1e\x00v\xc5\xa5\xd1Intel Corporation'
                  b'\xccF1UL16RISER2\xccBQWK63400247\xcaH88399-200\xccFRU '
                  b'Ver 0.02\xd1Field not present\xc1'
              )},
             {'device': 113,
              'bus': '2',
              'fru': bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')},
             {'device': 114,
              'bus': '2',
              'fru': bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')},
             {'device': 115,
              'bus': '2',
              'fru': bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')},
             {'device': 87,
              'bus': '1',
              'fru': bytearray(
                  b'\x01\x01\x00\x02\x00\x00\x00\xfc\x01\x00\x00\x00\x00\x00'
                  b'\x00\x01\x01\t\x00\xc8U\xa6\xd1Intel Corporation\xc7FFPANEL'
                  b'\xccBQRU60201108\xcaG28538-252\xccFRU Ver 0.01\xc1\x00s')}]
        fru_dev.append_baseboard_fru(
            os.path.join(THIS_DIR, 'S2600WFT.fru.bin'))
        for fru in fru_dev.children:
            fru['formatted'] = fru_device.read_fru(fru['fru'])
        p_scan = platform_scan.PlatformScan()
        p_scan.fru = fru_dev
        records = p_scan.parse_configuration()
        print "discovered {} records".format(len(records))

    def test_parse_configuration_2u(self):
        fru_dev = fru_device.FruDeviceProbe()
        fru_dev.children = \
            [{'device': 80,
              'bus': '2',
              'fru': bytearray(
                  b'\x01\x01\x00\x02\x00\x00\x00\xfc\x01\x00\x00\x00\x00\x00'
                  b'\x00\x01\x01\n\x009\xb2\x93\xd1Intel Corporation'
                  b'\xccA2UL16RISER2\xccBQWL42000359\xcaH20078-201\xccFRU Ver 0.01'
                  b'\xc1\x00\x00\x00\x00\xea')},
             {'device': 113,
              'bus': '2',
              'fru': bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')},
             {'device': 114,
              'bus': '2',
              'fru': bytearray(b'\x00\x00\x00\x00\x00\x00\x00\x00')},
             {'device': 87,
              'bus': '1',
              'fru': bytearray(
                  b'\x01\x01\x00\x02\x00\x00\x00\xfc\x01\x00\x00\x00'
                  b'\x00\x00\x00\x01\x01\t\x00\xbb}\x98\xd1Intel Corporation'
                  b'\xc7FFPANEL\xccBQWL52405743\xcaH39380-151\xccFRU Ver 0.01\xc10.')},
             {'device': 0,
              'bus': 0,
              'fru': bytearray(
                  b'\x01\x01\x02\x10\x19\x00\x00\xd3\x01\x00\x00\x00\x00\x00'
                  b'\x00\x01\x01\x0e\x17\xc567890\xc512345\xc513579\xc524680'
                  b'\xc1\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\xeb\x01\t\x00\xcd\xf2\xac\xd1Intel Corporation\xc8S2600WFT'
                  b'\xcc............\xca..........\xccFRU Ver 1.21\xc1\x07\x01'
                  b'\r\x00\xd1Intel Corporation\xc8S2600WFT\xc9123456789\xd4...'
                  b'.................\xc9123454321\xc3abc\xc0\xc1\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
                  b'\x00\x00\x00\x00\x00\x00\x00\x00\x93\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff'
                  b'\xff\xff\xff\xff\xff\xff\xff\xff')}]
        fru_dev.append_baseboard_fru(
            os.path.join(THIS_DIR, 'S2600WFT.fru.bin'))
        for fru in fru_dev.children:
            fru['formatted'] = fru_device.read_fru(fru['fru'])
        p_scan = platform_scan.PlatformScan()
        p_scan.fru = fru_dev
        records = p_scan.parse_configuration()
        print "discovered {} records".format(len(records))
