#!/bin/bash

ps auxx | grep 'Xvfb' | grep -v grep

export SDL_VIDEODRIVER=x11

while [ ! -f screenshot.png ]; do
  echo "::group::Launching freeorion at $(pwd)"

  rm -f freeoriond.log freeorion.log

  /snap/bin/freeorion.freeoriond \
      --hostless \
      --setup.seed RANDOM \
      --network.server.human.max 1 \
      --network.server.unconn-human-empire-players.max 0 \
      --setup.ai.player.count 3 \
      -q \
      --network.server.take-over-ai 1 \
      --setup.rules.RULE_TECH_COST_FACTOR 0.1 \
      --setup.rules.RULE_BUILDING_COST_FACTOR 0.1 \
      --setup.rules.RULE_SHIP_PART_COST_FACTOR 0.1 \
      --setup.rules.RULE_SHIP_HULL_COST_FACTOR 0.1 \
      --setup.star.count 140 \
      --setup.galaxy.shape RANDOM \
      --setup.galaxy.age GALAXY_SETUP_RANDOM \
      --setup.planet.density GALAXY_SETUP_RANDOM \
      --setup.starlane.frequency GALAXY_SETUP_RANDOM \
      --setup.specials.frequency GALAXY_SETUP_RANDOM \
      --setup.native.frequency GALAXY_SETUP_RANDOM \
      --setup.monster.frequency MONSTER_SETUP_RANDOM \
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
  if timeout 30s tail -F --retry -n +1 freeorion.log 2>/dev/null | grep -q -m 1 "Checksum received from server matches client checksum."; then
    sleep 5
    echo "Sending Ctrl+H"
    FO_WIN_ID=$(xdotool search --name "^FreeOrion" | head -n 1)
    if [ -n "$FO_WIN_ID" ]; then
      xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+h
    else
      xdotool key --delay 150 ctrl+h
    fi
    echo "Sent Ctrl+H to window $FO_WIN_ID"
    if timeout 30s tail -F --retry -n +1 freeorion.log 2>/dev/null | grep -q -m 1 "Zoomed to capital system "; then
      sleep 10
      import -display :99 -window root $(pwd)/screenshot.png
    else
      echo "::error::Skip screenshot because can't zoom"
    fi
  else
    echo "::error::Skip screenshot because checksum failed"
  fi
  kill -9 ${FOPID}
  wait ${FOPID}
  echo FreeOrion return code $?
  kill -9 ${FODPID}
  wait ${FODPID}
  echo FreeOrionD return code $?
  
  echo "::endgroup::"
done
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

