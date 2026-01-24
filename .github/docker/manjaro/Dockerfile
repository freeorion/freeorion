FROM manjarolinux/base:latest

RUN pacman-db-upgrade
RUN pacman -Sy --noconfirm gnupg archlinux-keyring manjaro-keyring
RUN pacman-key --init
RUN pacman-key --populate archlinux manjaro
RUN pacman-key --refresh-keys
RUN pacman -Sc --noconfirm
RUN pacman -Syyu --noconfirm
RUN pacman -S --noconfirm boost sdl2 freetype2 godot openal glew cmake make gcc base-devel ccache git xorg-server-xvfb

ENV CC=gcc
ENV CXX=g++

