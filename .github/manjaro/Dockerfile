FROM manjarolinux/base:latest

RUN pacman-db-upgrade
RUN pacman -Syyu --noconfirm
RUN pacman -S --noconfirm boost sdl2 freetype2 godot openal glew cmake make gcc base-devel
