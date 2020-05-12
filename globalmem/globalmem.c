#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <asm/io.h>
//#include <asm/system.h>
#include <asm/uaccess.h>

#include <linux/version.h>

#if LINUX_VERSION_CODE <= KERNEL_VERSION(3, 19, 0)
#include <asm/system.h>
#else
#include <linux/slab.h>
#endif

#define GLOBALMEM_SIZE	0x10000	/*全局内存最大40K字节*/
#define MEM_CLEAR 0x1  /*清0全局内存*/
#define GLOBALMEM_MAJOR 301    /*预设的globalmem的主设备号*/

static int globalmem_major = GLOBALMEM_MAJOR;


struct globalmem_dev
{
    struct cdev cdev;
    unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev *globalmem_devp;


// open
int globalmem_open(struct inode *inode, struct file *filp)
{
    /*将设备结构体指针赋值给文件私有数据指针*/
    struct globalmem_dev *dev;

    dev = container_of(inode->i_cdev,struct globalmem_dev,cdev);
    filp->private_data = dev;
    pr_info("globalmem open\n");
    return 0;
}


// release
int globalmem_release(struct inode *inode, struct file *filp)
{
    pr_info("globalmem release\n");
    return 0;
}


// ioctl
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
/* ioctl设备控制函数 */
static int globalmem_ioctl(
        struct inode *inodep,
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
#else
//long (*unlocked_ioctl) (struct file *, unsigned int, unsigned long);
//long (*compat_ioctl) (struct file *file, unsigned int cmd, unsigned long arg)
static long globalmem_compat_ioctl(
        struct file *filp,
        unsigned int cmd,
        unsigned long arg)
{
    //struct inode *inode = file->f_dentry->d_inode;
    struct inode *inode = inode = file_inode(filp);
#endif
    struct globalmem_dev *dev = filp->private_data;/*获得设备结构体指针*/

    switch (cmd)
    {
        case MEM_CLEAR:
            memset(dev->mem, 0, GLOBALMEM_SIZE);
            pr_info("globalmem is set to zero\n");
            break;

        default:
            return  - EINVAL;
    }
    return 0;  
}


// read
static ssize_t globalmem_read(struct file *filp, char __user *buf, size_t size,
  loff_t *ppos)
{
    unsigned long p =  *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/

    /*分析和获取有效的写长度*/
    if (p >= GLOBALMEM_SIZE)
        return count ?  - ENXIO: 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    /*内核空间->用户空间*/
    if (copy_to_user(buf, (void*)(dev->mem + p), count))
    {
        ret =  - EFAULT;
    }
    else
    {
        *ppos += count;
        ret = count;

        pr_info( "read %d bytes(s) from %d\n", count, p);
    }

    return ret;
}


// write
static ssize_t globalmem_write(struct file *filp, const char __user *buf,
  size_t size, loff_t *ppos)
{
    unsigned long p =  *ppos;
    unsigned int count = size;
    int ret = 0;
    struct globalmem_dev *dev = filp->private_data; /*获得设备结构体指针*/

    /*分析和获取有效的写长度*/
    if (p >= GLOBALMEM_SIZE)
        return count ?  - ENXIO: 0;
    if (count > GLOBALMEM_SIZE - p)
        count = GLOBALMEM_SIZE - p;

    /*用户空间->内核空间*/
    if (copy_from_user(dev->mem + p, buf, count))
        ret =  - EFAULT;
    else
    {
        *ppos += count;
        ret = count;

        pr_info("written %d bytes(s) from %d\n", count, p);
    }

    return ret;
}


// seek
static loff_t globalmem_llseek(struct file *filp, loff_t offset, int orig)
{
    loff_t ret = 0;
    switch (orig)
    {
        case 0:   /*相对文件开始位置偏移*/
            if (offset < 0)
            {
                ret =  - EINVAL;
                break;
            }
            if ((unsigned int)offset > GLOBALMEM_SIZE)
            {
                ret =  - EINVAL;
                break;
            }
            filp->f_pos = (unsigned int)offset;
            ret = filp->f_pos;
            break;
        case 1:   /*相对文件当前位置偏移*/
            if ((filp->f_pos + offset) > GLOBALMEM_SIZE)
            {
                ret =  - EINVAL;
                break;
            }
            if ((filp->f_pos + offset) < 0)
            {
                ret =  - EINVAL;
                break;
            }
            filp->f_pos += offset;
            ret = filp->f_pos;
            break;
        default:
            ret =  - EINVAL;
            break;
    }
    return ret;
}


// file struct
static const struct file_operations globalmem_fops =
{
    .owner = THIS_MODULE,
    .llseek = globalmem_llseek,
    .read = globalmem_read,
    .write = globalmem_write,

#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 36)
    .ioctl = globalmem_ioctl,
#else
    //.unlocked_ioctl = globalmem_ioctl,
    .compat_ioctl = globalmem_compat_ioctl,
#endif

    .open = globalmem_open,
    .release = globalmem_release,
};


// initial & register
static void globalmem_setup_cdev(struct globalmem_dev *dev, int index)
{
    int err, devno = MKDEV(globalmem_major, index);
    pr_info("error : %d    devno : %d\n", err, devno);
    cdev_init(&dev->cdev, &globalmem_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &globalmem_fops;
    err = cdev_add(&dev->cdev, devno, 1);
    if (err)
        printk(KERN_NOTICE "Error %d adding LED%d", err, index);
}


// initial driver & insmod
int globalmem_init(void)
{
    int result;
    dev_t devno = MKDEV(globalmem_major, 0);

    /* 申请设备号*/
    if (globalmem_major)
        result = register_chrdev_region(devno, 2, "globalmem");
    else  /* 动态申请设备号 */
    {
        result = alloc_chrdev_region(&devno, 0, 2, "globalmem");
        globalmem_major = MAJOR(devno);
    }
    if (result < 0)
        return result;

    /* 动态申请2个设备结构体的内存*/
    globalmem_devp = kmalloc(2*sizeof(struct globalmem_dev), GFP_KERNEL);
    if (!globalmem_devp)    /*申请失败*/
    {
        result =  - ENOMEM;
        goto fail_malloc;
    }
    memset(globalmem_devp, 0, 2*sizeof(struct globalmem_dev));

    pr_info("globalmem init\n");

    globalmem_setup_cdev(&globalmem_devp[0], 0);
    globalmem_setup_cdev(&globalmem_devp[1], 1);
    return 0;

    fail_malloc: unregister_chrdev_region(devno, 1);
    return result;
}


// rmmod
void globalmem_exit(void)
{
    cdev_del(&(globalmem_devp[0].cdev));
    cdev_del(&(globalmem_devp[1].cdev));   /*注销cdev*/
    kfree(globalmem_devp);     /*释放设备结构体内存*/
    unregister_chrdev_region(MKDEV(globalmem_major, 0), 2); /*释放设备号*/
    pr_info("global exit\n");
}


MODULE_AUTHOR("Song Baohua");
MODULE_LICENSE("Dual BSD/GPL");

module_param(globalmem_major, int, S_IRUGO);

module_init(globalmem_init);
module_exit(globalmem_exit);
