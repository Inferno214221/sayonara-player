Sayonara

== How to build ==

Please also look at https://sayonara-player.com/howto-build.php
If you have further questions visit the forum at https://sayonara-player.com/forum

Libraries

    - Qt >= 5.3: Core, Widgets, Network, Xml, Sql, Sqlite http://qt.nokia.com/products/
    - Taglib http://developer.kde.org/~wheeler/taglib.html
    - Gstreamer development files, GStreamer plugins 
    - zlib

1. Linux

	You need 
	* g++ >= 4.8 (important due to C++x11 standard)
	* cmake
	* Qt >= 5.3 + development files 
	* libtaglib (>= 1.11)
	* Gstreamer 1.0 + development files (libgstreamer1.0, libgstreamer-plugins-base1.0)
        * zlib development files

	= Build =

	* cmake . -DCMAKE_BUILD_TYPE=[Release|Debug|RelWithDebInfo|None] -DCMAKE_INSTALL_PREFIX=/usr (default: Release, /usr/local)
	* make
	* make install (as root)
	* sayonara
	
	1.1 Debian/Ubuntu/Mint:  
		apt-get install cmake libgstreamer-1.0-dev libgstreamer-plugins.bad-1.0-dev libgstreamer-plugins-base1.0-dev \
		libtag1-dev pkg-config qtbase5-dev qttools5-dev qttools5-dev-tools zlib1g-dev build-essential
		
		optional: vorbis-tools lame libmp3lame

	1.2 Fedora/Suse: 
		dnf install cmake gcc-c++ desktop-file-utils libappstream-glib qt5-qtbase-devel qt5-qttools-devel \
		gstreamer1-plugins-base-devel taglib-devel zlib-devel hicolor-icon-theme

		optional: gstreamer1-plugins-ugly lame (use rpmforge or rpmfusion)
	

