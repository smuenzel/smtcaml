#!/bin/zsh
INSTALL_DIR=$(pwd)/build
BUILD_DIR=build

mkdir -p build
mkdir -p build/include
pushd btor2tools
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=OFF
make -j8
make install
popd
popd

pushd minisat
make config prefix="${INSTALL_DIR}"
make -j8 install
popd

pushd lingeling
./configure.sh -static -O3
make -j8
cp liblgl.a "${INSTALL_DIR}/lib"
cp lglib.h "${INSTALL_DIR}/include"
popd

pushd cryptominisat
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=OFF -DSTATICCOMPILE=ON
make -j8
make install
popd
popd

pushd cadical
./configure
pushd build
make -j8
cp libcadical.a "${INSTALL_DIR}/lib"
popd
cp src/ccadical.h "${INSTALL_DIR}/include"
popd

pushd boolector
CMAKE_OPTS="-DCMAKE_CXX_STANDARD_LIBRARIES=-lboost_serialization" ./configure.sh --path ${INSTALL_DIR} --gmp --prefix ${INSTALL_DIR}
pushd build
make -j8
make install
popd
popd
