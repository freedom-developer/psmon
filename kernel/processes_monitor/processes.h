#ifndef PROCESSES_MONITOR_PROCESSES_H
#define PROCESSES_MONITOR_PROCESSES_H

#include <linux/proc_fs.h>

int create_processes_file(struct proc_dir_entry *parent);
void destroy_processes_file(void);


#endif