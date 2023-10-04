#!/bin/zsh
INSTALL_DIR=$(pwd)/../build
echo "Install Dir=\"${INSTALL_DIR}\""
BUILD_DIR=build

pushd stp
mkdir -p ${BUILD_DIR}
pushd ${BUILD_DIR}
cmake .. -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" -DBUILD_SHARED_LIBS=OFF -DSTATICCOMPILE=ON
make -j8
make install
popd
rsync -avP ./include/stp ${INSTALL_DIR}/include
cp "${BUILD_DIR}/include/stp/config.h" "${INSTALL_DIR}/include/stp"
rm "${INSTALL_DIR}/include/stp/config.h.in"
rm -rf "${BUILD_DIR}"
popd
