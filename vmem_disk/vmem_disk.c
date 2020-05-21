#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/stat.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/blk_types.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <uapi/linux/hdreg.h> //for struct hd_geometry
#include <uapi/linux/cdrom.h> //for CDROM_GET_CAPABILITY

// 重新设计vmem_disk 能够任意配置内核申请的内存大小
// 争取一个主设备下面能够分配多个子设备

