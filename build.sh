#!/usr/bin/env bash

if [[ ! -e bin ]]; then
   mkdir bin
fi

clear
ROOTFS="$(dirname "$0")"
CMD=$(which gcc)
if [[ -z $CMD ]]; then
  echo "gcc not found"
fi

cFilenames=$(find "$ROOTFS/engine" -type f -name "*.c")

assembly="teleios"

CFlags="-std=c11 -g -fPIC"
IFlags="-I$ROOTFS/engine/src"
LFlags="-lxcb -lX11 -lX11-xcb -lxkbcommon -L/usr/X11R6/lib -lrt"
DFlags="-DTELEIOS_EXPORT"

$CMD $cFilenames $CFlags $IFlags $LFlags $DFlags -o $ROOTFS/bin/$assembly
RESULT=$?

if [[ $RESULT -ne 0 ]]; then
   exit $RESULT
fi

export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:./bin"
./bin/teleios "./sandbox/application.yml"