// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) "psmon: " fmt

#include <linux/init.h>
#include <linux/module.h>

#include "psmon_internal.h"
#include "process_watch/pw_internal.h"

// static unsigned int scan_interval_ms = 5000;
// module_param_named(interval_ms, scan_interval_ms, uint, 0444);
// MODULE_PARM_DESC(interval_ms, "Process scan interval in milliseconds (minimum 100)");

static int __init psmon_init(void);
static void __exit psmon_exit(void);

static int __init psmon_init(void)
{
	int err;

	// create /proc/psmon directory
	if (psmon_root) {
		pr_warn("This module has been loaded\n");
		return 0;
	}
	if (!(psmon_root = proc_mkdir(PSMON_DIR_NAME, NULL))) {
		pr_err("create /proc/%s directory failed.\n", PSMON_DIR_NAME);
		return -EINVAL;
	}

	err = proc_watch_init(psmon_root);
	if (err) {
		psmon_exit();
		return err;
	}

	pr_info("loaded\n");
	return 0;
}

static void __exit psmon_exit(void)
{
	proc_watch_exit();
	
	if (psmon_root) {
		proc_remove(psmon_root);
		psmon_root = NULL;
	}

	pr_info("unloaded\n");
}

module_init(psmon_init);
module_exit(psmon_exit);

MODULE_AUTHOR("ShengbangWu");
MODULE_DESCRIPTION("A small process monitoring module");
MODULE_LICENSE("GPL");
