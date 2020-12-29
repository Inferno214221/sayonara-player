#!/bin/sh

# run in docker container luciocarreras/sayonara-fedora

set -e

COMMIT_ID=$1
SPEC_FILE=$(realpath -e $2)
BASE_DIR=${PWD}

if [ -z ${COMMIT_ID} -o ! -f ${SPEC_FILE} ] ; then
	echo "call $0 <commit-id> <spec-file>"
	exit 1
fi

export SPEC_DIR="/root/rpmbuild/SPECS"
export SOURCES_DIR="/root/rpmbuild/SOURCES"
export RPM_DIR="/root/rpmbuild/RPMS"

rm -rf ${SOURCES_DIR} ${SPEC_DIR} ${RPM_DIR}

mkdir -p ${SOURCES_DIR}
mkdir -p ${SPEC_DIR}
mkdir -p ${RPM_DIR}

# clone sayonara to sources
cd ${SOURCES_DIR}

git clone https://gitlab.com/luciocarreras/sayonara-player.git sayonara-player
cd sayonara-player
git checkout ${COMMIT_ID}
cd ..

VERSION=$(grep -oP '\d+(?:\.\d+)+-?\w+\d+' sayonara-player/CMakeLists.txt)
mv sayonara-player sayonara-player-${VERSION}

# create spec file
cp ${SPEC_FILE} ${SPEC_DIR}/

rm -rf sayonara-player/debian
rm -rf sayonara-player/.git*
rm -rf sayonara-player/build

tar cjSf sayonara-player-${VERSION}.tar.bz2 sayonara-player-${VERSION}

cd ${SPEC_DIR}

# build
rpmbuild -ba sayonara.spec

