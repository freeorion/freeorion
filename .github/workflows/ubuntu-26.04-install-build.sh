#!/bin/bash -e

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
	godot3-server

