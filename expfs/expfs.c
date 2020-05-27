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
#include <linux/uaccess.h>
#include <linux/delay.h>

MODULE_LICENSE("GPL");

#define MAX_BUF 256
#define MAX_FILE 50
#define MAX_NAME 50
#define BUFFER_SIZE 256

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
    char buffer[BUFFER_SIZE];
};

int usedBlks = 0; // 统计已经使用的文件块数量

struct expfs_fileblock * blks;  // 存储文件块 [0]->superblock  [1]->welcomefile

const struct inode_operations expfs_iops;

const struct file_operations expfs_fops;

const struct file_operations expfs_dir_fops;

// 获取空文件块
static int getblock (void)
{
    for (int i = 2;i < MAX_FILE;i++) {
        pr_info("%s: %d used %d\n", __func__, i, blks[i].used);
        if (blks[i].used == 0) {
            blks[i].used = 1;
            return i;
        }
    }
    // 没有空文件块
    return -1;
}

static struct inode * expfs_geti(struct super_block *sb, int index) 
{
    struct inode *inode;
    struct expfs_fileblock *blk;
    inode = new_inode(sb);
    inode->i_ino = index;
    inode->i_sb = sb;
    inode->i_op = &expfs_iops;

    blk = &blks[index];

    if (S_ISDIR(blk->mode)) {
        inode->i_fop = &expfs_dir_fops;
    } else if (S_ISREG(blk->mode)) {
        inode->i_fop = &expfs_fops;
    }

    // struct timespec64 current_time;
    // ktime_get_ts64(&current_time);
    inode->i_atime = inode->i_ctime = inode->i_mtime = current_time(inode);
    inode->i_private = blk;

    return inode;
}

/*=============== dir file operations =========================*/
// int count_iter = 0;
static int expfs_iterate(struct file *filp, struct dir_context *ctx)
{
    // mdelay(2000);
    // pr_info("%s  repeat %d\n", __func__, count_iter++);
    struct expfs_fileblock *blk;
    struct expfs_direntry *entry;
    // int i;
    // struct super_block *sb = filp->f_inode->i_sb;
    loff_t pos = filp->f_pos;
    if (pos) return 0;

	printk(KERN_INFO "%s : Iterate on inode [%lu]\n",
	       __func__, filp->f_inode->i_ino);

    // blk = (struct expfs_fileblock *) filp->f_path.dentry->d_inode->i_private;
    blk = (struct expfs_fileblock *) filp->f_path.dentry->d_inode->i_private;

    if (!S_ISDIR(blk->mode))
        return -ENOTDIR;

    entry = (struct expfs_direntry *) &blk->buffer[0];
    pr_info("%s mode:%d,  dir_children:%d\n", __func__, S_ISDIR(blk->mode), blk->dir_children);
    if (!dir_emit_dots(filp, ctx)) {
        pr_err("%s current or upper directory load failed\n", __func__);
    } 
    for (int i = 0;i < blk->dir_children;i++) {
        pr_info("expfs %s iterate  %d  : %s\n", __func__, entry[i].index, entry[i].name);
        dir_emit(ctx, entry[i].name, sizeof(entry[i].name), entry[i].index, DT_UNKNOWN);
        ++ctx->pos;
        filp->f_pos += sizeof(struct expfs_direntry);
        pos += sizeof(struct expfs_direntry);
    }
    return 0;
}

const struct file_operations expfs_dir_fops = {
    .owner = THIS_MODULE,
    .iterate = expfs_iterate,
};


/*=============== file operations =========================*/

ssize_t expfs_read (struct file *filp, char __user * buf, size_t len, loff_t *ppos)
{
    struct expfs_fileblock *blk;
    char *buffer;
    blk = (struct expfs_fileblock *) filp->f_path.dentry->d_inode->i_private;
    pr_info("%s : read file i_no %d\n", __func__, blk->index);
    if (*ppos >= blk->filesize) 
        return 0;
    
    buffer = (char *) &blk->buffer[0];
    len = min((size_t)blk->filesize, len);
    if (copy_to_user(buf, buffer, len)) {
        return -EFAULT;
    }

    *ppos += len;
    return len;
}

ssize_t expfs_write (struct file * filp, const char __user * buf, size_t len, loff_t * ppos)
{
    pr_info("%s : filename %s, content:%s,len:%ld\n",__func__, filp->f_path.dentry->d_name.name, buf, len);
    struct expfs_fileblock *blk;
    char *buffer;
    blk = (struct expfs_fileblock *) filp->f_path.dentry->d_inode->i_private;
    pr_info("%s : write file i_no %d  %ld\n", __func__, blk->index, filp->f_path.dentry->d_inode->i_ino);
    buffer = (char *) &blk->buffer[0];
    buffer += *ppos;

    if (copy_from_user(buffer, buf, len)) {
        return -EFAULT;
    }
    // strcpy(blk->buffer, buffer);
    pr_info("%s buffer %s \n --%s\n", __func__, buffer, blk->buffer);
    *ppos += len;
    blk->filesize = *ppos;

    return len;
}

const struct file_operations expfs_fops = {
    .read = expfs_read,
    .write = expfs_write,
};

/*================= inode operations ====================*/

static int expfs_do_create(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    struct inode *inode;
    struct super_block *sb = dir->i_sb;
    struct expfs_direntry *entry;
    struct expfs_fileblock *blk, *pblk;
    int idx ;

    if (usedBlks >= MAX_FILE) 
        return -EINVAL;
    if (!S_ISDIR(mode) && !S_ISREG(mode))
        return -EINVAL;

    inode = new_inode(sb);
    if (!inode)
        return -ENOMEM;

    inode->i_sb = sb;
    inode->i_op = &expfs_iops;
    // struct timespec64 current_time;
    // ktime_get_ts64(&current_time);
    inode->i_atime = inode->i_ctime = inode->i_mtime = current_time(inode);

    idx = getblock();
    blk = &blks[idx];
    inode->i_ino = idx;
    blk->index = idx;
    blk->mode = mode;
    blk->used = 1;
    usedBlks++;
    pr_info("%s get block index %d, used blocks %d\n", __func__, idx, usedBlks);

    if (S_ISDIR(mode)) {
        blk->dir_children = 0;
        inode->i_fop = &expfs_dir_fops;
    } else if (S_ISREG(mode)) {
        blk->filesize  = 0;
        inode->i_fop = &expfs_fops;
    }

    inode->i_private = blk;
    pblk = (struct expfs_fileblock * ) dir->i_private;
    pr_info("%s pblk : %d\n", __func__, pblk->index);
    entry = (struct expfs_direntry *) &pblk->buffer[0];
    entry += pblk->dir_children;
    pblk->dir_children++;
    entry->index = idx;
    strcpy(entry->name, dentry->d_name.name);

    inode_init_owner(inode, dir, mode);
    d_add(dentry, inode);

    return 0;
}

struct dentry * expfs_lookup (struct inode *parent_inode,struct dentry *child_dentry, unsigned int flags)
{
    struct super_block *sb;
    struct expfs_fileblock *blk;
    struct expfs_direntry *entry;

    blk = (struct expfs_fileblock *) parent_inode->i_private;
    pr_info("%s expfs lookup index %d, childname: %p\n", __func__, blk->index, child_dentry->d_name.name);
    entry = (struct expfs_direntry *) &blk->buffer[0];
    for (int i = 0;i < blk->dir_children;i++) {
        if (!strcmp(entry[i].name, child_dentry->d_name.name)) {
            struct inode *inode = expfs_geti(sb, entry[i].index);
            struct expfs_fileblock *inner = (struct expfs_fileblock *) inode->i_private;
            inode_init_owner(inode, parent_inode, inner->mode);
            d_add(child_dentry, inode);
            return NULL;
        }
    }
    return NULL;
}

int expfs_create (struct inode * dir,struct dentry * dentry, umode_t mode, bool excl)
{
    pr_info("%s  inode : %ld, dentryname : %s\n", __func__, dir->i_ino, dentry->d_name.name);
    return expfs_do_create(dir, dentry, mode);
}

// int expfs_unlink (struct inode *,struct dentry *);

int expfs_mkdir (struct inode *dir ,struct dentry *dentry, umode_t mode) 
{
    pr_info("%s\n", __func__);
    return expfs_do_create(dir, dentry, S_IFDIR | mode);
}

int expfs_rmdir (struct inode *dir,struct dentry *dentry)
{
    struct inode *inode = dentry->d_inode;
    pr_info("%s dir inode:%ld\n", __func__, inode->i_ino);
    struct expfs_fileblock *blk = (struct expfs_fileblock * ) inode->i_private;
    struct expfs_fileblock *pblk = dir->i_private;
    int ret = 0;
    // blk->used = 0;

    // TODO
    // 1. check empty
    pr_info("%s parent blk-> index:%d, mode:%d, children:%d\n", __func__, blk->index, S_ISDIR(blk->mode), blk->dir_children);
    pr_info("%s blk -> index:%d, mode:%d, children:%d\n", __func__, blk->index, S_ISDIR(blk->mode), blk->dir_children);
    struct expfs_direntry *entry = (struct expfs_direntry *) pblk->buffer;

    // 2. is empty dir?
    if (S_ISDIR(blk->mode) && blk->dir_children == 0) {
        blk->used = 0;

        // parent entry erase
        for (int i = 0;i < pblk->dir_children;i++) {
            if (!strcmp(entry[i].name, dentry->d_name.name)) {
                for (int j = i;j < pblk->dir_children-1; j++) {
                    memcpy(&entry[j], &entry[j+1], sizeof(struct expfs_direntry));
                }
                pblk->dir_children--;
                break;
            }
        }

        ret = simple_rmdir(dir, dentry);
        pr_info("%s simple_rmdir ret:%d\n", __func__, ret);

    } else {
        pr_info("%s dir not empty\n", __func__);
        return -ENOTEMPTY;
    }
    // return simple_rmdir(dir, dentry);
    return ret;
}

int expfs_unlink(struct inode *dir, struct dentry *dentry)
{
    struct inode *inode = dentry->d_inode;
    struct expfs_fileblock *blk = inode->i_private;
    struct expfs_fileblock *pblk = dir->i_private;
    struct expfs_direntry *entry;
    pr_info("%s unlink : %s\n", __func__, dentry->d_name.name);

    entry = (struct expfs_direntry *)&pblk->buffer[0];
    for (int i = 0;i < pblk->dir_children;i++) {
        if (!strcmp(entry[i].name, dentry->d_name.name)) {
            for (int j = i;j < pblk->dir_children - 1;j++) {
                memcpy(&entry[j], &entry[j+1], sizeof(struct expfs_direntry));
            }
            pblk->dir_children --;
            break;
        }
    }
    blk->used = 0;
    return simple_unlink(dir, dentry);
}

const struct inode_operations expfs_iops = {
    .create = expfs_create,
    .mkdir = expfs_mkdir,
    .rmdir = expfs_rmdir,
    .lookup = expfs_lookup,
    .unlink = expfs_unlink,
};



/*================= expfs type =========================*/
// 没从块设备读取超级块，所以没有super_operation
// 填充超级块
int expfs_fill_super(struct super_block *sb, void *data, int silent) 
{
    int ret = 0;
    struct inode *root_inode;
    mode_t mode = S_IFDIR;
    // struct timespec64 curr;

    root_inode = new_inode(sb);
    root_inode->i_ino = 1;
    inode_init_owner(root_inode, NULL, mode);
    root_inode->i_sb = sb;
    root_inode->i_op = &expfs_iops;
    root_inode->i_fop = &expfs_dir_fops;
    // ktime_get_ts64(&curr);
    root_inode->i_atime = root_inode->i_mtime = root_inode->i_ctime = current_time(root_inode);
    
    // iterate 函数显示 file指针里面的inode->i_ino 为1 ，所以这里必须定义为1
    blks[1].mode = mode;
    blks[1].dir_children = 0;
    blks[1].index = 1;
    blks[1].used = 1;
    root_inode->i_private = &blks[1];

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
    // unsigned int blksize = sizeof(struct expfs_fileblock) * MAX_FILE;
    // usedBlks = 0;

    blks = vmalloc((sizeof(struct expfs_fileblock)) * MAX_FILE);
    memset(blks, 0, (sizeof(struct expfs_fileblock)) * MAX_FILE);
    ret = register_filesystem(&expfs_type);
    if (ret) {
        pr_err("register filesystem failed\n");
    }
    pr_info("%s : load expfs at %p\nAllocate 0x%lx bytes\n", __func__, expfs_init, sizeof(blks));
    return ret;
}

static void __exit expfs_exit(void)
{
    // vfree(blks);
    unregister_filesystem(&expfs_type);
    pr_info("%s : unload expfs at %p\n", __func__, expfs_exit);
}

module_init(expfs_init);
module_exit(expfs_exit);

