#!/usr/bin/env bash

set -e

source $(dirname "$0")/common.sh

CONFIGURE_FLAGS="--prefix=${PREFIX} --with-mpfr=${PREFIX} ${EXTRA_FLINT_FLAGS}"
if [ "$GMP_INSTALLDIR" != "" ]; then
    CONFIGURE_FLAGS="${CONFIGURE_FLAGS} --with-gmp=${GMP_INSTALLDIR}"
fi

## script for the installation of Flint for the use in libnormaliz

FLINT_VERSION="2.6.0"
FLINT_URL="http://www.flintlib.org/flint-${FLINT_VERSION}.tar.gz"
FLINT_SHA256=9089edadd12cbbda4533ab6f58efb7565fd973b5b82a89f53f64203bc3510840


echo "Installing FLINT..."

mkdir -p ${NMZ_OPT_DIR}/Flint_source/
cd ${NMZ_OPT_DIR}/Flint_source
../../download.sh ${FLINT_URL} ${FLINT_SHA256}
if [ ! -d flint-${FLINT_VERSION} ]; then
    tar -xvf flint-${FLINT_VERSION}.tar.gz
fi
cd flint-${FLINT_VERSION}
if [ ! -f Makefile ]; then
    ./configure ${CONFIGURE_FLAGS}
fi
# patch to avoid PIE clash in Ubuntu >= 16-10
if [[ $OSTYPE == "linux-gnu" ]]; then
    sed -i s/"-Wl,"// Makefile.subdirs
fi
# make -j4 # verbose
make install -j4
