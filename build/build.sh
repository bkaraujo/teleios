#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$0")/.."
BUILDFS="$ROOTFS/build/cmake-build-files"
if [[ ! -e $BUILDFS ]]; then
   mkdir -p $BUILDFS ; cd $BUILDFS
   cmake -G Ninja $ROOTFS
else
   cd $BUILDFS
fi

cmake --build $BUILDFS -- -j$(expr $(hwinfo --cpu --short | wc -l) - 1)
RESULT=$?

if [[ $RESULT -ne 0 ]]; then
   exit $RESULT
fi