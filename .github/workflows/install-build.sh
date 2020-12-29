#!/bin/bash -e

sudo apt update
sudo apt install -y libboost1.62-all-dev \
	libglew-dev \
	libogg-dev \
	libopenal-dev \
	libsdl2-dev \
	libvorbis-dev \
	cppcheck \
	doxygen \
	python3-pip \
	ccache
python3 -m pip install flake8==3.7.9

