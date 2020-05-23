# simplefs 简单文件系统

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
const struct inode_operations simplefs_file_inode_operations；
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






