#!/bin/sh

# run in docker container luciocarreras/sayonara-appimage

set -e

case "$1" in
	build)
		mkdir -p build && cd build
		cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DWITH_COTIRE=1 -DLINK_GSTREAMER_PLUGINS=1 -DCMAKE_PREFIX_PATH=/opt/qt512
		make -j8
		make install DESTDIR=AppDir
		;;
	deploy)
		cd build
		export EXTRA_QT_PLUGINS="iconengines,sqldrivers/libqsqlite.so,platforms/libqxcb.so" 
		export QMAKE="/opt/qt512/bin/qmake"
		export QT_BASE_DIR=/opt/qt512
		export QTDIR=$QT_BASE_DIR
		export PATH=$QT_BASE_DIR/bin:$PATH
		export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
		export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
		linuxdeploy-x86_64.AppImage --appdir=AppDir --desktop-file=./AppDir/usr/share/applications/sayonara.desktop --plugin=qt --custom-apprun=../AppImage/AppRun

		# Workaround for https://github.com/linuxdeploy/linuxdeploy/issues/100
		cp ../AppImage/AppRun AppDir/
		chmod a+x AppDir/AppRun

		appimagetool-x86_64.AppImage AppDir
		;;
	*)
		echo "Usage $0 build|deploy"
		exit 1
		;;
esac

exit 0

