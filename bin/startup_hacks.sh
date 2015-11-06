#!/bin/sh

systemctl stop serial-getty@ttyS0

sleep 5
echo nct7904 0x2d > /sys/bus/i2c/devices/i2c-6/new_device
sleep 5
echo nct7904 0x2e > /sys/bus/i2c/devices/i2c-6/new_device

