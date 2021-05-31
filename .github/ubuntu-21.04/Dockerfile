FROM docker.io/ubuntu:hirsute

ENV DEBIAN_FRONTEND=noninteractive

RUN echo "# don't install documentation inside container\n\
path-exclude=/usr/share/doc/*\n\
path-exclude=/usr/share/man/*\n\
path-exclude=/usr/share/cmake-3.18/Help/*\n\
" > /etc/dpkg/dpkg.cfg.d/docker-no-documentation

RUN echo "# don't install static libraries inside container\n\
path-exclude=/usr/lib/x86_64-linux-gnu/*.a\n\
path-exclude=/usr/lib/python3.9/config-x86_64-linux-gnu/*.a\n\
path-include=/usr/lib/x86_64-linux-gnu/libc_nonshared.a\n\
path-include=/usr/lib/gcc/x86_64-linux-gnu/11/libgcc.a\n\
path-include=/usr/lib/x86_64-linux-gnu/libmvec_nonshared.a\n\
path-include=/usr/lib/x86_64-linux-gnu/libpthread.a\n\
path-include=/usr/lib/x86_64-linux-gnu/libpthread-2.0.a\n\
path-include=/usr/lib/x86_64-linux-gnu/libpthread_nonshared.a\n\
" > /etc/dpkg/dpkg.cfg.d/docker-no-static-libraries

RUN echo "# don't install recommends and suggests packages\n\
APT::Install-Recommends \"false\";\n\
APT::Install-Suggests \"false\";\n\
" > /etc/apt/apt.conf.d/docker-no-suggests

RUN apt-get update --assume-yes
RUN apt-get install --assume-yes \
        git \
        g++-11 \
	make \
        cmake \
        python3 \
        ccache \
        coreutils \
        libpython3-dev \
        libfreetype6-dev \
        libglew-dev \
        libopenal-dev \
        libogg-dev \
        libvorbis-dev \
        zlib1g-dev \
        libsdl2-dev \
        mesa-common-dev \
        libpng-dev \
	libboost-all-dev \
	godot3-server \
	libalut-dev \
	libboost-all-dev \
	libjpeg-dev \
	libtiff-dev \
    && rm -rf \
        /var/lib/apt/lists/*

