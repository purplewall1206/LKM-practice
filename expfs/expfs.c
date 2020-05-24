// 初步设计思路应该是个内存文件系统

// 下一步进化为硬盘文件系统，首先搞定整体结构 super_block， 等

// 文件系统变化也非常大，最好先找到可以运行的研究下怎么搞定之后的

#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <asm/current.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/time.h>
#include <linux/time64.h>

MODULE_LICENSE("GPL");

#define MAX_BUF 256
#define MAX_FILE 50
#define MAX_NAME 50

struct expfs_direntry
{
    char name[50];
    int index;
};

struct expfs_fileblock
{
    int used;
    mode_t mode;
    int index;
    union {
        int filesize;
        int dir_children;
    };
    char buffer[MAX_BUF];
};

int usedBlks = 0; // 统计已经使用的文件块数量

struct expfs_fileblock *blks;  // 存储文件块 [0]->superblock  [1]->welcomefile

const struct inode_operations expfs_iops;

const struct file_operations expfs_fops;

const struct file_operations expfs_dir_fops;

// 获取空文件块
static int getblock (void)
{
    for (int i = 1;i < MAX_FILE;i++) {
        if (blks[i].used != 1) {
            blks[i].used = 1;
            return i;
        }
    }
    // 没有空文件块
    return -1;
}



/*=============== dir file operations =========================*/

static int expfs_iterate(struct file *filp, struct dir_context *ctx)
{
    struct expfs_fileblock *blk;
    struct expfs_direntry *entry;
    int i;
    // struct super_block *sb = filp->f_inode->i_sb;

	printk(KERN_INFO "expfs : Iterate on inode [%lu]\n",
	       filp->f_inode->i_ino);

    // blk = (struct expfs_fileblock *) filp->f_path.dentry->d_inode->i_private;
    blk = (struct expfs_fileblock *) file_dentry(filp)->d_inode->i_private;

    if (!S_ISDIR(blk->mode))
        return -ENOTDIR;

    entry = (struct expfs_direntry *) &blk->buffer;
    for (i = 0;i < blk->dir_children;i++) {
        pr_info("expfs %s iterate  %d  : %s\n", __func__, entry[i].index, entry[i].name);
        dir_emit(ctx, entry[i].name, sizeof(struct expfs_fileblock), entry[i].index, DT_UNKNOWN);
        ++ctx->pos;
    }
    return 0;
}

const struct file_operations expfs_dir_fops = {
    .owner = THIS_MODULE,
    .iterate = expfs_iterate,
};


/*=============== file operations =========================*/

ssize_t expfs_read (struct file *, char __user *, size_t, loff_t *)
{

}

ssize_t expfs_write (struct file *, const char __user *, size_t, loff_t *)
{

}

const struct file_operations expfs_fops = {
    .read = expfs_read,
    .write = expfs_write,
}




/*================= expfs type =========================*/
// 没从块设备读取超级块，所以没有super_operation
// 填充超级块
int expfs_fill_super(struct super_block *sb, void *data, int silent) 
{
    int ret = 0;
    struct inode *root_inode;
    mode_t mode = S_IFDIR;
    struct timespec64 curr;

    root_inode = new_inode(sb);
    root_inode->i_ino = 1;
    inode_init_owner(root_inode, NULL, mode);
    root_inode->i_sb = sb;
    root_inode->i_op = &expfs_iops;
    root_inode->i_fop = &expfs_dir_fops;
    ktime_get_ts64(&curr);
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = curr;
    
    blks[0].mode = mode;
    blks[0].dir_children = 0;
    blks[0].index = 0;
    blks[0].used = 1;
    root_inode->i_private = &blks[0];

    sb->s_root = d_make_root(root_inode);
    usedBlks++;
    return ret;
}

// 挂载
static struct dentry *expfs_mount (struct file_system_type *fs_type, int flags,
		       const char *dev_name, void *data) 
{
    pr_info("%s : mounted \n", __func__);
    return mount_nodev(fs_type, flags, data, expfs_fill_super);
}

static void expfs_kill_sb (struct super_block *sb)
{
    pr_info("%s : kill_superblock\n", __func__);
    kill_anon_super(sb);
}


static struct file_system_type expfs_type = {
    .owner = THIS_MODULE,
    .name = "expfs",
    .mount = expfs_mount,
    .kill_sb = expfs_kill_sb,
};



static int __init expfs_init(void)
{
    int ret = 0;
    unsigned int blksize = sizeof(struct expfs_fileblock) * MAX_FILE;
    usedBlks = 0;

    blks = vmalloc(blksize);
    memset(blks, 0, blksize);
    ret = register_filesystem(&expfs_type);
    if (ret) {
        pr_err("register filesystem failed\n");
    }
    pr_info("%s : load expfs at %p\nAllocate 0x%x bytes\n", __func__, expfs_init, blksize);
    return ret;
}

static void __exit expfs_exit(void)
{
    vfree(blks);
    unregister_filesystem(&expfs_type);
    pr_info("%s : unload expfs at %p\n", __func__, expfs_exit);
}

module_init(expfs_init);
module_exit(expfs_exit);

