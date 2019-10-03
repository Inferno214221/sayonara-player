# Sayonara Player
[https://sayonara-player.com]()

Sayonara is a small, clear and fast audio player for Linux written in C++, supported by the Qt framework. It uses GStreamer as audio backend. Sayonara is open source and uses the GPLv3 license. One of Sayonara's goals is intuitive and easy usablility. Currently, it is only available for Linux and BSD. 

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

In order to compile you need the following packages
 * cmake >= 3.8
 * g++ >= 7.1 or clang++ >= 6.0
 * Qt >= 5.6: Core, Widgets, Network, Xml, Sql, Sqlite
 * Gstreamer development files, GStreamer plugins
 * zlib development files

#### Installing packages on Debian/Ubuntu/Mint

`apt-get install cmake pkg-config qtbase5-dev qttools5-dev qttools5-dev-tools zlib1g-dev libqt5sql5-sqlite libgstreamer-1.0-dev libgstreamer-plugins.bad-1.0-dev libgstreamer-plugins-base1.0-dev gstreamer-1.0-plugins-bad gstreamer-1.0-plugins-ugly vorbis-tools lame libmp3lame cmake`

#### Installing packages on Fedora/Suse

`dnf install cmake gcc-c++ desktop-file-utils libappstream-glib qt5-qtbase-devel qt5-qttools-devel gstreamer1-plugins-base-devel zlib-devel hicolor-icon-theme`
optional: gstreamer1-plugins-ugly lame (use rpmforge or rpmfusion)

#### Installing packages on Mageia
`urpmi gcc-c++ cmake desktop-file-utils lib64 appstream-glib lib64gstreamer1.0-devel lib64gstreamer-plugins-base1.0-devel lib64zlib-devel lib64qt5base5-devel lib64qt5core-devel lib64qt5dbus-devel lib64qt5gui-devel lib64qt5network-devel lib64qt5xml-devel lib64qt5sql-devel lib64qt5network-devel lib64qt5help-devel git`

### Get the source code

#### Archive
A source code archive can be fetched from Gitlab [https://gitlab.com/luciocarreras/sayonara-player]() or from the [Sayonara homepage](https://sayonara-player.com/downloads.php). The archive is extracted by using

`tar xzf sayonara-player-<version>.tar.gz`

#### Git
The source code can also be fetched via git from Gitlab [https://gitlab.com/luciocarreras/sayonara-player.git](). Sayonara's own Git repository will not be accessible anymore in the future.

##### Cloning
Fetch the source code via

`git clone https://gitlab.com/luciocarreras/sayonara-player.git`

If you are told to use another branch (typically beta or testing), use

`git clone https://gitlab.com/luciocarreras/sayonara-player.git -b <branch-name>`


### Build Sayonara
```
mkdir build
cd build
cmake ..

make -j<number of processes>
make install
```

#### Additional useful CMake arguments:
 * Build Type: `-DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebSymbols`
 * Install prefix: `-DCMAKE_INSTALL_PREFIX=<path for installing>`
 * Enable/Disable testing: `-DWITH_TESTS=ON|OFF`
 * Enable/Disable documentation: `-DWITH_DOC=ON|OFF`


### Running tests
`make test`

## Versioning
Version scheme is `<Major>.<Minor>.<Update>-[beta|stable]<Bugfix>`

where `<Major>` marks really big changes, `<Minor>` marks changes which may affect stability, `<Update>` marks changes which do not affect stability and `<Bugfix>` marks a single bugfix

## Authors
**Lucio Carreras** - *Project owner* luciocarreras@gmail.com

## License
This project is licensed under the GPL Version 3 License - see the [LICENSE](LICENSE) file for details
