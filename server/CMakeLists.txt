cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeoriond)

message("-- Configuring freeoriond")

set(sources
    ../combat/CombatSystem.cpp
    ../network/ServerNetworking.cpp
    ../server/SaveLoad.cpp
    ../server/ServerApp.cpp
    ../server/ServerFSM.cpp
    ../server/dmain.cpp
    ../universe/Universe.cpp
    ../util/AppInterface.cpp
)

add_definitions(-DFREEORION_BUILD_SERVER)

add_executable(freeoriond ${sources})

set_target_properties(
    freeoriond
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CLEAN_DIRECT_OUTPUT 1
    LABELS "${PROJECT_NAME}"
)

target_link_libraries(freeoriond freeorion_lib ${Boost_LIBRARIES})
