#include <linux/module.h>
#include <linux/init.h>

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

static unsigned long addr = 0x0;
module_param(addr, ulong, S_IRUGO);

static int __init hello_init(void)
{
    pr_info("%s: module loaded at 0x%p   0x%lx\n", MODULE_NAME, hello_init, addr);
    pr_info("%s: greetings %s\n", MODULE_NAME, name);
    return 0;
}

static void __exit hello_exit(void)
{
    pr_info("%s: goodbye %s\n", MODULE_NAME, name);
    pr_info("%s: module unloaded from 0x%p\n", MODULE_NAME, hello_exit);
}

module_init(hello_init);
module_exit(hello_exit);