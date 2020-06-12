#! /bin/sh
mthreads="-j$(nproc)"
make package $mthreads || exit
echo -n "Press return to exit..."
read a