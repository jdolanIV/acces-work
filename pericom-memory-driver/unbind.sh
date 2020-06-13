#!/bin/bash
#
#

# unbind built-in serial drivers from the plug-in PCI serial card
pcidevice=`find /sys |grep "pci.drivers.serial" |grep 0000 |awk -F"/" '{print $7}'`
if [ "$pcidevice" != "" ]; then
	echo -n "$pcidevice" >/sys/bus/pci/drivers/serial/unbind
fi

exit 0


