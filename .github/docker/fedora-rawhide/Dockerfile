FROM docker.io/fedora:rawhide

RUN dnf update -y
RUN dnf install -y cmake \
	ccache \
	python3-devel \
	gcc-c++ \
	git \
	boost-devel \
	boost-python3-devel \
	zlib-devel \
	SDL2-devel \
	freetype-devel \
	openal-soft-devel \
	libogg-devel \
	libvorbis-devel \
	glew-devel
	# godot3-headless # wait for update godot3-headless at https://packages.fedoraproject.org/pkgs/godot3/godot3-headless/fedora-rawhide.html since 2025-01-30

