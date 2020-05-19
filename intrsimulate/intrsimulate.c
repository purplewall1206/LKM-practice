
#include <linux/init.h>
#include <linux/module.h>
#include <asm/current.h>
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ppw");
#define BUFSIZE  0x1000
static int intr_major = 300;
module_param(intr_major, int, S_IRUGO);

struct intrsimulate_dev 
{
    struct cdev   dev;
    char   buf[BUFSIZE];
    struct mutex mutex;
}

struct intrsimulate_dev*  devp = NULL:

int intrsimulate_open(struct inode *inode, struct file *filp) 
{
    filp->private_data = devp;
    pr_info("intrsimulate opened by %d : intrsimulate\n", MODULE_NAME, current->pid, current->comm);
    return 0;
}

int intrsimulate_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long intrsimulate_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct inode *inode = inode = file_inode(filp);
    struct intrsimulate_dev *dev = filp->private_data;
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return 0;
}

static ssize_t intrsimulate_read(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    struct intrsimulate_dev *dev = filp->private_data;
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return 0;
}

static ssize_t intrsimulate_write(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    struct intrsimulate_dev *dev = filp->private_data;
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return 0;
}

static loff_t intrsimulate_llseek(struct file *filp, loff_t offset, int orig)
{
    return 0;
}

static unsigned int intrsimulate_poll(struct file *filp, poll_table *wait)
{
    unsigned int mask = 0;
    struct intrsimulate_dev *dev = filp->private_data; 
    mutex_lock(&dev->mutex);

    mutex_unlock(&dev->mutex);
    return mask;
}

static const struct file_operations intrsimulate_fops = 
{
    .owner = THIS_MODULE,
    .llseek = intrsimulate_llseek,
    .read = intrsimulate_read,
    .write = intrsimulate_write,
    .ioctl = intrsimulate_ioctl,
    .open = intrsimulate_open,
    .release = intrsimulate_release,
    .poll = intrsimulate_poll,
};


static int __init intrsimulate_init(void)
{
    pr_info("intrsimulate simulate loaded at %p\n", MODULE_NAME, intrsimulate_init);

    int ret;
    dev_t devno = MKDEV(intrsimulate_major, 0);
    if (intrsimulate_major) {
        ret = register_chrdev_region(devno, 1, "intrsimulate");
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, "intrsimulate");
        intrsimulate_major = MAJOR(devno);
    }
    if (ret < 0)
        return ret;

    devp = kmalloc(sizeof(struct intrsimulate_dev), GFP_KERNEL);
    if (!devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    memset(devp, 0, sizeof(struct intrsimulate_dev));

    // setup cdev
    int err, devno = MKDEV(intrsimulate_major, 0);

    cdev_init(&devp->cdev, &intrsimulate_fops);
    devp->cdev.owner = THIS_MODULE;
    devp->cdev.ops = &intrsimulate_fops;
    err = cdev_add(&devp->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding LED%d", err, 0);

    pr_info("intrsimulate simulate loaded at %p
", MODULE_NAME, intrsimulate_init);

    init_MUTEX(&(devp->mutex));
    return 0;

fail_malloc: 
    unregister_chrdev_region(devno, 1);
    return ret;
}

void __exit intrsimulate_exit(void)
{
    cdev_del(&devp->cdev);   /*注销cdev*/
    kfree(devp);     /*释放设备结构体内存*/
    unregister_chrdev_region(MKDEV(intrsimulate_major, 0), 1); /*释放设备号*/
    pr_info("intrsimulate simulate unload at %p\n", MODULE_NAME, intrsimulate_exit);
}

module_init(intrsimulate_init);
module_exit(intrsimulate_exit);
