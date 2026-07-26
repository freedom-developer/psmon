#include "pw_internal.h"

static struct proc_dir_entry *pw_pid_file = NULL;
static struct proc_dir_entry *pw_info_file = NULL;

int proc_watch_init(struct proc_dir_entry *root)
{  
    pw_pid_file = create_pw_pid_file(root);
    if (!pw_pid_file) 
        return -EINVAL;
    
    pw_info_file = create_pw_info_file(root);
    if (!pw_info_file) {
        proc_watch_exit();
        return -EINVAL;
    }

    return 0;
}

void proc_watch_exit(void)
{
    if (pw_pid_file) {
        proc_remove(pw_pid_file);
        pw_pid_file = NULL;
    }
    if (pw_info_file) {
        proc_remove(pw_info_file);
        pw_info_file = NULL;
    }

    if (watched_pid) {
        mutex_lock(&watched_pid_lock);
        put_pid(watched_pid);
        mutex_unlock(&watched_pid_lock);
        watched_pid = NULL;
    }
}