#include "pw_internal.h"

#include <linux/mutex.h>
#include <stddef.h>
#include <stdio.h>

#define WATCHED_PID_FILE_NAME "watched_pid"
static int watched_pid_open(struct inode *inode, struct file *filp);

static DEFINE_MUTEX(watched_pid_lock);
static pid_t watched_nr;
static struct pid *watched_pid;
static struct proc_ops watched_pid_ops = {
    .proc_open = watched_pid_open,
};

// create watched_pid file
int create_watched_pid_file(struct proc_dir_entry *parent)
{
    if (!parent) {
        pr_err("create_watched_pid_file failed: parent is NULL\n");
        return -EINVAL;
    }
    
    struct proc_dir_entry *watched_pid_file = proc_create(WATCHED_PID_FILE_NAME, 0666, parent, &watched_pid_ops);
    if (!watched_pid_file) {
        pr_err("create_watched_pid_file failed: create %s file failed\n", WATCHED_PID_FILE_NAME);
        return -EINVAL;
    }

    return 0;
}

static int watched_pid_open(struct inode *inode, struct file *filp)
{
    return 0;
}
static ssize_t watched_pid_read(struct file *filp, char __user *ubuf, size_t size, loff_t *ppos)
{
    char kbuf[16];
    int copy_len;
    mutex_lock(&watched_pid_lock);
    copy_len = snprintf(kbuf, 16, "%d", (int)watched_nr); 
    mutex_unlock(&watched_pid_lock);
    if (copy_len < 0 || size < copy_len)
        return -EINVAL;
    copy_to_user(ubuf, kbuf, copy_len);
    return 0;
}

