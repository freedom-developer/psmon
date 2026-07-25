#!/bin/sh
set -eu

project_root=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)

"$project_root/userspace/build/psmonctl" --version
"$project_root/userspace/build/psmon-agent" --version
