import unittest2 as unittest
import os
import fru_device


THIS_DIR = os.path.dirname(os.path.realpath(__file__))


class TestFruDevice(unittest.TestCase):

    def setUp(self):
        with open(os.path.join(THIS_DIR, r'S2600WFT.fru.bin'), 'rb') as fru:
            self.fru_bytes = bytearray(fru.read())

    def test_read_fru(self):
        self.assertEqual(fru_device.read_fru(self.fru_bytes)['PRODUCT_PRODUCT_NAME'], b'S2600WFT')