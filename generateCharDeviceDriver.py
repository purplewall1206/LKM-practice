import sys
import os

# 创建工程名
projectname = 'second'

code = '''
#include <linux/init.h>
#include <linux/module.h>
#include <asm/current.h>
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ppw");
#define BUFSIZE  0x1000
#define MODULE_NAME "%s"
static int %s_major = 300;
module_param(%s_major, int, S_IRUGO);

struct %s_dev 
{
    struct cdev   dev;
    char   buf[BUFSIZE];
    struct mutex mutex;
};

struct %s_dev*  devp = NULL;

int %s_open(struct inode *inode, struct file *filp) 
{
    filp->private_data = devp;
    pr_info("%s opened by %%d : %s\\n", MODULE_NAME, current->pid, current->comm);
    return 0;
}

int %s_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long %s_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct inode *inode = inode = file_inode(filp);
    struct %s_dev *dev = filp->private_data;
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return 0;
}

static ssize_t %s_read(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    struct %s_dev *dev = filp->private_data;
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return 0;
}

static ssize_t %s_write(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    struct %s_dev *dev = filp->private_data;
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return 0;
}

static loff_t %s_llseek(struct file *filp, loff_t offset, int orig)
{
    return 0;
}

static unsigned int %s_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    struct %s_dev *dev = filp->private_data; 
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return mask;
}

static const struct file_operations %s_fops = 
{
    .owner = THIS_MODULE,
    .llseek = %s_llseek,
    .read = %s_read,
    .write = %s_write,
    .ioctl = %s_ioctl,
    .open = %s_open,
    .release = %s_release,
    .poll = %s_poll,
};


static int __init %s_init(void)
{
    pr_info("%s simulate loaded at %%p\\n", MODULE_NAME, %s_init);

    int ret;
    dev_t devno = MKDEV(%s_major, 0);
    if (%s_major) {
        ret = register_chrdev_region(devno, 1, "%s");
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, "%s");
        %s_major = MAJOR(devno);
    }
    if (ret < 0)
        return ret;

    devp = kmalloc(sizeof(struct %s_dev), GFP_KERNEL);
    if (!devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    memset(devp, 0, sizeof(struct %s_dev));

    // setup cdev
    int err, devno = MKDEV(%s_major, 0);

    cdev_init(&devp->cdev, &%s_fops);
    devp->cdev.owner = THIS_MODULE;
    devp->cdev.ops = &%s_fops;
    err = cdev_add(&devp->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %%d adding LED%%d", err, 0);

    pr_info("%s simulate loaded at %%p\\n", MODULE_NAME, %s_init);

    init_MUTEX(&(devp->mutex));
    return 0;

fail_malloc: 
    unregister_chrdev_region(devno, 1);
    return ret;
}

void __exit %s_exit(void)
{
    cdev_del(&devp->cdev);   /*注销cdev*/
    kfree(devp);     /*释放设备结构体内存*/
    unregister_chrdev_region(MKDEV(%s_major, 0), 1); /*释放设备号*/
    pr_info("%s simulate unload at %%p\\n", MODULE_NAME, %s_exit);
}

module_init(%s_init);
module_exit(%s_exit);
'''

makefile = '''
MODULE_NAME := %s



ifneq ($(KERNELRELEASE),) 	# kernelspace

obj-m := $(MODULE_NAME).o

else						# userspace


LINUX_KERNEL ?= $(shell uname -r)
LINUX_KERNEL_PATH ?= /lib/modules/$(LINUX_KERNEL)/build

CURRENT_PATH ?= $(shell pwd)
CFG_INC = $(CURRENT_PATH)
MODCFLAGS:=-O2 -Wall -DMODULE -D__KERNEL__ -DLINUX -std=c99
EXTRA_CFLAGS  += $(MODULE_FLAGS) -I $(CFG_INC)


modules:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules

modules_install:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) modules_install

insmod:
	sudo insmod $(MODULE_NAME).ko %s_major=300
	sudo mknod /dev/%s c 300 0
	sudo chmod 666 /dev/%s

rmmod:
	sudo rmmod $(MODULE_NAME)
	sudo rm -rf /dev/%s

github:
	cd $(ROOT) && make github

test :
	echo "test device driver %s" > /dev/%s
	cat /dev/%s

clean:
	make -C $(LINUX_KERNEL_PATH) M=$(CURRENT_PATH) clean
	rm -f modules.order Module.symvers Module.markers

.PHNOY:
	modules modules_install clean
	
endif
'''


os.mkdir(projectname)
codepath = projectname+'/'+projectname+'.c'
makefilepath = projectname+'/Makefile'

with open(codepath, 'w') as f:
    f.write(code%(projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname))

with open(makefilepath, 'w') as f:
    f.write(makefile%(projectname,projectname,projectname,
        projectname,projectname,projectname,projectname,projectname))

print('generate char device format in ./%s'%projectname)