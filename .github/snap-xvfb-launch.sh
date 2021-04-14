#!/bin/bash

xauth list

ps auxx | grep 'Xvfb' | grep -v grep

echo = Launching freeorion =
echo = Display ${DISPLAY} =
echo = XAuthority ${XAUTHORITY} =

export SDL_VIDEODRIVER=x11

freeorion &
FOPID=$!
sleep 10
kill ${FOPID}
wait ${FOPID}
echo Freeorion return code $?
cat ~/.local/share/freeorion.log || echo No log file

