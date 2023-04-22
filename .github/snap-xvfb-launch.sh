#!/bin/bash

LIBGL_DEBUG=verbose glxinfo

ps auxx | grep 'Xvfb' | grep -v grep

export SDL_VIDEODRIVER=x11

echo = Launching freeorion =

LIBGL_DEBUG=verbose freeorion.freeorion &
FOPID=$!
sleep 10
kill ${FOPID}
wait ${FOPID}
echo Freeorion return code $?
cat ~/.local/share/freeorion/freeorion.log || echo No log file

echo = Launching freeorion godot =

export SDL_VIDEODRIVER=x11

LIBGL_DEBUG=verbose freeorion.freeorion-godot &
FOPID=$!
sleep 10
kill ${FOPID}
wait ${FOPID}
echo Freeorion Godot return code $?
cat ~/.local/share/freeorion/freeorion-godot.log || echo No log file

