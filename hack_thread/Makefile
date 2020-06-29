obj-m += hack_thread.o

KBUILD=/lib/modules/$(shell uname -r)/build/
 
default:
	make -C $(KBUILD) M=$(PWD) modules

clean:
	make -C $(KBUILD) M=$(PWD) clean

menuconfig:
	$(MAKE) -C $(KBUILD) M=$(PWD) menuconfig

ins:
	sudo insmod hack_thread.ko
	sudo rmmod hack_thread