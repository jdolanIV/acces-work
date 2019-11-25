#!/bin/sh

for i in `find /sys/bus/pci/devices -maxdepth 1` ; do
	echo $i
	cat $i/device
	cat $i/vendor
	xxd $i/config
done