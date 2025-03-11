#!/usr/bin/env bash

# if [[ ! -e bin ]]; then
#    mkdir bin
# fi

clear
ROOTFS="$(dirname "$0")"
# CMD=$(which gcc)
# if [[ -z $CMD ]]; then
#   echo "gcc not found"
# fi

# cFilenames=$(find "$ROOTFS/engine" -type f -name "*.c")

# assembly="teleios"

# CFlags="-std=c11 -g -fPIC"
# IFlags="-I$ROOTFS/engine/src"
# LFlags="-L/usr/X11R6/lib -lxcb -lX11 -lX11-xcb -lxkbcommon"
# DFlags="-DTELEIOS_EXPORT"

if [[ ! -e cmake-ninja-files ]]; then
   mkdir cmake-ninja-files
   cd cmake-ninja-files
   cmake -G Ninja $ROOTFS
else
   cd cmake-ninja-files
fi

ninja -j8
RESULT=$?

if [[ $RESULT -ne 0 ]]; then
   exit $RESULT
fi

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./bin"
./teleios "./sandbox/application.yml"