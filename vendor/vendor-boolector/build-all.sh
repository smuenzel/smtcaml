#!/bin/zsh
set -e

INSTALL_DIR=$(pwd)/../build
echo "Install Dir=\"${INSTALL_DIR}\""
BUILD_DIR=build

mkdir -p build
mkdir -p build/include
pushd btor2tools
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=OFF -DCMAKE_POSITION_INDEPENDENT_CODE=true
make -j8
make install
popd
rm -rf "${BUILD_DIR}"
popd

pushd minisat
make clean
make config prefix="${INSTALL_DIR}" MINISAT_REL="-fPIC -O3 -D NDEBUG"
make -j8 install
popd

pushd lingeling
CC="gcc -fPIC" ./configure.sh -static -O3
make -j8
cp liblgl.a "${INSTALL_DIR}/lib"
cp lglib.h "${INSTALL_DIR}/include"
make clean
popd

pushd cryptominisat
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=OFF -DSTATICCOMPILE=ON
make -j8
make install
popd
rm -rf "${BUILD_DIR}"
popd

pushd cadical
./configure -fPIC
pushd build
make -j8
cp libcadical.a "${INSTALL_DIR}/lib"
popd
cp src/ccadical.h "${INSTALL_DIR}/include"
popd

pushd boolector
CMAKE_OPTS="-DCMAKE_CXX_STANDARD_LIBRARIES=-lboost_serialization -DCMAKE_POSITION_INDEPENDENT_CODE=true" ./configure.sh --path ${INSTALL_DIR} --gmp --prefix ${INSTALL_DIR}
pushd build
make -j8
make install
popd
popd
