BASE_NAME := mt7902
KERNEL_SOURCE_DIR = /lib/modules/`$(shell uname -r)`/build

obj-m := ???

all: modules

clean modules modules_install:
	$(MAKE) -C $(KERNEL_SOURCE_DIR) M=$(shell pwd) VERSION="0.1" $@

install: modules_install
