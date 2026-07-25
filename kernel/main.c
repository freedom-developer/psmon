// SPDX-License-Identifier: GPL-2.0
#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/init.h>
#include <linux/module.h>

#include "psmon_internal.h"

// static unsigned int scan_interval_ms = 5000;
// module_param_named(interval_ms, scan_interval_ms, uint, 0444);
// MODULE_PARM_DESC(interval_ms, "Process scan interval in milliseconds (minimum 100)");

static int __init psmon_init(void)
{
	int ret;

	ret = psmon_monitor_start(scan_interval_ms);
	if (ret) {
		pr_err("interval_ms must be at least %u (got %u)\n",
		       PSMON_MIN_INTERVAL_MS, scan_interval_ms);
		return ret;
	}

	pr_info("loaded; scan interval is %u ms\n", scan_interval_ms);
	return 0;
}

static void __exit psmon_exit(void)
{
	psmon_monitor_stop();
	pr_info("unloaded\n");
}

module_init(psmon_init);
module_exit(psmon_exit);

MODULE_AUTHOR("psmon contributors");
MODULE_DESCRIPTION("A small process monitoring module");
MODULE_LICENSE("GPL");
