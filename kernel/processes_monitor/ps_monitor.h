/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PSMON_INTERNAL_H
#define PSMON_INTERNAL_H

#include <linux/proc_fs.h>

int ps_monitor_init(struct proc_dir_entry *dir);
void ps_monitor_exit(void);

#endif /* PSMON_INTERNAL_H */
