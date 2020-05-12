ROOT=..

MODULE_NAME := globalmem

ifneq ($(KERNELRELEASE), )



obj-m := $(MODULE_NAME).o

else

KERNELDIR ?= /lib/modules/$(shell uname -r)/build

PWD := $(shell pwd)

modules:
	make -C $(KERNELDIR) M=$(PWD) modules
	gcc -g -o test test.c

modules_install:
	make -C $(KERNELDIR) M=$(PWD) modules_install



insmod:
	sudo insmod $(MODULE_NAME).ko
	sudo mknod /dev/gatieme c 301 0
	sudo chmod 777 /dev/gatieme
reinsmod:
	sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

github:
	cd $(ROOT) && make github

rmmod:
	sudo rmmod $(MODULE_NAME)
	sudo rm /dev/gatieme 

test :
	./test

clean:
	make -C $(KERNELDIR) M=$(PWD) clean
	rm -f modules.order Module.symvers Module.markers

.PHNOY:
	modules modules_install clean



endif