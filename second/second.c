
#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
// #include <asm/system.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/timer.h> /*包括timer.h头文件*/
#include <asm/atomic.h> 
#include <asm/current.h>
#include<linux/jiffies.h>
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ppw");
#define BUFSIZE  0x1000
#define MODULE_NAME "second"
static int second_major = 300;
module_param(second_major, int, S_IRUGO);

struct second_dev 
{
    struct cdev   cdev;
    // char   buf[BUFSIZE];
    struct mutex mutex;
    atomic_t counter;
    struct timer_list s_timer;
};

struct second_dev*  devp = NULL;

static void second_timer_handler(struct timer_list *t)
{
    pr_info("hello?\n");
    struct second_dev * sdev = from_timer(sdev, t, s_timer);
    mod_timer(&sdev->s_timer, jiffies+HZ);
    pr_info("did you really work?\n");
    atomic_inc(&sdev->counter);
    printk(KERN_NOTICE "current jiffies is %ld\n", jiffies);
}

int second_open(struct inode *inode, struct file *filp) 
{
    filp->private_data = devp;
    pr_info("second opened by %d : %s\n", current->pid, current->comm);
    // init_timer(&devp->s_timer);
    timer_setup(&devp->s_timer, second_timer_handler, 0);
    // devp->s_timer.function = &second_timer_handler;
    // devp->s_timer.expires = jiffies + HZ;
    pr_info("timer_setup success\n");
    mod_timer(&devp->s_timer, jiffies+HZ);
    // add_timer(&devp->s_timer); /*添加（注册）定时器*/
    pr_info("add_timer success\n");
    atomic_set(&devp->counter,0); //计数清0
    return 0;
}

int second_release(struct inode *inode, struct file *filp)
{
    del_timer(&devp->s_timer);
    return 0;
}

static ssize_t second_read(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    struct second_dev *dev = filp->private_data;
    // mutex_lock(&dev->mutex);
    int counter;
  
    counter = atomic_read(&devp->counter);
    if(put_user(counter, (int*)buf)) {
        pr_info("read wrong happend\n");
        return - EFAULT;
    } else {
        pr_info("read ok\n");
        return sizeof(unsigned int);
    }  
    // mutex_unlock(&dev->mutex);
    return 0;
}




static const struct file_operations second_fops = 
{
    .owner = THIS_MODULE,
    .read = second_read,
    .open = second_open,
    .release = second_release,
};


static int __init second_init(void)
{
    pr_info("second simulate loaded at %p\n", MODULE_NAME, second_init);

    int ret;
    int err;
    dev_t devno = MKDEV(second_major, 0);
    if (second_major) {
        ret = register_chrdev_region(devno, 1, "second");
    } else {
        ret = alloc_chrdev_region(&devno, 0, 1, "second");
        second_major = MAJOR(devno);
    }
    if (ret < 0)
        return ret;

    devp = kmalloc(sizeof(struct second_dev), GFP_KERNEL);
    if (!devp) {
        ret = -ENOMEM;
        goto fail_malloc;
    }
    memset(devp, 0, sizeof(struct second_dev));

    // setup cdev
    err, devno = MKDEV(second_major, 0);

    cdev_init(&devp->cdev, &second_fops);
    devp->cdev.owner = THIS_MODULE;
    devp->cdev.ops = &second_fops;
    err = cdev_add(&devp->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding LED%d", err, 0);

    pr_info("second simulate loaded at %p", second_init);

    // init_MUTEX(&(devp->mutex));
    return 0;

fail_malloc: 
    unregister_chrdev_region(devno, 1);
    return ret;
}

void __exit second_exit(void)
{
    cdev_del(&devp->cdev);   /*注销cdev*/
    kfree(devp);     /*释放设备结构体内存*/
    unregister_chrdev_region(MKDEV(second_major, 0), 1); /*释放设备号*/
    pr_info("second simulate unload at %p\n", second_exit);
}

module_init(second_init);
module_exit(second_exit);
