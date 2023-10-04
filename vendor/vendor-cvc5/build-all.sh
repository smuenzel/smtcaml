#!/bin/zsh
set -e

INSTALL_DIR=$(pwd)/../build
echo "Install Dir=\"${INSTALL_DIR}\""
BUILD_DIR=build

PARALLEL=$(nproc || echo 1)

pushd cadical
rm -rf build
./configure -fpic -static
pushd build
make -j${PARALLEL}
cp cadical --target-directory ${INSTALL_DIR}/bin
cp libcadical.a --target-directory ${INSTALL_DIR}/lib
popd
cp src/cadical.hpp --target-directory ${INSTALL_DIR}/include
popd

pushd kissat
git clean -Xf
rm -rf build
./configure -fpic
pushd build
make -j${PARALLEL}
cp kissat --target-directory ${INSTALL_DIR}/bin
cp libkissat.a --target-directory ${INSTALL_DIR}/lib
popd
mkdir -p ${INSTALL_DIR}/include/kissat
cp src/kissat.h --target-directory ${INSTALL_DIR}/include/kissat
popd


pushd libpoly
rm -rf build
mkdir build
pushd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR}
make -j${PARALLEL}
make install
popd
rm -rf build
git restore build
popd

mkdir -p ${INSTALL_DIR}/include/symfpu
rsync -avP --include='*.h' --include='*/' --exclude='*' symfpu/core ${INSTALL_DIR}/include/symfpu
rsync -avP --include='*.h' --include='*/' --exclude='*' symfpu/utils ${INSTALL_DIR}/include/symfpu

pushd cvc5
rm -rf build
./configure.sh --static --kissat --poly --cryptominisat --dep-path=${INSTALL_DIR}  --prefix=${INSTALL_DIR}
pushd build
make -j${PARALLEL}
make install
popd
popd

