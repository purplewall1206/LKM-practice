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

## 结论

字符设备里面互斥区比信号量似乎也没啥显著区别，但是这次只是在用户态测出来的，fifo的直接在内核里面测试，看看区别

