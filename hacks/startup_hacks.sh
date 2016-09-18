#!/bin/sh

systemctl stop serial-getty@ttyS0

# Setup VUART
VUART=/sys/devices/platform/ahb/ahb:apb/1e787000.vuart
echo 4 > $VUART/sirq
echo 0x3f8 > $VUART/lpc_address

# Enable fans on Firestone
FIRESTONE_FAN_CTL_BASE=/sys/class/i2c-adapter/i2c-3/3-0052/hwmon/hwmon0
if [[ -e $FIRESTONE_FAN_CTL_BASE ]]; then
	echo 1 > ${FIRESTONE_FAN_CTL_BASE}/pwm1_enable
	echo 1 > ${FIRESTONE_FAN_CTL_BASE}/pwm2_enable
	echo 1 > ${FIRESTONE_FAN_CTL_BASE}/pwm3_enable
	echo 1 > ${FIRESTONE_FAN_CTL_BASE}/pwm4_enable
fi
