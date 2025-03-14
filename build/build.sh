#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$(realpath -s "$0")")/.."
BUILDFS="$ROOTFS/build/cmake-build-files"
if [[ ! -z $1 && $1 -eq "clean" ]]; then
  rm -rf $BUILDFS
fi

if [[ ! -e $BUILDFS ]]; then
   mkdir -p $BUILDFS ; cd $BUILDFS
   cmake -G Ninja $ROOTFS
fi

cd $BUILDFS
cmake --build . -- -j$(expr $(hwinfo --cpu --short | wc -l) - 1)
RESULT=$?

if [[ $RESULT -ne 0 ]]; then
   exit $RESULT
fi