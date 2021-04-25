
#include <linux/init.h>
#include <linux/module.h>
#include <asm/current.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/seq_file.h>
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ppw");

#define MODULE_NAME "procfs_rw"

int procfs_rw_show(struct seq_file *seq, void *offset ) 
{
    char *msg = "this is test\n";
    // count = strlen(msg);
    seq_printf(seq, msg);
    return 0;
}

static int procfs_rw_open(struct inode *inode, struct file *file)
{
	return single_open(file, procfs_rw_show, NULL);
}

const struct proc_ops proc_fops = {
    .proc_open		= procfs_rw_open,
	.proc_read		= seq_read,
};

static int __init procfs_rw_init(void)
{
    pr_info("procfs_rw simulate loaded at %p\n", MODULE_NAME, procfs_rw_init);
    proc_create("procfs_rw", 0, NULL, &proc_fops);
    return 0;
}

void __exit procfs_rw_exit(void)
{
    pr_info("procfs_rw simulate unload at %p\n", MODULE_NAME, procfs_rw_exit);
    remove_proc_entry("procfs_rw", NULL);
}

module_init(procfs_rw_init);
module_exit(procfs_rw_exit);
