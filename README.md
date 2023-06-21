# Sayonara Player
[https://sayonara-player.com](https://sayonara-player.com)


[![Twitter URL](https://img.shields.io/twitter/follow/SayonaraPlayer?style=social)](https://twitter.com/sayonaraplayer)

<a href="https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=KD4GKTTLAP3JQ&source=url">
    <img src="https://img.shields.io/badge/PayPal-Donate-blue.svg"
            alt="PayPal: Donate">
</a>
<a href="https://patreon.com/sayonaraplayer">
    <img src="https://img.shields.io/badge/Patreon-Become a patron-orange.svg"
        alt="Patreon: Become a patron">
</a>
<a href="https://liberapay.com/LucioCarreras/donate">
    <img src="https://img.shields.io/liberapay/patrons/LucioCarreras.svg?logo=liberapay"
	    alt="Liberapay: Donate">
</a>
<a href="bitcoin:1M1pY98SGfyt2SR858Q14F8GCweHf17jQs?message=Sayonara%20Player%20Donation&time=1514892640">
    <img src="https://img.shields.io/badge/Bitcoin-Donate-green" 
        alt="Bitcoin">
</a>
![Maintenance](https://img.shields.io/maintenance/yes/2023)
![License](https://img.shields.io/badge/License-GPLv3-green)

![Debian package](https://img.shields.io/debian/v/sayonara/sid)
![Fedora package](https://img.shields.io/fedora/v/sayonara)
![AUR version](https://img.shields.io/aur/version/sayonara-player-git)

Sayonara is a small, clear and fast audio player for Linux written in C++, supported by the Qt framework. It uses GStreamer as audio backend. Sayonara is open source and uses the GPLv3 license. One of Sayonara's goals is intuitive and easy usablility. Currently, it is only available for Linux and BSD. 

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

In order to compile you need the following packages
 * cmake >= 3.8
 * g++ >= 7.1 or clang++ >= 6.0
 * Qt >= 5.8: Core, Widgets, Network, Xml, Sql, Sqlite, DBus
 * Gstreamer development files, GStreamer plugins
 * zlib development files

#### Installing packages on Debian/Ubuntu/Mint
```
apt install cmake \
	cmake \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-ugly \
	lame \
	libgstreamer1.0-dev \
	libgstreamer-plugins-bad1.0-dev \
	libgstreamer-plugins-base1.0-dev \
	libqt5sql5-sqlite \
	libqt5svg5-dev \
	libmp3lame0 \
	libtag1-dev \
	pkg-config \
	qtbase5-dev \
	qttools5-dev \
	qttools5-dev-tools \
	vorbis-tools \
	zlib1g-dev
```

#### Installing packages on Fedora/Suse
```
dnf install \
	cmake \
	desktop-file-utils \
	gcc-c++ \
	gstreamer1-plugins-base-devel \
	hicolor-icon-theme \
	libappstream-glib \
	qt5-qtbase-devel \
	qt5-qttools-devel \
	taglib-devel \
	zlib-devel
```

optional: gstreamer1-plugins-ugly lame (use rpmforge or rpmfusion)

#### Installing packages on Mageia
```
urpmi \
	appstream-glib \
	cmake \
	desktop-file-utils \
	gcc-c++ \
	lib64 \
	lib64gstreamer1.0-devel \
	lib64gstreamer-plugins-base1.0-devel \
	lib64qt5base5-devel \
	lib64qt5core-devel \
	lib64qt5dbus-devel \
	lib64qt5gui-devel \
	lib64qt5help-devel git
	lib64qt5network-devel \
	lib64qt5network-devel \
	lib64qt5sql-devel \
	lib64qt5xml-devel \
	lib64zlib-devel
```

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

 * Choose another compiler: `-DCMAKE_CXX_COMPILER=<compiler-name>`
 * Use a different Qt installation: `-DCMAKE_PREFIX_PATH=<path to qt> -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=ON`

### Running tests
`make test`

## Versioning
Version scheme is `<Major>.<Minor>.<Update>-[beta|stable]<Bugfix>`

where `<Major>` marks really big changes, `<Minor>` marks changes which may affect stability, `<Update>` marks changes which do not affect stability and `<Bugfix>` marks a single bugfix

## Authors
**Michael Lugmair** - *Project owner* sayonara-player at posteo dot org

## License
This project is licensed under the GPL Version 3 License - see the [LICENSE](LICENSE) file for details
