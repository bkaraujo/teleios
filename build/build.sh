#!/usr/bin/env bash

clear
ROOTFS="$(dirname "$(realpath -s "$0")")/.."
BUILDFS="$ROOTFS/cmake-build-debug"
# -----------------------------------------
# (Re)create the build directory
# -----------------------------------------
if [[ ! -z $1 && $1 == "clean" ]]; then
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
# -----------------------------------------
# Check for correct meta.h usage
# -----------------------------------------
find $ROOTFS/engine/src/main/teleios/ -type f -name "*.c" ! -name 'logger.c' ! -name 'profiler.c' | while read -r fname ; do
    if [[ $(grep -c return "$fname") -ne 0 ]] ; then
        echo "Unexpected [return] statemente in $fname"
    fi
done
# -----------------------------------------
# Build the target
# -----------------------------------------
echo "cmake --build $BUILDFS -j$(expr $(cat /proc/cpuinfo | grep 'model name' | wc -l) - 1)"
cmake --build $BUILDFS -j$(expr $(cat /proc/cpuinfo | grep 'model name' | wc -l) - 1)
RESULT=$?

if [[ $RESULT -ne 0 ]]; then
   exit $RESULT
fi