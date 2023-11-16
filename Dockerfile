# Use an official Python runtime as a parent image
FROM ubuntu:focal

ENV DEBIAN_FRONTEND "noninteractive"
RUN apt-get update -qq && \
	apt-get install -y -qq --no-install-recommends \
	gnupg-agent \
	software-properties-common \
	wget \ 
	apt-utils

# Qt 
RUN add-apt-repository --yes ppa:beineri/opt-qt-5.15.4-focal

# Clang
RUN echo "deb http://apt.llvm.org/focal/ llvm-toolchain-focal-17 main" > /etc/apt/sources.list.d/clang.list
RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key| apt-key add -
ENV CC clang-17
ENV CXX clang++-17

RUN	apt-get update -qq && \
	apt-get install -y --no-install-recommends \
	make \
	coreutils \
	clang-17 \
	curl \
	git \
	libgstreamer-plugins-bad1.0-dev \
	libgstreamer-plugins-base1.0-dev \
	libgstreamer1.0-dev \
	gstreamer1.0-plugins-bad \
	gstreamer1.0-plugins-base \
	gstreamer1.0-plugins-base-apps \
	gstreamer1.0-plugins-good \
	gstreamer1.0-plugins-ugly \
	gstreamer1.0-pulseaudio \
	gstreamer1.0-tools \
	gtk-update-icon-cache \
	libtag1-dev \
	pkg-config \
	qttools5-dev-tools \
	qt515base \
	qt515svg \
	qt515tools \
	qt515translations \
	qt515x11extras \
	qt515xmlpatterns \
	sudo \
	zlib1g-dev \
    libgl1-mesa-dev

RUN ln -sf /opt/qt515 /opt/qt

# Needed for linuxdeploy images
RUN apt-get install -y -qq --no-install-recommends \
	ca-certificates \
	file \
	libfuse2 \
	openssl \
	wget

# CMake
ENV CMAKE_VERSION_NR "3.26.5"
ENV CMAKE_VERSION "${CMAKE_VERSION_NR}-linux-x86_64"
RUN wget "https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION_NR}/cmake-${CMAKE_VERSION}.tar.gz"
RUN tar xzf "cmake-${CMAKE_VERSION}.tar.gz"
RUN cp -r "cmake-${CMAKE_VERSION}/bin" /usr/local
RUN cp -r "cmake-${CMAKE_VERSION}/doc" /usr/local
RUN cp -r "cmake-${CMAKE_VERSION}/share" /usr/local
RUN rm -rf "cmake-*"

# App Image
RUN wget "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
RUN wget "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"
RUN wget "https://github.com/AppImage/AppImageKit/releases/download/continuous/appimagetool-x86_64.AppImage"
RUN mv "linuxdeploy-x86_64.AppImage" "linuxdeploy-plugin-qt-x86_64.AppImage" "appimagetool-x86_64.AppImage" /usr/local/bin/
RUN chmod a+x /usr/local/bin/*.AppImage

# Transifex
RUN mkdir -p /tmp/tx && cd /tmp/tx && curl -o- https://raw.githubusercontent.com/transifex/cli/master/install.sh | bash
ENV PATH "/tmp/tx:${PATH}"

#ARG userid=1000
#ARG groupid=1000
#ARG username=root
#ARG groupname=root

#RUN groupadd -o -g ${groupid} ${groupname} && \
#	useradd -o --no-log-init -m -u ${userid} -g ${groupid} ${username} && \
#	adduser ${username} sudo

# USER ${username}

CMD ["/bin/bash"]
