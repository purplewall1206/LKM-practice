obj-m += expfs.o

KBUILD = /lib/modules/$(shell uname -r)/build

CFG_INC = $(CURRENT_PATH)
# MODCFLAGS:=-O2 -Wall -DMODULE -D__KERNEL__ -DLINUX -std=c99
# EXTRA_CFLAGS  += $(MODULE_FLAGS) -I $(CFG_INC)
CFLAGS=-std=c99

default:
	$(MAKE) -C $(KBUILD) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD) M=$(PWD) clean