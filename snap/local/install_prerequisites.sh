#!/bin/bash

: <<'Comments'

This is a script for installing the prerequisites to build the snap package of the sayonara player

Comments

apt-get update
apt-get install snapd -y


snap install snapcraft --classic
snap install multipass --beta --classic
