obj-m := add_sub.o

KBUILD ?= /lib/modules/$(shell uname -r)/build/

 
default:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules

install:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean