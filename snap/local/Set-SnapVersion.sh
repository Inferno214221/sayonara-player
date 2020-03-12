#!/bin/bash

: <<'Comments'

This is a script for building the snap package of the sayonara player
cd into the repo root directory an launch snap/local/build_snap.sh

Comments

echo "Start Set-SnapVersion"
echo "were in directory '$(pwd)'"

# get version from CMakeLists
echo "Get Version from CMakeLists.txt"
versionFinderRegex='\d+(?:\.\d+)+-(stable|beta)\d?'
version=$(cat CMakeLists.txt | grep -oP $versionFinderRegex)

if [[ -z "$version" ]]; then
   echo "Could not find a version in CMakeLists.txt" 1>&2
   exit 1
fi

echo "Get CommitId from latest Commit"
commitid=$(git rev-parse --short HEAD)

if [[ -z "$commitid" ]]; then
   commitid=$CI_COMMIT_SHORT_SHA
fi

# replacing values in snapcraft.yaml
echo "Replacing Version in snapcraft.yaml"
sed -i "s/^version:.*$/version: ${version}-${commitid}/" snap/snapcraft.yaml
sed -i "s/source-commit:.*$/source-commit: ${commitid}/" snap/snapcraft.yaml

echo "Finished Set-SnapVersion"