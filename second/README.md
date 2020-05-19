# 内核时钟编程

1. init_timer 更换为 timer_setup, 更具体的在 Note 内核近期改变 里面，直接去找。

2. add_timer被省略，因此初次设置时延由mod_timer 触发

3. 内核sleep函数在 #include <linux/delay.h> 里面
    常用的有 ndelay()   udelay()   mdelay() 从小到大分别表示纳秒 微秒  毫秒
    用户态的 usleep() 和 sleep() 在 #include <linux/unistd.h>


4. 定时器的到期时间往往是在目前jiffies的基础上添加一个时延，若 为Hz，则表示延迟1s
    循环定时的方法是每次到期之后再设置一次到期时间。

5. **jiffies是记录着从电脑开机到现在总共的时钟中断次数。**