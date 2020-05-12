#include <linux/module.h>
// #include <linux/types.h>
// #include <linux/fs.h>
// #include <linux/errno.h>
// #include <linux/mm.h>
// #include <linux/sched.h>
#include <linux/init.h>
// #include <linux/cdev.h>
// #include <asm/io.h>
// //#include <asm/system.h>
// #include <asm/uaccess.h>

// #include <linux/version.h>

// 给内核传输一个64位虚拟地址
// sudo cat /proc/kallsyms | grep modules
static unsigned long addr = 0x0;
module_param(addr, ulong, S_IRUGO);

static char* module_name = "globalmem";
module_param(module_name, charp, S_IRUGO);

static int __init unload_init(void)
{
    pr_info("unload_init  %lx   %s\n", addr, module_name);
    struct list_head *modules=(struct list_head *)addr;
    struct module *mod=0;
    struct module *list_mod;
    // int i;
    // int zero=0;
    
    list_for_each_entry(list_mod,modules,list){
        if(strcmp(list_mod->name,module_name) == 0)
            mod=list_mod;
    }


    mod->state=MODULE_STATE_LIVE;
    uint64_t refcnt = atomic_read(&(mod->refcnt)); 
    pr_info("result : %s   %d   %ld\n", mod->name, mod->state, refcnt);
    atomic_set(&(mod->refcnt), 1); 
    // for (i = 0; i < NR_CPUS; i++){
    //     // mod->ref[i].count=*(local_t *)&zero;
    // }
    return 0;
}

static void __exit unload_exit(void)
{
    pr_info("unload_exit\n");
}

module_init(unload_init);
module_exit(unload_exit);