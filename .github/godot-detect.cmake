find_program(GODOT_EXECUTABLE NAMES godot3-server godot-headless godot3 godot godot3-runner REQUIRED)
execute_process(COMMAND ${GODOT_EXECUTABLE} --version OUTPUT_VARIABLE GODOT_VERSION)
string(STRIP "${GODOT_VERSION}" GODOT_VERSION)
message("Found godot at ${GODOT_EXECUTABLE} version ${GODOT_VERSION}")
message("::set-output name=godot::${GODOT_EXECUTABLE}")
message("::set-output name=version::${GODOT_VERSION}")

