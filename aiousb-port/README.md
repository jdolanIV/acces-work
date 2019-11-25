# ACCES I/O Linux USB drivers

To build the driver, you'll need to ensure you have the latest kernel source and headers; see your respective distributions documentation on how to achieve this (usually it's just issuing a `$PAC_MAN install linux-headers-generic` where `$PAC_MAN` is your distributions package manger, like `yum` or `apt`).

Please note: the following assumes you have unzipped the current ACCES Linux driver source to `/usr/src/acces`.

## USB driver install

The ACCES USB driver is a loadable kernel module, to install it do the following:

```
cd /usr/src/acces/acces/usb
make
sudo make install
```

Note: if you've unzipped the `linux-drivers` source to `/usr/src/acces`, then the drivers would be under `/usr/src/acces/acces` and the tests under `/usr/src/acces/tests`.

To verify that the module is loaded, you can do `lsmod | grep accesio` and you should see an entry for `accesio_usb`, additionally, there should be a new device class under the `/dev` folder where any devices that are plugged into the machine will be added. For example, if you have a USB-DIO-48, you would see `/dev/accesio/usb_dio_48_0`. If the firmware for a device has not been copied properly and thus not uploaded to the device, it will show up as `/dev/accesio/unknown_usb_0` until it has been unplugged and the proper firmware loaded on to it.

### Firmware uploading and locations

The following search paths are used to look for firmware on your root filesystem in Linux.
```
/lib/firmware/updates/UTS_RELEASE/
/lib/firmware/updates/
/lib/firmware/UTS_RELEASE/
/lib/firmware/
```

When you run `make`, the firmware files in the `fw` directory are copied to `/lib/firmware` on your system, and when you plug the USB device in, the firmware will be auto-loaded to the device by the driver, re-enumerating the device automatically.

Please note that the firmware will remain on the device for as long as it has power, so system restarts will not cause the firmware to be re-loaded as long as the device still receives power during the reboot. In this way, if you plug a USB device in, it can load it's firmware once, then each subsequent boot of the system will show the product as the correct ID and allow you to immediately start communicating with the device.

If you wish to see which firmware file the driver looks for based on the product ID, you can [view the source](https://github.com/accesio/linux-drivers/blob/master/acces/usb/module/ids.h#L766). 

**!!!** Please note that some products utilize the same firmware.

## Using the device

### Shell and command line access

The USB line of cards this driver supports do not operate on typical file I/O; the driver _does_ have file I/O ability, but only for user customization and the bulk I/O in the file I/O functions are not officially supported.

### Programming language support

Since the driver supports `ioctl` functionality, one can write a C wrapper and thus just about any language can be utilized to communicate with the device.

### libacces

This driver does _not_ support the `libacces.c` C wrapper library since the USB line of cards do not support direct file I/O. For more information see the [HOWTO-LIB](https://github.com/accesio/linux-drivers/blob/master/acces/HOWTO-LIB.md).

### api.h

This driver supports the `api.h` ACCES API. For more information see the [HOWTO-API](https://github.com/accesio/linux-drivers/blob/master/acces/HOWTO-API.md).

## Supported devices

The following is a list (from [`module/ids.h`](https://github.com/accesio/linux-drivers/blob/master/acces/usb/module/ids.h)) of devices supported by this driver:

ACCES I/O USB Vender ID (VID): `0x1605`

Device Product ID's (PID)

Product           | ID (HEX)
------------------| --------
USB-DIO-32        | 0x8001
USB-DIO-32I       | 0x8004
USB-DIO-24        | 0x8005
USB-DIO-48        | 0x8002
USB-DIO-96        | 0x8003
USB-DIO-24-CTR6   | 0x8006
USB-DI-16A-REV-A1 | 0x8008
USB-DO-16A-REV-A1 | 0x8009
USB-DI-16A-REV-A2 | 0x800A
USB-DIO-16H       | 0x800C
USB-DI-16A        | 0x800D
USB-DO-16A        | 0x800E
USB-DIO-16A       | 0x800F
USB-IIRO-16       | 0x8010
USB-II-16         | 0x8011
USB-RO-16         | 0x8012
USB-IIRO-8        | 0x8014
USB-II-8          | 0x8015
USB-IIRO-4        | 0x8016
USB-IDIO-16       | 0x8018
USB-II-16-OLD     | 0x8019
USB-IDO-16        | 0x801A
USB-IDIO-8        | 0x801C
USB-II-8-OLD      | 0x801D
USB-IDIO-4        | 0x801E
USB-CTR-15        | 0x8020
USB-IIRO4-2SM     | 0x8030
USB-IIRO4-COM     | 0x8031
USB-DIO-16-RO-8   | 0x8032
USB-DIO-48-DO-24  | 0x803C
USB-DIO-24-DO-12  | 0x803D
USB-DO-24         | 0x803E
PICO-DIO-16-RO-8  | 0x8033
USBP-II8-IDO-4A   | 0x8036
USB-AI16-16A      | 0x8040
USB-AI16-16E      | 0x8041
USB-AI12-16A      | 0x8042
USB-AI12-16       | 0x8043
USB-AI12-16E      | 0x8044
USB-AI16-64MA     | 0x8045
USB-AI16-64ME     | 0x8046
USB-AI12-64MA     | 0x8047
USB-AI12-64M      | 0x8048
USB-AI12-64ME     | 0x8049
USB-AI16-32A      | 0x804A
USB-AI16-32E      | 0x804B
USB-AI12-32A      | 0x804C
USB-AI12-32       | 0x804D
USB-AI12-32E      | 0x804E
USB-AI16-64A      | 0x804F
USB-AI16-64E      | 0x8050
USB-AI12-64A      | 0x8051
USB-AI12-64       | 0x8052
USB-AI12-64E      | 0x8053
USB-AI16-96A      | 0x8054
USB-AI16-96E      | 0x8055
USB-AI12-96A      | 0x8056
USB-AI12-96       | 0x8057
USB-AI12-96E      | 0x8058
USB-AI16-128A     | 0x8059
USB-AI16-128E     | 0x805A
USB-AI12-128A     | 0x805B
USB-AI12-128      | 0x805C
USB-AI12-128E     | 0x805D
USB-AO-ARB1       | 0x8068
USB-AO16-16A      | 0x8070
USB-AO16-16       | 0x8071
USB-AO16-12A      | 0x8072
USB-AO16-12       | 0x8073
USB-AO16-8A       | 0x8074
USB-AO16-8        | 0x8075
USB-AO16-4A       | 0x8076
USB-AO16-4        | 0x8077
USB-AO12-16A      | 0x8078
USB-AO12-16       | 0x8079
USB-AO12-12A      | 0x807A
USB-AO12-12       | 0x807B
USB-AO12-8A       | 0x807C
USB-AO12-8        | 0x807D
USB-AO12-4A       | 0x807E
USB-AO12-4        | 0x807F
USB-AIO16-16A     | 0x8140
USB-AIO16-16E     | 0x8141
USB-AIO12-16A     | 0x8142
USB-AIO12-16      | 0x8143
USB-AIO12-16E     | 0x8144
USB-AIO16-64MA    | 0x8145
USB-AIO16-64ME    | 0x8146
USB-AIO12-64MA    | 0x8147
USB-AIO12-64M     | 0x8148
USB-AIO12-64ME    | 0x8149
USB-AIO16-32A     | 0x814A
USB-AIO16-32E     | 0x814B
USB-AIO12-32A     | 0x814C
USB-AIO12-32      | 0x814D
USB-AIO12-32E     | 0x814E
USB-AIO16-64A     | 0x814F
USB-AIO16-64E     | 0x8150
USB-AIO12-64A     | 0x8151
USB-AIO12-64      | 0x8152
USB-AIO12-64E     | 0x8153
USB-AIO16-96A     | 0x8154
USB-AIO16-96E     | 0x8155
USB-AIO12-96A     | 0x8156
USB-AIO12-96      | 0x8157
USB-AIO12-96E     | 0x8158
USB-AIO16-128A    | 0x8159
USB-AIO16-128E    | 0x815A
USB-AIO12-128A    | 0x815B
USB-AIO12-128     | 0x815C
USB-AIO12-128E    | 0x815D

**!!!** Please note that these are the Product ID's of the device **_AFTER_** the firmware has been loaded. If the driver is not loaded or the firmware has not been uploaded to the device, the Product ID of the device will be minus `0x8000`, e.g. the USB-DIO-48 has a product of `0x8002`, without firmware it's `0x0002`.
