MODULE_NAME := sblkdev
obj-m := $(MODULE_NAME).o

OBJ_LIST := main.o
$(MODULE_NAME)-y += $(OBJ_LIST)

ccflags-y := -O2

KERNELDIR := /lib/modules/$(shell uname -r)/build

all: sblkdev

sblkdev:
	make -C $(KERNELDIR) M=$(PWD) modules

insmod:
	sudo insmod $(MODULE_NAME).ko
	sudo mknod  /dev/sblkdev b 252 0
	sudo chmod 777 /dev/sblkdev
	sudo mkdir mnt
	sudo mkfs.ext2 /dev/sblkdev
	sudo mount /dev/sblkdev ./mnt
	sudo chmod 777 ./mnt

reinsmod:
	sudo rmmod $(MODULE_NAME)
	sudo insmod $(MODULE_NAME).ko

rmmod:
	sudo umount mnt/
	sudo rm -r mnt/
	sudo rmmod $(MODULE_NAME)
	sudo rm /dev/sblkdev*

test:
	echo "aaaaaaaaaa" > mnt/t.txt
	cat mnt/t.txt
	echo "ls -al ./mnt" > mnt/t.sh
	sudo chmod +x mnt/t.sh
	./mnt/t.sh
