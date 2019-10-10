#!/bin/sh

# run in docker container luciocarreras/sayonara-fedora

set -e

case "$1" in
	build-and-deploy)
		pwd
		mkdir rpms

		mkdir -p /root/rpmbuild/SOURCES
		mkdir -p /root/rpmbuild/SPECS

		cd /root/rpmbuild/SOURCES
		git clone https://gitlab.com/luciocarreras/sayonara-player.git --branch 27-create-distribution-building-scripts
		rm -rf sayonara-player/.git*

		TODAY=$(LC_TIME="en_US.UTF-8" date +"%a %b %d %Y")
		VERSION=$(grep -oP '\d+(?:\.\d+)+-?\w+\d+' sayonara-player/CMakeLists.txt)
		VERSION_BASE=$(echo $VERSION | grep -Po "\d(\.\d)+")
		VERSION_RELEASE=$(echo $VERSION | grep -Po "\w+\d+")
		tar czf sayonara-player-${VERSION} sayonara-player

		# write changelog
		cp dist/sayonara.spec.in /root/rpmbuilds/SPEC/sayonara.spec
		cd /root/rpmbuilds/SPEC

		echo "* ${TODAY} Lucio Carreras <luciocarreras@gmail.com> - ${VERSION}" >> sayonara.spec
		echo "- Building test" >> sayonara.spec

		# build
		rpmbuild -ba sayonara.spec
		find /root/rpmbuild/RPMS -name "*.rpm" -exec cp {} /apps/rpms

		;;
	*)
		echo "Usage: $0 build-and-deploy"
		exit 1
		;;
esac

exit 0

