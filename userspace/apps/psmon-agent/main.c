// SPDX-License-Identifier: GPL-2.0
#include <stdio.h>
#include <string.h>

#include "psmon_client.h"

static void print_usage(const char *program)
{
	printf("Usage: %s [--version|--help]\n", program);
}

int main(int argc, char **argv)
{
	if (argc == 2 && strcmp(argv[1], "--version") == 0) {
		printf("psmon-agent %s (ABI %u)\n", psmon_client_version(),
		       psmon_client_abi_version());
		return 0;
	}

	if (argc == 2 && strcmp(argv[1], "--help") == 0) {
		print_usage(argv[0]);
		return 0;
	}

	if (argc != 1) {
		print_usage(argv[0]);
		return 2;
	}

	printf("psmon-agent: collection loop is not implemented yet\n");
	return 0;
}
