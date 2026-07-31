#include "processes.h"
#include <linux/seq_file.h>
#include <linux/sched.h>


static struct proc_dir_entry *processes_file = NULL;
#define PROCESSES_FILENAME "processes"

static int processes_show(struct seq_file *, void *);

int create_processes_file(struct proc_dir_entry *parent)
{
    processes_file = proc_create_single(PROCESSES_FILENAME, 0444, parent, processes_show);
    if (!processes_file) {
        pr_err("create processes file failed\n");
        return -ENOMEM;
    }
    return 0;
}

void destroy_processes_file(void)
{
    if (processes_file) {
        proc_remove(processes_file);
        processes_file = NULL;
    }
}

static int processes_show(struct seq_file *m, void *data)
{
    struct task_struct *p;

    seq_printf(m, "%-10s%-10s%-7s%-9s %s\n", "PID", "PPID", "STATE", "THREADS", "COMM");
    rcu_read_lock();
    for_each_process(p) {
        seq_printf(m, "%-10d%-10d%-7c%-9d %s\n", 
            task_pid_nr(p), task_ppid_nr(p), task_state_to_char(p),
            READ_ONCE(p->signal->nr_threads),
            p->comm
        );
    }
    rcu_read_unlock();

    return 0;
}
