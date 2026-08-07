// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include "ps_monitor.h"
#include "processes.h"

#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/rcupdate.h>
#include <linux/sched/signal.h>
#include <linux/workqueue.h>

#include "ps_monitor.h"

int ps_monitor_init(struct proc_dir_entry *dir)
{
	int err;

	err = create_processes_file(dir);
	if (err)
		return err;



	return 0;
}

void ps_monitor_exit(void)
{
	destroy_processes_file();
}