obj-m := test.o
KERNELDIR ?= /lib/modules/$(shell uname -r)/build/
SYMBOL_INC = $(PWD)/../include
 
modules:
	$(MAKE)  -C $(KERNELDIR) M=$(PWD) modules
	echo $(SYMBOL_INC)

modules_install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean