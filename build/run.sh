#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$(realpath -s "$0")")/.."
BUILDFS="$ROOTFS/build/cmake-build-files"

$BUILDFS/teleios $1