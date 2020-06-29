#include <linux/mm.h>	
#include <linux/sched.h>	
#include <linux/module.h>	
#include <linux/sched/signal.h>
#include <linux/sched/task.h>
static int pid = 1;	
module_param(pid, int, 0644);	


static int hack_thread_init(void)
{
    struct task_struct *task;
    // pte_t* pte;
    // struct page* page;
    // task = pid_task(find_pid_ns(pid, &init_pid_ns), PIDTYPE_PID);
    // pr_info("%s: mm->%lx, active_mm->%lx\n", task->comm, task->mm, task->active_mm);
    int i = 0;
    // int pids[10] = {1, 2, 6,  10,22, 261, 3863, 3864};
    // for (i = 0;i < 8;i++) {
    //     task = pid_task(find_pid_ns(pids[i], &init_pid_ns), PIDTYPE_PID);
    //     pr_info("%d:%s mm->%lx, active_mm->%lx\n", pids[i], task->comm, task->mm, task->active_mm);
    // }
    // task = pid_task(find_pid_ns(1, &init_pid_ns), PIDTYPE_PID);
    pr_info("%d:%s mm->%lx, active_mm->%lx\n", init_task.pid, init_task.comm, 
                init_task.mm, init_task.active_mm);
    for_each_process(task) {
        if (i++ < 10) {
            pr_info("%d:%s mm->%lx, active_mm->%lx\n", task->pid, task->comm, task->mm, task->active_mm);
        } else {
            continue;
        }
    }
    return 0;
}

static void hack_thread_exit(void)
{

}

module_init(hack_thread_init);
module_exit(hack_thread_exit);
MODULE_LICENSE("GPL");