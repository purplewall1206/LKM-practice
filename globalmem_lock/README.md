# 字符型设备写入信号量，互斥区时间对比

 使用 `#if defined(init_MUTEX)` 决定使用信号量或是互斥区

测试程序 `test2.c` 分别测试10000次，由于时间太短，取总时间（us）。

写入数据为 `"1111111111111111111122aaaaaaaaaaaaaabbbbbbbbbbbbbbbbbbbccccccccccc"` 共66个字符（可以多测一些，但是没必要）

### 信号量

|time|1|2|3|
|---|---|---
|write | 4800 us|5800 |5592|
|read  | 8722 us|10933|10967|
|ioctl | 6439 us|6927|6779|


### 互斥区

write : 7796 us   
read  : 9227 us   
ioctl : 6108 us 

write : 5337 us   
read  : 9998 us   
ioctl : 6308 us   

write : 5233 us   
read  : 16077 us   
ioctl : 6399 us   

## 在内核中进行测试

```
#include <linux/posix-clock.h>
long timer_end(struct timespec start_time)
{
    struct timespec end_time;
    getrawmonotonic(&end_time);
    return(end_time.tv_nsec - start_time.tv_nsec);
}

struct timespec timer_start(void)
{
    struct timespec start_time;
    getrawmonotonic(&start_time);
    return start_time;
}
```

根据 [stackoverflow](https://stackoverflow.com/questions/4655711/measuring-execution-time-of-a-function-inside-linux-kernel) 上述方法准确率较高，延迟较低。

## 测试结果（50次读写）

[11360.104069] read timer : 386 ns  
[11404.228719] write timer -------------------: 765 ns  
[11404.228723] read timer : 824 ns  
[11404.228730] read timer : 438 ns  
[11404.228733] read timer : 381 ns  
[11404.228736] read timer : 380 ns  
[11404.228740] read timer : 380 ns   
[11404.228743] read timer : 385 ns  
[11404.228746] read timer : 385 ns  
[11404.228749] read timer : 380 ns  
[11404.228753] read timer : 384 ns  
[11404.228756] read timer : 382 ns  

由于加锁机制读执行了11次，写只执行了1次，显然所需时间比在用户态中测试低了不少，多出的时间就是系统调用，上下文切换的时间。

## 结论

字符设备里面互斥区比信号量似乎也没啥显著区别，但是这次只是在用户态测出来的，fifo的直接在内核里面测试，看看区别

