#!/bin/bash

ps auxx | grep 'Xvfb' | grep -v grep

export SDL_VIDEODRIVER=x11

echo "::group::Launching freeorion at $(pwd)"

LIBGL_DEBUG=verbose /snap/bin/freeorion \
  -q \
  --log-file $(pwd)/freeorion.log \
  --audio.effects.enabled 0 \
  --audio.music.enabled 0 \
  --auto-advance-n-turns 100 \
  --video.fullscreen.width 1280 \
  --video.windowed.width 1280 \
  --video.fullscreen.height 720 \
  --video.windowed.height 720 \
  --video.windowed.top 0 \
  --setup.ai.player.count 3 &
FOPID=$!
echo "FreeOrion started pid ${FOPID}"
sleep 25
import -display :99 -window root $(pwd)/screenshot.png
kill -9 ${FOPID}
wait ${FOPID}
echo Freeorion return code $?

echo "::endgroup::"
echo "::group::Launching freeorion godot"

export SDL_VIDEODRIVER=x11

LIBGL_DEBUG=verbose /snap/bin/freeorion.freeorion-godot --log-file $(pwd)/freeorion-godot.log &
FOPID=$!
echo "FreeOrion Godot started pid ${FOPID}"
sleep 10
kill -9 ${FOPID}
wait ${FOPID}
echo Freeorion Godot return code $?
echo "::endgroup::"

