#include "processes.h"

static struct proc_dir_entry *processes_file = NULL;
#define PROCESSES_FILENAME "processes"

static int processes_show(struct seq_file *, void *);

int create_processes_file(struct proc_dir_entry *parent)
{
    processes_file = proc_create_single(PROCESSES_FILENAME, 0444, parent, processes_show);
    if (!processes_file) {
        pr_err("create processes file failed\n");
        return -EINVAL;
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

    for_each_process(p) {

    }

    return 0;
}