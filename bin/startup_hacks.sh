#!/bin/sh

systemctl stop serial-getty@ttyS0

echo nct7904 0x2e > /sys/bus/i2c/devices/i2c-6/new_device
echo nct7904 0x2d > /sys/bus/i2c/devices/i2c-6/new_device

echo 0x2d > /sys/bus/i2c/devices/i2c-6/delete_device
echo 0x2e > /sys/bus/i2c/devices/i2c-6/delete_device

echo nct7904 0x2e > /sys/bus/i2c/devices/i2c-6/new_device
echo nct7904 0x2d > /sys/bus/i2c/devices/i2c-6/new_device

echo 255 > /sys/class/hwmon/hwmon1/pwm1
echo 255 > /sys/class/hwmon/hwmon1/pwm2
echo 255 > /sys/class/hwmon/hwmon1/pwm3
echo 255 > /sys/class/hwmon/hwmon1/pwm4

echo 255 > /sys/class/hwmon/hwmon2/pwm1
echo 255 > /sys/class/hwmon/hwmon2/pwm2
echo 255 > /sys/class/hwmon/hwmon2/pwm3
echo 255 > /sys/class/hwmon/hwmon2/pwm4
