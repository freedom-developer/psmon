#!/bin/sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
module_path="$project_root/kernel/psmon.ko"

if [ "$(id -u)" -ne 0 ]; then
	echo "error: run this script as root (for example: sudo $0)" >&2
	exit 1
fi

if [ ! -f "$module_path" ]; then
	echo "error: $module_path does not exist; run 'make kernel' first" >&2
	exit 1
fi

exec insmod "$module_path" "$@"
