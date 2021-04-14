#!/bin/bash

LIBGL_DEBUG=verbose glxinfo

ps auxx | grep 'Xvfb' | grep -v grep

echo = Launching freeorion =

export SDL_VIDEODRIVER=x11

freeorion &
FOPID=$!
sleep 10
kill ${FOPID}
wait ${FOPID}
echo Freeorion return code $?
cat ~/.local/share/freeorion/freeorion.log || echo No log file

