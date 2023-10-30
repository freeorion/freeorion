#!/bin/bash -e

sudo apt update
sudo apt install -y libboost1.74-all-dev \
	libglew-dev \
	libogg-dev \
	libopenal-dev \
	libsdl2-dev \
	libvorbis-dev \
	cppcheck \
	doxygen \
	python3-pip \
	ccache \
	clang-15 \
	godot3-server

