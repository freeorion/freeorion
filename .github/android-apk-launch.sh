#!/bin/bash -e

echo "::group::Installing APK"
adb devices
adb install freeorion.apk
adb logcat -c
echo "::endgroup::"

echo "::group::Starting APK"
adb shell am start -W -n org.godotengine.freeoriongodotclient/com.godot.game.GodotApp
echo "::endgroup::"

sleep 180

echo "::group::Stopping APK"
adb shell am force-stop org.godotengine.freeoriongodotclient
echo "::endgroup::"

echo "::group::Getting logs"
adb logcat -d >logcat.log
adb exec-out run-as org.godotengine.freeoriongodotclient cat files/freeorion-godot.log >freeorion-godot.log 2>&1
echo "::endgroup::"

echo "::group::Checking errors"
ERRORS=$(grep -B1 "\[error\] godot : \(PythonParser\|Parse\|ReportParseError\)" freeorion-godot.log || true)
if [ -n "$ERRORS" ]; then
  echo "$ERRORS" | while IFS= read -r line; do
    echo "::error title=Parser::$line"
  done
fi
echo "::endgroup::"

echo "::group::Checking finish"
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

