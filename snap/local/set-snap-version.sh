#!/bin/bash

script_name=$0

: <<'Comments'

This is a script for setting the git revision in the latest commit
cd into the repo root directory an launch snap/local/${script_name}

Comments

echo "${script_name}: Starting ${script_name}"
echo "${script_name}: We are in directory '$(pwd)'"
echo "${script_name}: Get revision from latest commit"
revision=$(git rev-parse HEAD)

if [[ -z "${revision}" ]]; then
   echo "${script_name}: Couldn't retreive revision from git - using gitlab variable 'CI_COMMIT_SHA'"
   revision=${CI_COMMIT_SHA}
fi
echo "${script_name}: revision is ${revision}"

# replace local values with repo values in snapcraft.yaml
sed -i \
    -e "s/source-type:.*$/source-type: git/" \
    -e "s/source:.*$/source: https:\/\/gitlab.com\/luciocarreras\/sayonara-player.git/" \
    -e "s/# source-commit:.*$/source-commit: '${revision}'/" \
    -e "s/# source-depth/source-depth/" \
    "./snap/snapcraft.yaml"

echo "${script_name}: Finished ${script_name}"
