// 初步设计思路应该是个内存文件系统

// 下一步进化为硬盘文件系统，首先搞定整体结构 super_block， 等

// 文件系统变化也非常大，最好先找到可以运行的研究下怎么搞定之后的

#include <linux/init.h>
#include <linux/types.h>
#include <linux/module.h>
#include <asm/current.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>

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

struct expfs_fileblock *blks;  // 存储文件块

// 填充超级块
static int expfs_fill_super(struct super_block *sb, void *data, int silent) 
{
    int ret;

    return ret;
}

// 挂载
static struct dentry *expfs_mount (struct file_system_type *fs_type, int flags,
		       const char *dev_name, void *data) 
{
    return mount_nodev(fs_type, flags, data, expfs_fill_super);
}

static void expfs_kill_sb (struct super_block *sb)
{
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
    usedBlks = 0;
    unsigned int blksize = sizeof(struct expfs_fileblock) * MAX_FILE;
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

MODULE_LICENSE("GPL");