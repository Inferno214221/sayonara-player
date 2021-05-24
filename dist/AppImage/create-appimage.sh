#!/bin/sh

# run in docker container luciocarreras/sayonara-ubuntu:xenial

set -e

COMMIT_ID=$1
SAYONARA_VERSION=$(cat ./build/version)

if [ -z ${COMMIT_ID} ] ; then
	echo "Call $0 <commit-id>"
	exit 1
fi

cd build

if [ "x${DOCKER_QT_BASE_DIR}" != "x" ] ; then
	export QT_BASE_DIR=${DOCKER_QT_BASE_DIR}
else
	export QT_BASE_DIR=/opt/qt
fi

# avoid fuse problems
export APPIMAGE_EXTRACT_AND_RUN=1

export QMAKE="$QT_BASE_DIR/bin/qmake"
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
export EXTRA_QT_PLUGINS="iconengines,sqldrivers/libqsqlite.so,platforms/libqxcb.so" 

linuxdeploy-x86_64.AppImage --appdir=AppDir --desktop-file=./AppDir/usr/share/applications/com.sayonara-player.Sayonara.desktop --plugin=qt --custom-apprun=../dist/AppImage/AppRun

appimagetool-x86_64.AppImage AppDir

mv Sayonara_Player-x86_64.AppImage "sayonara-${SAYONARA_VERSION}-g${COMMIT_ID}.AppImage"

