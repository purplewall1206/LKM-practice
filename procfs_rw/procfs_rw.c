
#include <linux/init.h>
#include <linux/module.h>
#include <asm/current.h>
MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("ppw");

#define MODULE_NAME "procfs_rw"



static int __init procfs_rw_init(void)
{
    pr_info("procfs_rw simulate loaded at %p\n", MODULE_NAME, procfs_rw_init);

    return 0;
}

void __exit procfs_rw_exit(void)
{
    pr_info("procfs_rw simulate unload at %p\n", MODULE_NAME, procfs_rw_exit);
}

module_init(procfs_rw_init);
module_exit(procfs_rw_exit);
