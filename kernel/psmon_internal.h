/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PSMON_INTERNAL_H
#define PSMON_INTERNAL_H

#include <linux/proc_fs.h>

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

struct proc_dir_entry *psmon_root = NULL;
#define PSMON_DIR_NAME "psmon"


#define PSMON_MIN_INTERVAL_MS 100U

// int psmon_monitor_start(unsigned int interval_ms);
// void psmon_monitor_stop(void);

#endif /* PSMON_INTERNAL_H */
