#!/bin/sh

systemctl stop serial-getty@ttyS0

echo 'module i2c_aspeed +p' > /sys/kernel/debug/dynamic_debug/control

sleep 5
echo nct7904 0x2d > /sys/bus/i2c/devices/i2c-6/new_device
sleep 5
echo nct7904 0x2e > /sys/bus/i2c/devices/i2c-6/new_device

echo 'module i2c_aspeed -p' > /sys/kernel/debug/dynamic_debug/control

