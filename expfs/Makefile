obj-m += expfs.o

KBUILD = /lib/modules/$(shell uname -r)/build

CFG_INC = $(CURRENT_PATH)
# MODCFLAGS:=-O2 -Wall -DMODULE -D__KERNEL__ -DLINUX -std=c99
# EXTRA_CFLAGS  += $(MODULE_FLAGS) -I $(CFG_INC)
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

default:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean

load:
	sudo dmesg -C
	sudo insmod expfs.ko
	sudo mount -t expfs none ~/Documents/testmnt/
	sudo chmod -R 777 ~/Documents/testmnt/
	sudo chown -R ppw:ppw ~/Documents/testmnt/

unload:
	sudo umount ~/Documents/testmnt/
	sudo rmmod expfs
	