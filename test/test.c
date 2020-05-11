#include <linux/init.h>
#include <linux/module.h>
// #include "add_sub.h"

static long a = 1;
static long b = 1;
static int AddOrSub = 1;

long add_integer(long a, long b) 
{
    pr_info("add %ld\n", a+b);
    return a+b;
}

long sub_integer(long a, long b)
{
    pr_info("sub %ld\n", a-b);
    return a- b;
}

static int test_init(void)
{
    long result = 0;
    printk(KERN_ALERT "test init\n");
    if (1 == AddOrSub) {
        result = add_integer(a, b);
    } else {
        result = sub_integer(a, b);
    }
    printk(KERN_ALERT "The %s result is %ld\n", 
            AddOrSub==1 ? "ADD" : "SUB", result);
    return 0;
}

static void test_exit(void)
{
    printk(KERN_ALERT "test exit\n");
}

module_init(test_init);
module_exit(test_exit);
module_param(a, long, S_IRUGO);  
module_param(b, long, S_IRUGO);  
module_param(AddOrSub, int, S_IRUGO); 
/* 描述信息 */                               
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("ppw");
MODULE_DESCRIPTION("A Module for testing module params and EXPORT_SYMBOL");
MODULE_VERSION("V1.0");