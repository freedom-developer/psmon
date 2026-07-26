#ifndef PSMON_PROC_WATCH_INTERNAL_H
#define PSMON_PROC_WATCH_INTERNAL_H

#include <linux/proc_fs.h>
#include <linux/pid.h>

extern struct mutex watched_pid_lock;
extern struct pid *watched_pid;
extern const struct proc_ops pi_ops;

int proc_watch_init(struct proc_dir_entry *_root);
void proc_watch_exit(void);

struct proc_dir_entry *create_pw_pid_file(struct proc_dir_entry *parent);
struct proc_dir_entry *create_pw_info_file(struct proc_dir_entry *parent);

#endif
