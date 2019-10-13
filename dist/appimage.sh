#!/bin/sh

# run in docker container luciocarreras/sayonara-appimage

set -e

case "$1" in
	build)
		mkdir -p build && cd build
		cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -DWITH_COTIRE=1 -DWITH_TESTS=1 -DCMAKE_PREFIX_PATH=/opt/qt512 -DCXX_COMPILER=clang++
		make -j6
		make install DESTDIR=AppDir
		;;
	test)
		cd build 
		make test
		;;
	deploy)
		cd build
		export EXTRA_QT_PLUGINS="iconengines,sqldrivers/libqsqlite.so,platforms/libqxcb.so" 
		export QMAKE="/usr/lib/x86_64-linux-gnu/qt5/bin/qmake" 
		linuxdeploy-x86_64.AppImage --appdir=AppDir --desktop-file=./AppDir/usr/share/applications/sayonara.desktop --plugin=qt
		appimagetool-x86_64.AppImage AppDir
		;;
	*)
		echo "Usage $0 build|test|deploy"
		exit 1
		;;
esac

exit 0

