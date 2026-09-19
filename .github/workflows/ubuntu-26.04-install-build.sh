#!/bin/bash -e

sudo dpkg --add-architecture amd64
cat << 'EOF' | sudo tee /etc/apt/sources.list.d/amd64.list
deb [arch=amd64] http://archive.ubuntu.com/ubuntu resolute main universe
deb [arch=amd64] http://archive.ubuntu.com/ubuntu resolute-updates main universe
deb [arch=amd64] http://security.ubuntu.com/ubuntu resolute-security main universe
EOF
sudo apt update
sudo apt install -y libboost-all-dev \
	libglew-dev \
	libogg-dev \
	libopenal-dev \
	libsdl2-dev \
	libvorbis-dev \
	libfreetype-dev \
	cppcheck \
	doxygen \
	python3-pip \
	ccache \
	godot3-server:amd64

