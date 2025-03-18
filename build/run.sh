#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$(realpath -s "$0")")/.."
BUILDFS="$ROOTFS/cmake-build-debug"

$BUILDFS/teleios $1