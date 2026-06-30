#!/bin/bash

ps auxx | grep 'Xvfb' | grep -v grep

export SDL_VIDEODRIVER=x11

for i in {1..2}; do
  echo "::group::Launching freeorion at $(pwd) - $i"

  rm -f freeoriond${i}.log freeorion${i}.log

  RANDOM_SEED=$SRANDOM

  /snap/bin/freeorion.freeoriond \
      --hostless \
      --setup.seed "$RANDOM_SEED" \
      --network.server.human.max 1 \
      --network.server.unconn-human-empire-players.max 0 \
      --setup.ai.player.count 3 \
      -q \
      --network.server.take-over-ai 1 \
      --setup.rules.RULE_TECH_COST_FACTOR 0.1 \
      --setup.rules.RULE_BUILDING_COST_FACTOR 0.1 \
      --setup.rules.RULE_SHIP_PART_COST_FACTOR 0.1 \
      --setup.rules.RULE_SHIP_HULL_COST_FACTOR 0.1 \
      --setup.star.count 130 \
      --setup.galaxy.shape RANDOM \
      --setup.galaxy.age GALAXY_SETUP_RANDOM \
      --setup.planet.density GALAXY_SETUP_RANDOM \
      --setup.starlane.frequency GALAXY_SETUP_RANDOM \
      --setup.specials.frequency GALAXY_SETUP_RANDOM \
      --setup.native.frequency GALAXY_SETUP_RANDOM \
      --setup.monster.frequency MONSTER_SETUP_RANDOM \
      --log-file $(pwd)/freeoriond${i}.log &
  FODPID=$!
  echo "FreeOrionD started pid ${FODPID}"

  sleep 25 # Let's AI play a little

  EMPIRE_ID=$(( $RANDOM % 3 + 1 ))
  MODE=$(( $RANDOM % 8 + 1 ))
  LIBGL_DEBUG=verbose /snap/bin/freeorion \
    --log-file $(pwd)/freeorion${i}.log \
    --audio.effects.enabled 0 \
    --audio.music.enabled 0 \
    --video.fullscreen.width 1280 \
    --video.windowed.width 1280 \
    --video.fullscreen.height 720 \
    --video.windowed.height 720 \
    --video.windowed.top 0 \
    -A \
    --setup.multiplayer.host.address localhost \
    --setup.multiplayer.player.name AI_${EMPIRE_ID} &
  FOPID=$!
  SCREENSHOT_TEXT=""
  echo "FreeOrion started pid ${FOPID}"
  if timeout 30s tail -F --retry -n +1 freeorion${i}.log 2>/dev/null | grep -q -m 1 "Checksum received from server matches client checksum."; then
    sleep 4
    FO_WIN_ID=$(xdotool search --name "^FreeOrion" | head -n 1)
    if [ -n "$FO_WIN_ID" ]; then
      xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+h
      if timeout 30s tail -F --retry -n +1 freeorion${i}.log 2>/dev/null | grep -q -m 1 "Zoomed to capital system "; then
        case $MODE in
          1)
            SCREENSHOT_TEXT="Home System"
            ;;
          2)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+shift+period
            sleep 3
            SCREENSHOT_TEXT="Next System"
            ;;
          3)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+p
            sleep 3
            SCREENSHOT_TEXT="Home System Production"
            ;;
          4)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+shift+period
            sleep 3
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+p
            sleep 3
            SCREENSHOT_TEXT="Next System Production"
            ;;
          5)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+g
            sleep 3
            SCREENSHOT_TEXT="Fleet"
            ;;
          6)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+r
            sleep 3
            SCREENSHOT_TEXT="Research"
            ;;
          7)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+d
            sleep 3
            SCREENSHOT_TEXT="Design"
            ;;
          8)
            xdotool key --window "$FO_WIN_ID" --delay 150 ctrl+i
            sleep 3
            SCREENSHOT_TEXT="Government"
            ;;
        esac
      else
        echo "::error::Skip screenshot because can't zoom"
      fi

      if [ -n "${SCREENSHOT_TEXT}" ]; then
        sleep 3
        import -display :99 -window root $(pwd)/screenshot${i}.png
        echo "screenshot-alt${i}=${SCREENSHOT_TEXT}" >> "$GITHUB_OUTPUT"
      else
        echo "::error::Missing screenshot text"
      fi
    else
      echo "::error::Skip screenshot because no FreeOrion window"
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

