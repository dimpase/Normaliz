#!/bin/sh -e

WITH_GMP=""
if [ "$GMP_INSTALLDIR" != "" ]; then
  WITH_GMP="--with-gmp=$GMP_INSTALLDIR"
fi

if [ "x$NMZ_OPT_DIR" = x ]; then 
    export NMZ_OPT_DIR=${PWD}/nmz_opt_lib
        mkdir -p ${NMZ_OPT_DIR}
fi

if [ "x$NMZ_COMPILER" != x ]; then
    export CXX=$NMZ_COMPILER
elif [[ $OSTYPE == darwin* ]]; then
    export CXX=clang++
fi

#mkdir -p BUILD
#cd BUILD
if [ ! -e configure ];
then
    ./bootstrap.sh
fi
./configure --with-cocoalib=${NMZ_OPT_DIR} --with-flint=${NMZ_OPT_DIR} $EXTRA_FLAGS $WITH_GMP
make -j4
echo "******************************************************"
echo -e "\033[0;31mIf you want to install normaliz, run sudo make install\033[0m"
echo "******************************************************"
