#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$(realpath -s "$0")")/.."
BUILDFS="$ROOTFS/cmake-build-debug"
# -----------------------------------------
# Build the target
# -----------------------------------------
${ROOTFS}/build/build.sh
# -----------------------------------------
# Execute the target
# -----------------------------------------
echo "[TELEIOS] Run ... "
$BUILDFS/teleios $1