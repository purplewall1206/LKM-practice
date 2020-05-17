#include <linux/init.h>
#include <linux/module.h>
#include <asm/current.h>
MODULE_LICENSE("GPL v2");

// 思路 ：
// 1.构建 ioctl 把模拟的端口号当中断号发过去
// 2.如果不行构建write和read，write发送中断和数据，read获取得到的数据

// ***********写之前用python构建一个模板，下次想写直接输出，省得再写一遍*****************

#define BUFSIZE  0x1000
static int intr = 300;
module_param(intr, int, S_IRUGO);

struct intr_sim_dev 
{
    struct cdev   dev;
    char   buf[BUFSIZE];
    struct mutex mutex;
}

struct intr_sim_dev*  devp = NULL:

int intr_sim_open(struct inode *inode, struct file *filp) 
{
    filp->private_data = devp;
    pr_info("%s opened by %d : %s\n", MODULE_NAME, current->pid, current->comm);
    return 0;
}

int intr_sim_release(struct inode *inode, struct file *filp)
{
    return 0;
}

static long intr_sim_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    struct inode *inode = inode = file_inode(filp);
    mutex_lock(&dev->mutex);
    mutex_unlock(&dev->mutex);
    return 0;
}

static ssize_t intr_sim_read(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    mutex_lock(&dev->mutex);
    mutex_unlock(&dev->mutex);
    return 0;
}

static ssize_t intr_sim_write(struct file *filp,char __user *buf,
                        size_t      size,loff_t      *ppos)
{
    mutex_lock(&dev->mutex);
    mutex_unlock(&dev->mutex);
    return 0;
}

static loff_t intr_sim_llseek(struct file *filp, loff_t offset, int orig)
{
    return 0;
}


static const struct file_operations intr_sim_fops = 
{
    .owner = THIS_MODULE,
    .llseek = intr_sim_llseek,
    .read = intr_sim_read,
    .write = intr_sim_write,
    .ioctl = intr_sim_ioctl,
    .open = intr_sim_open,
    .release = intr_sim_release,
};


static int __init intr_sim_init(void)
{
    pr_info("%s simulate loaded at %p\n", MODULE_NAME, intr_sim_init);

    init_MUTEX(&(devp->mutex));
    return 0;
}

static int __exit intr_sim_exit(void)
{
    pr_info("%s simulate unload at %p\n", MODULE_NAME, intr_sim_exit);
}

module_init(intr_sim_init);
module_exit(intr_sim_exit);