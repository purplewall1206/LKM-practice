#include <linux/module.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/gfp.h>
#include <linux/hugetlb.h>
#include <asm/pgalloc.h>
#include <asm/pgtable.h>
#include <asm/tlb.h>
#include <asm/fixmap.h>
#include <asm/mtrr.h>
#include <linux/sched/task.h>

//  Define the module metadata.
#define MODULE_NAME "hello"
MODULE_AUTHOR("Dave Kerr");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("A simple kernel module to greet a user");
MODULE_VERSION("0.1");

//  Define the name parameter.
static char *name = "Bilbo";
module_param(name, charp, S_IRUGO);
MODULE_PARM_DESC(name, "The name to display in /var/log/kern.log");

static unsigned long addr = 0x111;
module_param(addr, ulong, S_IRUGO);

static int __init hello_init(void)
{
    pr_info("%s: module loaded at 0x%x  0x%lx\n", MODULE_NAME, hello_init, addr);
    pr_info("%s: greetings %s\n", MODULE_NAME, name);
	//pgd_t * swapper_addr = (pgd_t *)0xffffffff9c60a000;
	
	//pr_info("init_top_gdt : addr->%lx  value->%lx  %lx  %lx      %lx %lx\n",swapper_addr, swapper_addr[0], swapper_addr[1], swapper_addr[2]);

	//pr_info("init_top_gdt : %lx\n", &swapper_pg_dir);

	pr_info("init-task : %lx  %s\n", &init_task, init_task.comm);
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("%s: goodbye %s\n", MODULE_NAME, name);
    pr_info("%s: module unloaded from 0x%lx\n", MODULE_NAME, hello_exit);
}

module_init(hello_init);
module_exit(hello_exit);
