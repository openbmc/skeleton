from setuptools import setup
import glob

setup(name='entity-manager',
      version='0.1',
	  packages=['utils'],
      scripts=['platform_scan.py', 'overlay_gen.py', 'fru_device.py'],
      data_files=[('overlay_templates', glob.glob('overlay_templates/*')),
                  ('configurations', glob.glob('configurations/*'))]
      )
