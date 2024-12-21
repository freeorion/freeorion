FROM ghcr.io/void-linux/void-buildroot-musl:20241214R1

RUN echo noextract=/etc/hosts > /etc/xbps.d/test.conf

RUN xbps-install -Suy || xbps-install -uy xbps
RUN xbps-install -Suy 

RUN xbps-install -y cmake godot boost-devel boost-python3 libvorbis-devel freetype-devel glew-devel libopenal-devel python3-devel SDL2-devel

