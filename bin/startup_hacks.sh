#!/bin/sh

systemctl stop serial-getty@ttyS0

# Set to output pin
 i2cset -y 0x06 0x20 0x06 0x00
 i2cset -y 0x06 0x20 0x07 0x00
 
 # Turn on all fan LED to BLUE
 # i2cset -y 0x06 0x20 0x03 0x55
 # i2cset -y 0x06 0x20 0x02 0xaa

# Setup VUART
VUART=/sys/devices/platform/ahb/ahb:apb/1e787000.vuart
echo 4 > $VUART/sirq
echo 0x3f8 > $VUART/lpc_address
echo 1 > $VUART/enabled
