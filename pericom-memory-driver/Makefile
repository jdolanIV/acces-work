obj-m += ptlser.o
CC		:= gcc
KVERSION        := $(shell uname -r)
KDIR		:= /lib/modules/$(KVERSION)/build

# ptlser-objs := ptlser         \
#         apci_fops.o   \
# 	apci_dev.o

all:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) modules  EXTRA_CFLAGS=-g

clean: 
	$(MAKE) CC=$(CC) -C $(KDIR) M=$(PWD) clean

install:
	$(MAKE) CC=$(CC) -C $(KDIR) M=$$PWD modules_install
	depmod
