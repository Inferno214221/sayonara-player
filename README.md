# Project Title

One Paragraph of project description goes here

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

In order to compile you need the following packages
 * cmake >= 3.8
 * g++ >= 7.1 or clang++ >= 6.0
 * Qt >= 5.6: Core, Widgets, Network, Xml, Sql, Sqlite
 * Gstreamer development files, GStreamer plugins
 * zlib development files
 
### Installing packages on Debian/Ubuntu/Mint
`apt-get install cmake pkg-config qtbase5-dev qttools5-dev qttools5-dev-tools zlib1g-dev libqt5sql5-sqlite libgstreamer-1.0-dev libgstreamer-plugins.bad-1.0-dev libgstreamer-plugins-base1.0-dev gstreamer-1.0-plugins-bad gstreamer-1.0-plugins-ugly vorbis-tools lame libmp3lame cmake`

### Installing packages on Fedora/Suse
`dnf install cmake gcc-c++ desktop-file-utils libappstream-glib qt5-qtbase-devel qt5-qttools-devel gstreamer1-plugins-base-devel zlib-devel hicolor-icon-theme`
optional: gstreamer1-plugins-ugly lame (use rpmforge or rpmfusion)
 
 
## Building
### Calling CMake

`mkdir build`
`cd build`
`cmake ..`

Additional useful arguments:
 * Build Type: -DCMAKE_BUILD_TYPE=Debug|Release|RelWithDebSymbols
 * Install prefix: -DCMAKE_INSTALL_PREFIX=<path for installing>
 * Enable/Disable testing: -DWITH_TESTS=ON|OFF 
 * Enable/Disable documentation: -DWITH_DOC=ON|OFF
 
### Calling make
`make -j<number of processes>`
e.g. `make -j8`

### Installing
`make install`

## Running the tests
`make test`


## Contributing

Please read [CONTRIBUTING.md](https://gist.github.com/PurpleBooth/b24679402957c63ec426) for details on our code of conduct, and the process for submitting pull requests to us.

## Versioning

We use [SemVer](http://semver.org/) for versioning. For the versions available, see the [tags on this repository](https://github.com/your/project/tags). 

## Authors

**Lucio Carreras** - *Project owner* luciocarreras@gmail.com

## License

This project is licensed under the GPL Version 3 License - see the [LICENSE](LICENSE) file for details
