#ifndef PSMON_PROC_WATCH_INTERNAL_H
#define PSMON_PROC_WATCH_INTERNAL_H

#include <linux/proc_fs.h>

int proc_watch_init(struct proc_dir_entry *_root);
void proc_watch_exit(void);

int create_watched_pid_file(struct proc_dir_entry *parent);

#endif