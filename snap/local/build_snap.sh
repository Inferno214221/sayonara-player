#!/bin/bash

: <<'Comments'

This is a script for building the snap package of the sayonara player
cd into the repo root directory an launch snap/local/build_snap.sh

Comments


# get version from CMakeLists
versionFinderRegex='\d+(?:\.\d+)+-(stable|beta)\d?'
version=$(cat CMakeLists.txt | grep -oP $versionFinderRegex)

if [[ -z "$version" ]]; then
   echo "Could not find a version in CMakeLists.txt" 1>&2
   exit 1
fi

commitid=$(git rev-parse --short HEAD)

# replacing values in snapcraft.yaml
sed -i "s/^version:.*$/version: ${version}-${commitid}/" snap/snapcraft.yaml

# running snapcraft
cd snap
snapcraft clean
snapcraft --debug