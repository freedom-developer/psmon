// SPDX-License-Identifier: GPL-2.0
#include "psmon_client.h"

#include <psmon.h>

const char *psmon_client_version(void)
{
	return "0.1.0";
}

unsigned int psmon_client_abi_version(void)
{
	return PSMON_ABI_VERSION;
}
