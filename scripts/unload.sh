#!/bin/sh
set -eu

if [ "$(id -u)" -ne 0 ]; then
	echo "error: run this script as root (for example: sudo $0)" >&2
	exit 1
fi

exec rmmod psmon
