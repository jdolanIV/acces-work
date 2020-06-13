DEVICE=`lsusb | grep 1605 | awk '{print $4}'`
DEVICE="${DEVICE%%:}"

sudo fxload -v -D /dev/bus/usb/003/$DEVICE -I $1  -t fx2lp
sleep 3

DEVICE=`lsusb | grep 1605 | awk '{print $4}'`
DEVICE="${DEVICE%%:}"

echo sudo chmod 777 /dev/bus/usb/003/$DEVICE
sudo chmod 666 /dev/bus/usb/003/$DEVICE

