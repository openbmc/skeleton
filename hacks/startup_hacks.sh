#!/bin/sh

systemctl stop serial-getty@ttyS0

# Setup VUART
VUART=/sys/devices/platform/ahb/ahb:apb/1e787000.vuart
echo 4 > $VUART/sirq
echo 0x3f8 > $VUART/lpc_address
echo 1 > $VUART/enabled
