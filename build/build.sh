#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$(realpath -s "$0")")/.."
BUILDFS="$ROOTFS/cmake-build-debug"
# -----------------------------------------
# (Re)create the build directory
# -----------------------------------------
if [[ ! -z $1 && $1 -eq "clean" ]]; then
  rm -rf $BUILDFS
fi

if [[ ! -e $BUILDFS ]]; then
  echo "[TELEIOS] Clean ... "
   mkdir -p $BUILDFS ; cd $BUILDFS
   echo "cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S $ROOTFS -B $BUILDFS"
   cmake -DCMAKE_BUILD_TYPE=Debug -G Ninja -S $ROOTFS -B $BUILDFS
fi
# -----------------------------------------
# Build the target
# -----------------------------------------
echo "[TELEIOS] Build ... "
cd $BUILDFS

echo "cmake --build $BUILDFS -j$(expr $(hwinfo --cpu --short | wc -l) - 1)"
cmake --build $BUILDFS -j$(expr $(hwinfo --cpu --short | wc -l) - 1)
RESULT=$?

if [[ $RESULT -ne 0 ]]; then
   exit $RESULT
fi