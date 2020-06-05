# simplefs 简单文件系统


## 第二阶段 借鉴minixfs功能增强阶段

之前思路有点想偏了，为啥要借鉴github上面的不成熟fs，直接借鉴Minixfs不香吗？

github上面的repo存在的问题: 
* [jserv/simplefs](https://github.com/jserv/simplefs)  
* [rgouicem/ouichefs](https://github.com/rgouicem/ouichefs)   
    测试过程中遇到：删除文件夹后重新创建，无法在文件夹内部创建文件。
    ```
    mkdir dir
    echo "test" > dir/a.txt
    cat dir/a.txt
    // delete dir
    rm -rf dir
    
    mkdir dir
    echo "test" > dir/b.txt
    // 文件打开失败，找不到文件
    ```
    
* [krinkinmu/aufs](https://github.com/krinkinmu/aufs)   
* [psankar/simplefs](https://github.com/psankar/simplefs)   
    函数有问题，根本无法编译

### 挂载文件系统脚本

```
mkdir ./mnt
dd if=/dev/zero  of=test.img  bs=10M count=50
mkfs.minix  test.img
sudo mount -o loop -t minix test.img ./mnt
sudo chmod -R 777 ./mnt
cd mnt
```
### minixfs 编译
内核默认不编译minixfs，因此需要在menuconfig -> filesystem -> miscellaneous -> minix  
在偏下的位置，需要仔细找一下。


### 2.1 简化minixfs代码
* minix分成v1 v2 v3 三个版本，我们要做的第一步就是删除掉额外的版本，只保留v3，减少代码量，熟悉文件系统架构

* 动手完成mkfs.minix，先读出来，后重新写入

### 2.2 模仿minixfs写exp1fs

包括pagecache，dcache等特性


## update
2020年6月5日 给文件系统加入锁机制

考虑到本来文件块本来也不大，而且直接写内存，因此使用相对开销较小，非阻塞的自旋锁。


## 第一阶段 内存文件系统阶段



### 1. 设计思路

根据linux文件系统的文件抽象模型， 想要定义一个文件系统首先需要以下数据结构

```
// 文件系统声明
struct file_system_type simplefs_fs_type;
// 超级块
static const struct super_operations simplefs_ops;
// 文件相关
const struct file_operations simplefs_file_operations；
<del>const struct inode_operations simplefs_file_inode_operations；</del>
// 目录相关
const struct file_operations simplefs_dir_operations；
const struct inode_operations simplefs_dir_inode_operations；

// address space
static const struct address_space_operations simplefs_aops;
```

在真正的文件系统中，我们往往将超级块，inode等数据转化一定的struct，安装预先设计好的存储格式，保存的硬盘中。其中还涉及到大量缓存例如 dcache 块cache 和 inode cache ， imap bmap等结构。

待需要读写文件时先通过读取硬盘中保存的文件信息数据写入。

但以上内容留给第二阶段完成，第一阶段直接把文件数量和大小固定,由文件系统的全局变量保存,而不写入硬盘中。

### 2. 数据结构设计

将文件设计成文件块，将索引和数据直接保存在一起，当文件块表示目录时，文件块中的数据块保存目录索引。

整个文件系统中共存在256 个文件块，也就是说文件+目录的最大数量不能超过256个

```
struct dir_entry {
    char name[MAX_NAME];
    int index;
}

struct file_block {
    int used;
    umode_t mode;
    union {
        int dir_children;
        int filelen;
    }
    char buf[MAX_SIZE];
}
```

### 3. 具体实现中问题

第一阶段目标基本完成

<del>可以创建文件和文件夹，但是cat 读取命令和 ls 搜索命令会执行很多遍（dmesg中显示函数正常运行，但是不知道为什么会运行这么多遍），不知道为什么，可能是等待什么返回值？目前可以执行的命令 ls -al a.txt 不可以 ls  更加诡异的是示例代码调整之后竟然没有上面的问题，这就需要讨论一下了。</del>

此处的问题是ls过程中没有调整 file->f_pos 和 pos的位置。

read 无限循环的问题主要也是原版代码blk数据结构存在问题

```
struct xxx {
    ...;
    char data[0]
};

struct xxx *x = kmalloc ((sizeof(struct xxx) + length) * SIZE);
```
这里这么实现是为了动态申请内存的时候能够在尾巴上加上任意长度内存，使得struct为动态长度，但是问题是申请内存的时候直接使用数组申请，所以导致获得内存块的位置经常出现问题

### 4. 可以进一步实现的目标

* 代码优化，拆分到多个文件里面
* 遍历时添加当前目录和上级目录，注意i_ino为1的时候没有上级目录
* 增加动态申请文件数
* 给fileblock的操作加锁
* 删除文件夹
* 创建时间正确
* 修改时间正确
* 修改读写便宜（read write llseek）
* 重命名
* iterate 循环经常崩溃的地方试着加入异常抛出（新版代码可能没有）

```
[ 1248.657816] expfs_iterate : Iterate on inode [1]
[ 1248.657817] expfs_iterate mode:1,  dir_children:2
[ 1248.657818] expfs expfs_iterate iterate  2  : d
[ 1248.657819] expfs expfs_iterate iterate  3  : e
[ 1248.657833] BUG: kernel NULL pointer dereference, address: 0000000000000030
[ 1248.657836] #PF: supervisor read access in kernel mode
[ 1248.657837] #PF: error_code(0x0000) - not-present page


[11877.310151] Call Trace:
[11877.310210]  simple_unlink+0x46/0x60
[11877.310212]  simple_rmdir+0x34/0x50
[11877.310234]  expfs_rmdir+0x43/0x49 [expfs]
[11877.310237]  vfs_rmdir+0x86/0x1a0
[11877.310238]  do_rmdir+0x18c/0x1c0
[11877.310240]  __x64_sys_unlinkat+0x45/0x60
[11877.315722]  do_syscall_64+0x57/0x190
[11877.315789]  entry_SYSCALL_64_after_hwframe+0x44/0xa9
[11877.315806] RIP: 0033:0x7fa1cc287f9b

```





