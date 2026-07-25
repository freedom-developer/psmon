#!/bin/sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
kernel_dir="$project_root/kernel"
kernel_build_dir=${KDIR:-/lib/modules/$(uname -r)/build}
output="$project_root/compile_commands.json"

if [ ! -d "$kernel_build_dir" ]; then
	echo "error: kernel build directory not found: $kernel_build_dir" >&2
	echo "install the headers for kernel $(uname -r), or set KDIR" >&2
	exit 1
fi

if ! command -v bear >/dev/null 2>&1; then
	echo "error: bear is required to capture Kbuild compiler commands" >&2
	echo "on Debian/Ubuntu, install it with: sudo apt install bear" >&2
	exit 1
fi

# Force every module source file through the compiler so Bear can capture the
# exact include paths, defines, and forced includes selected by Kbuild. Keep
# going after a source error: the captured command is still valid for the IDE.
make -C "$kernel_dir" KDIR="$kernel_build_dir" clean

set +e
bear --output "$output" -- \
	make -k -C "$kernel_dir" KDIR="$kernel_build_dir"
build_status=$?
set -e

if [ ! -s "$output" ] || ! grep -q '"file"' "$output"; then
	echo "error: no compilation commands were generated" >&2
	exit 1
fi

echo "generated $output"

if [ "$build_status" -ne 0 ]; then
	echo "warning: the module build did not finish, but the compilation database was generated" >&2
fi
