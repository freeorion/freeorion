#!/bin/bash -e

echo "::group::Installing APK"
adb devices
adb shell settings put secure immersive_mode_confirmations confirmed
adb install freeorion.apk
sleep 1
adb logcat -c
echo "::endgroup::"

echo "::group::Search APK lanucher"
LAUNCHER=""
for i in $(seq 1 30); do
  LAUNCHER=$(adb shell cmd package resolve-activity --brief --user 0 -a android.intent.action.MAIN -c android.intent.category.LAUNCHER org.godotengine.freeoriongodotclient 2>/dev/null | tail -1)
  case "$LAUNCHER" in
    */*) break ;;
    *) LAUNCHER="" ;;
  esac
  if [ -z "$LAUNCHER" ]; then
    echo "Waiting for APK launcher to be resolvable (attempt $i / 30)..."
    sleep 2
  fi
done
if [ -z "$LAUNCHER" ]; then
  echo "::error title=Launch::APK launcher could not be resolved"
  echo "::endgroup::"
  exit 1
fi
echo "::endgroup::"

echo "::group::Starting APK"
adb logcat "*:W" &
LOGCAT_PID=$!
adb shell am start -W -n "$LAUNCHER" --ez quickstart true --ei auto-advance-n-turns 100 --ei setup.ai.player.count 2
kill "${LOGCAT_PID}"
echo "::endgroup::"

echo "::group::Waiting..."
if [ "$1" = "4" ]; then
  sleep 420
else
  sleep 180
fi
echo "::endgroup::"

echo "::group::Taking screenshot"
adb exec-out screencap -p > android-screenshot.png || echo "Failed to take screenshot"
echo "::endgroup::"

if [ "$1" = "4" ] && [ "$2" = "true" ]; then
  echo "::group::Dumping server stacks"
  adb root >/dev/null 2>&1 || true
  adb wait-for-device
  STACK_PIDS=$(adb shell pidof org.godotengine.freeoriongodotclient:freeoriond 2>/dev/null | tr -d '\r')
  if [ -n "$STACK_PIDS" ]; then
    for P in $STACK_PIDS; do
      echo "Dumping stack for server pid $P"
      if ! adb shell debuggerd -b "$P" > "server-stack-$P.log" 2>&1 && \
         ! adb shell su 0 debuggerd -b "$P" > "server-stack-su-$P.log" 2>&1; then
        echo "Failed to dump stack for pid $P"
      fi
      sleep 2
    done
  else
    echo "::error title=Server::Server process not found"
  fi
  echo "::endgroup::"
fi

echo "::group::Stopping APK"
adb shell am force-stop org.godotengine.freeoriongodotclient
echo "::endgroup::"

echo "::group::Getting logs"
adb logcat -d >logcat.log
adb exec-out run-as org.godotengine.freeoriongodotclient cat files/freeorion-godot.log >freeorion-godot.log 2>&1
if [ "$1" = "4" ]; then
  adb exec-out run-as org.godotengine.freeoriongodotclient cat files/freeoriond.log >freeoriond.log 2>&1
  adb exec-out run-as org.godotengine.freeoriongodotclient cat files/AI_1.log >AI_1.log 2>&1
  adb exec-out run-as org.godotengine.freeoriongodotclient cat files/AI_2.log >AI_2.log 2>&1
fi
echo "::endgroup::"

echo "::group::List logs"
adb exec-out run-as org.godotengine.freeoriongodotclient ls -l files/
echo "::endgroup::"

echo "::group::Checking errors"
ERRORS=$(grep -B1 "\[error\] godot : \(PythonParser\|Parse\|ReportParseError\|PythonCommon\)" freeorion-godot.log || true)
if [ -n "$ERRORS" ]; then
  echo "$ERRORS" | while IFS= read -r line; do
    echo "::error title=Parser::$line"
  done
fi
echo "::endgroup::"

echo "::group::Checking parsing finish"
if ! grep -q "\[debug\] godot : FreeOrionNode.cpp:[0-9]\+ : FreeOrionNode::parsing_thread(): Freeorion parsing stopped" freeorion-godot.log; then
  echo "::error title=Parser::Parsing thread did not stopped!"
  echo "::endgroup::"
  exit 1
fi
if [ -n "$ERRORS" ]; then
  echo "::endgroup::"
  exit 1
fi
echo "::endgroup::"

