#!/usr/bin/env bash

## script for the installation of antic for the use in libnormaliz

set -e

echo "::group::antic"

source $(dirname "$0")/common.sh

CONFIGURE_FLAGS="--prefix=${PREFIX}"
if [ "$OSTYPE" != "msys" ]; then
	CONFIGURE_FLAGS="${CONFIGURE_FLAGS} --with-mpfr=${PREFIX} --with-flint=${PREFIX}"
else
	CONFIGURE_FLAGS="${CONFIGURE_FLAGS} --with-gmp=${MSYS_STANDARD_LOC} --with-mpfr=${MSYS_STANDARD_LOC} --with-flint=${MSYS_STANDARD_LOC}"
fi

if [ "$GMP_INSTALLDIR" != "" ]; then
    CONFIGURE_FLAGS="${CONFIGURE_FLAGS} --with-gmp=${GMP_INSTALLDIR}"
fi

ANTIC_VERSION=0.2.5
ANTIC_URL="https://github.com/wbhart/antic/archive/v${ANTIC_VERSION}.tar.gz"
ANTIC_SHA256=7b236b6226d5730cf66d492f1562c34d4dcf1bdc8775fec2330f560332ab05c1

mkdir -p ${NMZ_OPT_DIR}/ANTIC_source/
cd ${NMZ_OPT_DIR}/ANTIC_source

../../download.sh ${ANTIC_URL} ${ANTIC_SHA256}
if [ ! -d antic-${ANTIC_VERSION} ]; then
    tar -xvf v${ANTIC_VERSION}.tar.gz
fi

cd antic-${ANTIC_VERSION}/

if [ "$OSTYPE" != "msys" ]; then
	./configure ${CONFIGURE_FLAGS}
	make -j4
	make install
else
	# must make shared and static in separate builds
	./configure ${CONFIGURE_FLAGS} --disable-static
	make -j4
	make install
	./configure ${CONFIGURE_FLAGS} --disable-shared
	make -j4
	make install
fi



