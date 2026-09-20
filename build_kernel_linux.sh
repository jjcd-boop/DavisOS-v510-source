#!/usr/bin/env bash
set -euo pipefail
# v27: this compatibility entrypoint deliberately delegates to the canonical all-kernel build.
# The old hand-maintained object list silently drifted as subsystems were added.
exec bash ./BUILD_DAVIS_OS.sh
