from distutils.core import setup

setup(name='pyinventorymgr',
      version='1.0',
      packages=['obmc.inventory'],
      scripts=['inventory_items.py', 'sync_inventory_items.py'],
      )
