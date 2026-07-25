/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PSMON_INTERNAL_H
#define PSMON_INTERNAL_H

#define PSMON_MIN_INTERVAL_MS 100U

int psmon_monitor_start(unsigned int interval_ms);
void psmon_monitor_stop(void);

#endif /* PSMON_INTERNAL_H */
