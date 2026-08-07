#include "ps_chrdev.h"

#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kfifo.h>

#define DEVBASE 0
#define DEVCOUNT 1
#define DEVNAME "psmon"
#define PSMCLASS "psm_class"


dev_t dev_no;

struct psm_char_dev {
    // DECLARE_KFIFO_PTR()
    struct cdev dev;
};

struct file_operations psm_cdev_ops = {
    .owner = THIS_MODULE,
};

struct psm_char_dev psmon_dev;
struct device *psm_device;
struct class *psm_class;

int ps_chrdev_init(void)
{
    int ret;
    // 申请设备号
    
    ret = alloc_chrdev_region(&dev_no, DEVBASE, DEVCOUNT, DEVNAME);
    if (ret)
        return ret;

    cdev_init(&psmon_dev.dev, &psm_cdev_ops);
    ret = cdev_add(&psmon_dev.dev, dev_no, DEVCOUNT);
    if (ret) {
        unregister_chrdev_region(dev_no, DEVCOUNT);
        return ret;
    }

    // 创建设备文件
    psm_class = class_create(PSMCLASS);
    if (IS_ERR(psm_class)) {
        ps_chrdev_exit();
        return -EINVAL;
    }
    psm_device = device_create(psm_class, NULL, dev_no, NULL, DEVNAME);
    if (IS_ERR(psm_device)) {
        ps_chrdev_exit();
        return -EINVAL;
    }

    return 0;
}


void ps_chrdev_exit(void)
{
    if (psm_device) {
        device_destroy(psm_class, dev_no);
        psm_device = NULL;
    }

    if (psm_class) {
        class_destroy(psm_class);
        psm_class = NULL;
    }
    cdev_del(&psmon_dev.dev);
    unregister_chrdev_region(dev_no, DEVCOUNT);
}