# SPDX-License-Identifier: ISC
ifndef MT7902
BASE_NAME = mt7902

obj-m := mt7902-common.o
obj-m += mt7902.o

KVER = $(shell uname -r)
KDIR = /lib/modules/$(KVER)/build
PWD := $(shell pwd)
VERSION="0.0.1"

mt7902-common-objs = mac.o mcu.o main.o init.o debugfs.o
mt7902-objs = pci.o pci_mac.o pci_mcu.o

default: modules

modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

endif
