#!/bin/bash

: <<'Comments'

This is a script for setting the versions in the manifest for the snap package of the sayonara player
cd into the repo root directory an launch snap/local/Set-SnapVersion.sh

Comments

echo "Set-SnapVersion: Starting Set-SnapVersion"
echo "Set-SnapVersion: We are in directory '$(pwd)'"

# get version from CMakeLists
echo "Set-SnapVersion: Get Version from CMakeLists.txt"
versionFinderRegex='\d+(?:\.\d+)+-(stable|beta)\d?'
version=$(cat CMakeLists.txt | grep -oP $versionFinderRegex)

if [[ -z "$version" ]]; then
   echo "Set-SnapVersion: Get Version from CMakeLists.txt: Could not find a version in CMakeLists.txt" 1>&2
   exit 1
fi
echo "Set-SnapVersion: Get Version from CMakeLists.txt finished. Version to build is $version"

echo "Set-SnapVersion: Get CommitId from latest Commit"
commitid=$(git rev-parse HEAD)

if [[ -z "$commitid" ]]; then
   echo "Set-SnapVersion: Get CommitId from latest Commit. Couldn't retreive commitid from git - using gitlab variable 'CI_COMMIT_SHA'"
   commitid=$CI_COMMIT_SHA
fi
echo "Set-SnapVersion: Get CommitId from latest Commit. Commit id is ${commitid}"

# replacing values in snapcraft.yaml
echo "Set-SnapVersion: Replacing Version in snapcraft.yaml"
sed -i "s/^version:.*$/version: ${version}-${commitid:0:8}/" snap/snapcraft.yaml
sed -i "s/source-commit:.*$/source-commit: '${commitid}'/" snap/snapcraft.yaml
echo "Set-SnapVersion: Replacing Version in snapcraft.yaml finished"

echo "Set-SnapVersion: Finished Set-SnapVersion"