MODULE_NAME := globalfifo



ifneq ($(KERNELRELEASE),) 	# kernelspace

obj-m := $(MODULE_NAME).o

else						# userspace


LINUX_KERNEL ?= $(shell uname -r)
LINUX_KERNEL_PATH ?= /lib/modules/$(LINUX_KERNEL)/build

CURRENT_PATH ?= $(shell pwd)
CFG_INC = $(CURRENT_PATH)
MODCFLAGS:=-O2 -Wall -DMODULE -D__KERNEL__ -DLINUX -std=gnu99
EXTRA_CFLAGS  += $(MODULE_FLAGS) -I $(CFG_INC)


modules:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules

modules_install:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules_install

insmod:
	sudo insmod $(MODULE_NAME).ko globalfifo_major=300
	sudo mknod /dev/globalfifo c 300 0
	sudo chmod 666 /dev/globalfifo

rmmod:
	sudo rmmod $(MODULE_NAME)
	sudo rm -rf /dev/globalfifo

github:
	cd $(ROOT) && make github

test :
	echo "gatieme" > /dev/globalfifo
	cat /dev/globalfifo

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm -f modules.order Module.symvers Module.markers

.PHNOY:
	modules modules_install clean



endif