#!/bin/bash

ps auxx | grep 'Xvfb' | grep -v grep

export SDL_VIDEODRIVER=x11

echo "::group::Launching freeorion at $(pwd)"

/snap/bin/freeorion.freeoriond \
    --hostless \
    --setup.seed RANDOM \
    --network.server.human.max 1 \
    --network.server.unconn-human-empire-players.max 0 \
    --setup.ai.player.count 3 \
    -q \
    --network.server.take-over-ai 1 \
    --log-file $(pwd)/freeoriond.log &
FODPID=$!
echo "FreeOrionD started pid ${FODPID}"

sleep 25 # Let's AI play a little

LIBGL_DEBUG=verbose /snap/bin/freeorion \
  --log-file $(pwd)/freeorion.log \
  --audio.effects.enabled 0 \
  --audio.music.enabled 0 \
  --video.fullscreen.width 1280 \
  --video.windowed.width 1280 \
  --video.fullscreen.height 720 \
  --video.windowed.height 720 \
  --video.windowed.top 0 \
  -A \
  --setup.multiplayer.host.address localhost \
  --setup.multiplayer.player.name AI_$(( $RANDOM % 3 + 1 )) &
FOPID=$!
echo "FreeOrion started pid ${FOPID}"
sleep 10
xdotool key ctrl+h
sleep 5
import -display :99 -window root $(pwd)/screenshot.png
kill -9 ${FOPID}
wait ${FOPID}
echo FreeOrion return code $?
kill -9 ${FODPID}
wait ${FODPID}
echo FreeOrionD return code $?

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

