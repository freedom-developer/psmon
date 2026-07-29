// SPDX-License-Identifier: GPL-2.0
#include "processes.h"
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/compiler.h>
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/rcupdate.h>
#include <linux/sched/signal.h>
#include <linux/workqueue.h>

#include "ps_monitor.h"

/*
struct psmon_stats {
	size_t processes;
	size_t threads;
};

static struct delayed_work psmon_work;
static unsigned int psmon_interval_ms;
static bool psmon_stopping;

static void psmon_collect_stats(struct psmon_stats *stats)
{
	struct task_struct *leader;
	struct task_struct *task;

	rcu_read_lock();
	for_each_process(leader) {
		stats->processes++;
		stats->threads++; 

		for_each_thread(leader, task)
			stats->threads++;
	}
	rcu_read_unlock();
}

static void psmon_monitor_work(struct work_struct *work)
{
	struct psmon_stats stats = { 0 };

	(void)work;
	psmon_collect_stats(&stats);
	pr_info("processes=%zu threads=%zu\n", stats.processes, stats.threads);

	if (!READ_ONCE(psmon_stopping))
		schedule_delayed_work(&psmon_work,
				      msecs_to_jiffies(psmon_interval_ms));
}

int psmon_monitor_start(unsigned int interval_ms)
{
	if (interval_ms < PSMON_MIN_INTERVAL_MS)
		return -EINVAL;

	psmon_interval_ms = interval_ms;
	WRITE_ONCE(psmon_stopping, false);
	INIT_DELAYED_WORK(&psmon_work, psmon_monitor_work);
	schedule_delayed_work(&psmon_work, msecs_to_jiffies(interval_ms));

	return 0;
}

void psmon_monitor_stop(void)
{
	WRITE_ONCE(psmon_stopping, true);
	cancel_delayed_work_sync(&psmon_work);
}

*/

int ps_monitor_init(struct proc_dir_entry *dir)
{
	int err;

	err = create_processes_file(dir);
	if (err) {
		ps_monitor_exit();
		return err;
	}

	return 0;
}

void ps_monitor_exit(void)
{
	destroy_processes_file();
}