/*
 * Sample disk driver, from the beginning.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/sched.h>
#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/timer.h>
#include <linux/types.h>	/* size_t */
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/hdreg.h>	/* HDIO_GETGEO */
#include <linux/kdev_t.h>
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/buffer_head.h>	/* invalidate_bdev */
#include <linux/bio.h>
#include <linux/blk-mq.h>
#include <linux/blk_types.h>
#include <linux/stat.h>
#include <linux/device.h>

MODULE_LICENSE("Dual BSD/GPL");

static int sbull_major = 0;
module_param(sbull_major, int, 0);
static int hardsect_size = 512;
module_param(hardsect_size, int, 0);
static int nsectors = 1024;	/* How big the drive is */
module_param(nsectors, int, 0);
static int ndevices = 4;
module_param(ndevices, int, 0);

/*
 * The different "request modes" we can use.
 */
enum {
	RM_SIMPLE  = 0,	/* The extra-simple request function */
	RM_FULL    = 1,	/* The full-blown version */
	RM_NOQUEUE = 2,	/* Use make_request */
};
static int request_mode = RM_SIMPLE;
module_param(request_mode, int, 0);

/*
 * Minor number and partition management.
 */
#define SBULL_MINORS	16
#define MINOR_SHIFT	4
#define DEVNUM(kdevnum)	(MINOR(kdev_t_to_nr(kdevnum)) >> MINOR_SHIFT

/*
 * We can tweak our hardware sector size, but the kernel talks to us
 * in terms of small sectors, always.
 */
#define KERNEL_SECTOR_SHIFT	9
#define KERNEL_SECTOR_SIZE	(1<<KERNEL_SECTOR_SHIFT)

/*
 * After this much idle time, the driver will simulate a media change.
 */
#define INVALIDATE_DELAY	30*HZ

/*
 * The internal representation of our device.
 */
struct sbull_dev {
        unsigned int size;                       /* Device size in sectors */
        u8 *data;                       /* The data array */
        short users;                    /* How many users */
        short media_change;             /* Flag a media change? */
        // spinlock_t lock;                /* For mutual exclusion */
        struct request_queue *queue;    /* The device request queue */
        struct gendisk *gd;             /* The gendisk structure */
        struct timer_list timer;        /* For simulated media changes */
		struct blk_mq_tag_set tag_set;
};

static struct sbull_dev *Devices = NULL;

static int bytes_to_sectors_checked(unsigned long bytes)
{
	if( bytes % KERNEL_SECTOR_SIZE )
	{
		printk("***************WhatTheFuck***********************\n");
	}
	return bytes / KERNEL_SECTOR_SIZE;
}

/*
 * Handle an I/O request.
 */
static void sbull_transfer(struct sbull_dev *dev, unsigned long sector,
		unsigned long nsect, char *buffer, int write)
{
	unsigned long offset = sector*KERNEL_SECTOR_SIZE;
	unsigned long nbytes = nsect*KERNEL_SECTOR_SIZE;

	if ((offset + nbytes) > dev->size) {
		printk (KERN_NOTICE "Beyond-end write (%ld %ld)\n", offset, nbytes);
		return;
	}
	if (write)
		memcpy(dev->data + offset, buffer, nbytes);
	else
		memcpy(buffer, dev->data + offset, nbytes);
}

/*
 * The simple form of the request function.
 */
// static void sbull_request(struct request_queue *q)
// {
// 	struct request *req;
// 	int ret;

// 	req = blk_fetch_request(q);
// 	while (req) {
// 		struct sbull_dev *dev = req->rq_disk->private_data;
// 		if (blk_rq_is_passthrough(req)) {
// 			printk (KERN_NOTICE "Skip non-fs request\n");
// 			ret = -EIO;
// 			goto done;
// 		}
// 		printk (KERN_NOTICE "Req dev %u dir %d sec %lld, nr %d\n",
// 			(unsigned)(dev - Devices), rq_data_dir(req),
// 			blk_rq_pos(req), blk_rq_cur_sectors(req));
// 		sbull_transfer(dev, blk_rq_pos(req), blk_rq_cur_sectors(req),
// 				bio_data(req->bio), rq_data_dir(req));
// 		ret = 0;
// 	done:
// 		if(!__blk_end_request_cur(req, ret)){
// 			req = blk_fetch_request(q);
// 		}
// 	}
// }

static int do_simple_request(struct request *rq, unsigned int *nr_bytes)
{
    int ret = 0;
    struct bio_vec bvec;
    struct req_iterator iter;
    struct sbull_dev *dev = rq->q->queuedata;
    loff_t pos = blk_rq_pos(rq);
    loff_t dev_size = (loff_t)(dev->size);

    printk(KERN_WARNING "sblkdev: request start from sector %lld \n", blk_rq_pos(rq));
    
    rq_for_each_segment(bvec, rq, iter)
    {
        unsigned long b_len = bvec.bv_len;

        void* b_buf = page_address(bvec.bv_page) + bvec.bv_offset;

        if ((pos + b_len) > dev_size)
            b_len = (unsigned long)(dev_size - pos);

        if (rq_data_dir(rq))//WRITE
            memcpy(dev->data + pos, b_buf, b_len);
        else//READ
            memcpy(b_buf, dev->data + pos, b_len);

        pos += b_len;
        *nr_bytes += b_len;
    }

    return ret;
}

static blk_status_t _queue_rq(struct blk_mq_hw_ctx *hctx, const struct blk_mq_queue_data* bd)
{
	unsigned int nr_bytes = 0;
    blk_status_t status = BLK_STS_OK;
    struct request *rq = bd->rq;

    //we cannot use any locks that make the thread sleep
    blk_mq_start_request(rq);

    if (do_simple_request(rq, &nr_bytes) != 0)
        status = BLK_STS_IOERR;

    printk(KERN_WARNING "sblkdev: request process %d bytes\n", nr_bytes);

    if (blk_update_request(rq, status, nr_bytes)) //GPL-only symbol
        BUG();
    __blk_mq_end_request(rq, status);

    return BLK_STS_OK;//always return ok
}

static struct blk_mq_ops sbull_mq_ops = {
	.queue_rq = _queue_rq,
};


/*
 * Transfer a single BIO.
 */
// static int sbull_xfer_bio(struct sbull_dev *dev, struct bio *bio)
// {
// 	struct bio_vec bvec;
// 	struct bvec_iter iter;
// 	sector_t sector = bio->bi_iter.bi_sector;

// 	/* Do each segment independently. */
// 	bio_for_each_segment(bvec, bio, iter) {
// 		char *buffer = kmap_atomic(bvec.bv_page) + bvec.bv_offset;
// 		sbull_transfer(dev, sector,bytes_to_sectors_checked(bio_cur_bytes(bio)),
// 				buffer, bio_data_dir(bio) == WRITE);
// 		sector += (bytes_to_sectors_checked(bio_cur_bytes(bio)));
// 		kunmap_atomic(buffer);
// 	}
// 	return 0; /* Always "succeed" */
// }


/*
 * Transfer a full request.
 */
// static int sbull_xfer_request(struct sbull_dev *dev, struct request *req)
// {
// 	struct bio *bio;
// 	int nsect = 0;
    
// 	__rq_for_each_bio(bio, req) {
// 		sbull_xfer_bio(dev, bio);
// 		nsect += bio->bi_iter.bi_size/KERNEL_SECTOR_SIZE;
// 	}
// 	return nsect;
// }



/*
 * Smarter request function that "handles clustering".
 */
// static void sbull_full_request(struct request_queue *q)
// {
// 	struct request *req;
// 	struct sbull_dev *dev = q->queuedata;
// 	int ret;

// 	while ((req = blk_fetch_request(q)) != NULL) {
// 		if (blk_rq_is_passthrough(req)) {
// 			printk (KERN_NOTICE "Skip non-fs request\n");
// 			ret = -EIO;
// 			goto done;
// 		}
// 		sbull_xfer_request(dev, req);
// 		ret = 0;
// 	done:
// 		__blk_end_request_all(req, ret);
// 	}
// }



/*
 * The direct make request version.
 */
// static blk_qc_t sbull_make_request(struct request_queue *q, struct bio *bio)
// {
// 	struct sbull_dev *dev = q->queuedata;
// 	int status;

// 	status = sbull_xfer_bio(dev, bio);
// 	bio->bi_status = status;
// 	bio_endio(bio);
// 	return BLK_QC_T_NONE;
// }


/*
 * Open and close.
 */

static int sbull_open(struct block_device *bdev, fmode_t mode)
{
	struct sbull_dev *dev = bdev->bd_disk->private_data;

	// del_timer_sync(&dev->timer);
	// spin_lock(&dev->lock);
	if (! dev->users) 
		check_disk_change(bdev);
	dev->users++;
	// spin_unlock(&dev->lock);
	return 0;
}

static void sbull_release(struct gendisk *disk, fmode_t mode)
{
	struct sbull_dev *dev = disk->private_data;

	// spin_lock(&dev->lock);
	dev->users--;

	if (!dev->users) {
		dev->timer.expires = jiffies + INVALIDATE_DELAY;
		// add_timer(&dev->timer);
	}
	// spin_unlock(&dev->lock);

}

/*
 * Look for a (simulated) media change.
 */
int sbull_media_changed(struct gendisk *gd)
{
	struct sbull_dev *dev = gd->private_data;
	
	return dev->media_change;
}

/*
 * Revalidate.  WE DO NOT TAKE THE LOCK HERE, for fear of deadlocking
 * with open.  That needs to be reevaluated.
 */
// int sbull_revalidate(struct gendisk *gd)
// {
// 	struct sbull_dev *dev = gd->private_data;
	
// 	if (dev->media_change) {
// 		dev->media_change = 0;
// 		memset (dev->data, 0, dev->size);
// 	}
// 	return 0;
// }

/*
 * The "invalidate" function runs out of the device timer; it sets
 * a flag to simulate the removal of the media.
 */
// void sbull_invalidate(struct timer_list* arg)
// {
// 	struct sbull_dev *dev = from_timer(dev, arg, timer);

// 	spin_lock(&dev->lock);
// 	if (dev->users || !dev->data) 
// 		printk (KERN_WARNING "sbull: timer sanity check failed\n");
// 	else
// 		dev->media_change = 1;
// 	spin_unlock(&dev->lock);
// }

/*
 * The ioctl() implementation
 */

int sbull_ioctl (struct block_device *bdev,
		 fmode_t mode,
                 unsigned int cmd, unsigned long arg)
{
	long size;
	struct hd_geometry geo;
	struct sbull_dev *dev = bdev->bd_disk->private_data;

	switch(cmd) {
	    case HDIO_GETGEO:
        	/*
		 * Get geometry: since we are a virtual device, we have to make
		 * up something plausible.  So we claim 16 sectors, four heads,
		 * and calculate the corresponding number of cylinders.  We set the
		 * start of data at sector four.
		 */
		size = dev->size*(hardsect_size/KERNEL_SECTOR_SIZE);
		geo.cylinders = (size & ~0x3f) >> 6;
		geo.heads = 4;
		geo.sectors = 16;
		geo.start = 4;
		if (copy_to_user((void __user *) arg, &geo, sizeof(geo)))
			return -EFAULT;
		return 0;
	}

	return -ENOTTY; /* unknown command */
}



/*
 * The device operations structure.
 */
static struct block_device_operations sbull_ops = {
	.owner           = THIS_MODULE,
	.open 	         = sbull_open,
	.release 	 = sbull_release,
	.media_changed   = sbull_media_changed,
	// .revalidate_disk = sbull_revalidate,
	.ioctl	         = sbull_ioctl
};


/*
 * Set up our internal device.
 */
static void setup_device(struct sbull_dev *dev, int which)
{
	int ret = 0;
	/*
	 * Get some memory.
	 */
	memset (dev, 0, sizeof (struct sbull_dev));
	dev->size = nsectors*hardsect_size;
	dev->data = vmalloc(dev->size);
	if (dev->data == NULL) {
		printk (KERN_NOTICE "vmalloc failure.\n");
		return;
	}
	// spin_lock_init(&dev->lock);
	
	/*
	 * The timer which "invalidates" the device.
	 */
	// timer_setup(&dev->timer, sbull_invalidate, 0);
	
	/*
	 * The I/O queue, depending on whether we are using our own
	 * make_request function or not.
	 */
	switch (request_mode) {
		// 暂时不考虑，sbull——load里面也是默认载入RM_SIMPLE的
	    // case RM_NOQUEUE:
		// dev->queue = blk_alloc_queue(GFP_KERNEL);
		// if (dev->queue == NULL)
		// 	goto out_vfree;
		// blk_queue_make_request(dev->queue, sbull_make_request);
		// break;

	    // case RM_FULL:
		// dev->queue = blk_init_queue(sbull_full_request, &dev->lock);
		// if (dev->queue == NULL)
		// 	goto out_vfree;
		// break;

	    default:
		printk(KERN_NOTICE "Bad request mode %d, using simple\n", request_mode);
        	/* fall into.. */
	
	    case RM_SIMPLE:
		// dev->queue = blk_init_queue(sbull_request, &dev->lock);
		// if (dev->queue == NULL)
		// 	goto out_vfree;
			dev->tag_set.ops = &sbull_mq_ops;
            dev->tag_set.nr_hw_queues = 1;
            dev->tag_set.queue_depth = 128;
            dev->tag_set.numa_node = NUMA_NO_NODE;
            dev->tag_set.cmd_size = sizeof(int);
            dev->tag_set.flags = BLK_MQ_F_SHOULD_MERGE;
            dev->tag_set.driver_data = dev;

            ret = blk_mq_alloc_tag_set(&dev->tag_set);
            if (ret) {
                printk(KERN_WARNING "sblkdev: unable to allocate tag set\n");
                break;
            }

			struct request_queue *queue = blk_mq_init_queue(&dev->tag_set);
            if (IS_ERR(queue)) {
                ret = PTR_ERR(queue);
                printk(KERN_WARNING "sblkdev: Failed to allocate queue\n");
                break;
            }
            dev->queue = queue;
		break;
	}
	// blk_queue_logical_block_size(dev->queue, hardsect_size);
	dev->queue->queuedata = dev;
	/*
	 * And the gendisk structure.
	 */
	dev->gd = alloc_disk(SBULL_MINORS);
	if (! dev->gd) {
		printk (KERN_NOTICE "alloc_disk failure\n");
		goto out_vfree;
	}
	dev->gd->major = sbull_major;
	dev->gd->first_minor = which*SBULL_MINORS;
	dev->gd->fops = &sbull_ops;
	dev->gd->queue = dev->queue;
	dev->gd->private_data = dev;
	snprintf (dev->gd->disk_name, 32, "sbull%c", which + 'a');
	set_capacity(dev->gd, nsectors*(hardsect_size/KERNEL_SECTOR_SIZE));
	add_disk(dev->gd);
	return;

  out_vfree:
	if (dev->data)
		vfree(dev->data);
}



static int __init sbull_init(void)
{
	int i;
	/*
	 * Get registered.
	 */
	sbull_major = register_blkdev(sbull_major, "sbull");
	if (sbull_major <= 0) {
		printk(KERN_WARNING "sbull: unable to get major number\n");
		return -EBUSY;
	}
	/*
	 * Allocate the device array, and initialize each one.
	 */
	Devices = kmalloc(ndevices*sizeof (struct sbull_dev), GFP_KERNEL);
	if (Devices == NULL)
		goto out_unregister;
	for (i = 0; i < ndevices; i++) 
		setup_device(Devices + i, i);
    
	return 0;

  out_unregister:
	unregister_blkdev(sbull_major, "sbd");
	return -ENOMEM;
}

static void sbull_exit(void)
{
	int i;

	for (i = 0; i < ndevices; i++) {
		struct sbull_dev *dev = Devices + i;

		// del_timer_sync(&dev->timer);
		if (dev->gd) {
			del_gendisk(dev->gd);
			put_disk(dev->gd);
		}
		if (dev->queue) {
			// if (request_mode == RM_NOQUEUE)
			// 	blk_put_queue(dev->queue);
			// else
				blk_cleanup_queue(dev->queue);
		}
		if (dev->data)
			vfree(dev->data);

		if (dev->tag_set.tags) {
			blk_mq_free_tag_set(&dev->tag_set);
		}
	}
	unregister_blkdev(sbull_major, "sbull");
	kfree(Devices);
}
	
module_init(sbull_init);
module_exit(sbull_exit);