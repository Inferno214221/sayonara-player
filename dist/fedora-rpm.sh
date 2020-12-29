#!/bin/sh

# run in docker container luciocarreras/sayonara-fedora

set -e

BASE_DIR=${PWD}

export SPEC_DIR="/root/rpmbuild/SPECS"
export SOURCES_DIR="/root/rpmbuild/SOURCES"
export RPM_DIR="/root/rpmbuild/RPMS"

rm -rf ${SOURCES_DIR} ${SPEC_DIR} ${RPM_DIR}

mkdir -p ${SOURCES_DIR}
mkdir -p ${SPEC_DIR}
mkdir -p ${RPM_DIR}

cd ${SOURCES_DIR}
git clone https://gitlab.com/luciocarreras/sayonara-player.git sayonara-player
cd sayonara-player

# create spec file
mkdir -p build && cd build
cmake ..
cd ../..
cp sayonara-player/build/dist/sayonara.spec ${SPEC_DIR}/

rm -rf sayonara-player/debian
rm -rf sayonara-player/.git*
rm -rf sayonara-player/build

VERSION=$(grep -oP '\d+(?:\.\d+)+-?\w+\d+' sayonara-player/CMakeLists.txt)
tar czf sayonara-player-${VERSION}.tar.gz sayonara-player

cd ${SPEC_DIR}

# build
rpmbuild -ba sayonara.spec

mkdir -p "${BASE_DIR}/rpms"

find ${RPM_DIR} -name "*.rpm" -exec cp -v {} "${BASE_DIR}/rpms/" \;

