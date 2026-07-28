#include "pw_internal.h"

#include <linux/kstrtox.h>
#include <linux/mutex.h>

#define WATCHED_PID_FILE_NAME "watched_pid"
DEFINE_MUTEX(watched_pid_lock);
struct pid *watched_pid;
static pid_t watched_nr;

static ssize_t watched_pid_read(struct file *filp, char __user *ubuf, size_t size, loff_t *ppos);
static ssize_t watched_pid_write(struct file *filp, const char __user *ubuf, size_t size, loff_t *ppos);
static const struct proc_ops watched_pid_ops = {
    .proc_read = watched_pid_read,
    .proc_write = watched_pid_write,
};

#define PROCESS_INFO_FILE_NAME "process_info"

// create watched_pid file
struct proc_dir_entry *create_pw_pid_file(struct proc_dir_entry *parent)
{
    if (!parent) {
        pr_err("create_watched_pid_file failed: parent is NULL\n");
        return NULL;
    }
    
    struct proc_dir_entry *tmp = proc_create(WATCHED_PID_FILE_NAME, 0666, parent, &watched_pid_ops);
    if (!tmp) {
        pr_err("create_watched_pid_file failed: create %s file failed\n", WATCHED_PID_FILE_NAME);
        return NULL;
    }

    return tmp;
}

static ssize_t watched_pid_read(struct file *filp, char __user *ubuf, size_t size, loff_t *ppos)
{
    char kbuf[16];
    int copy_len;
    if (*ppos < 0)
        return -EINVAL;
    if (*ppos > 0) // 只允许读一次
        return 0;
    mutex_lock(&watched_pid_lock);
    copy_len = scnprintf(kbuf, 16, "%d\n", (int)watched_nr); 
    mutex_unlock(&watched_pid_lock);
    return simple_read_from_buffer(ubuf, size, ppos, kbuf, copy_len);
}

static ssize_t watched_pid_write(struct file *filp, const char __user *ubuf, size_t size, loff_t *ppos)
{
    int err;
    int nr;
    struct pid *new_pid;

    err = kstrtoint_from_user(ubuf, size, 10, &nr);
    if (err)
        return err;

    mutex_lock(&watched_pid_lock);
    watched_nr = (pid_t)nr;
    new_pid = find_get_pid(watched_nr);
    if (!new_pid) {
        mutex_unlock(&watched_pid_lock);
        pr_err("find_get_pid failed with pid %d\n", watched_nr);
        return -ESRCH;
    } else {
        if (watched_pid)
            put_pid(watched_pid);
        watched_pid = new_pid;
    }
    mutex_unlock(&watched_pid_lock);

    return size;
}

struct proc_dir_entry *create_pw_info_file(struct proc_dir_entry *parent)
{
    struct proc_dir_entry *tmp;

    tmp = proc_create(PROCESS_INFO_FILE_NAME, 0400, parent, &pi_ops);
    if (!tmp) {
        pr_err("create process_info failed\n");
        return NULL;
    }

    return tmp;
}