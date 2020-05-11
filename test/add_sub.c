#include <linux/init.h>
#include <linux/module.h>
#include "add_sub.h"
MODULE_LICENSE("Dual DSP/GPL");

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

EXPORT_SYMBOL(add_integer);
EXPORT_SYMBOL(sub_integer);