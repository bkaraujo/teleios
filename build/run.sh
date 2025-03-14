#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$0")/.."
BUILDFS="$ROOTFS/build/cmake-build-files"

$BUILDFS/teleios $1