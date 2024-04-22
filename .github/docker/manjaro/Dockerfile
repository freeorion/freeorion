FROM manjarolinux/base:latest

RUN pacman-db-upgrade
RUN pacman -Syyu --noconfirm
RUN pacman -S --noconfirm boost sdl2 freetype2 openal glew cmake make gcc base-devel ccache git libogg libvorbis

ENV CC=gcc
ENV CXX=g++

