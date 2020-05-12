obj-m += unload.o

# Set the path to the Kernel build utils.
KBUILD=/lib/modules/$(shell uname -r)/build/
 
default:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean

menuconfig:
	$(MAKE) -C $(KBUILD) M=$(PWD) menuconfig

ins:
	sudo insmod hello.ko
	sudo rmmod hello