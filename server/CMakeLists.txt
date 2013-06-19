cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeoriond)

message("-- Configuring freeoriond")

add_definitions(-DFREEORION_BUILD_SERVER)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
endif ()

add_executable(freeoriond
    ../network/ServerNetworking.cpp
    ../server/SaveLoad.cpp
    ../server/ServerApp.cpp
    ../server/ServerFSM.cpp
    ../server/dmain.cpp
    ../universe/UniverseServer.cpp
    ../util/AppInterface.cpp
    ../combat/CombatSystem.cpp
)

target_link_libraries(freeoriond
    freeorioncommon
    freeorionparse
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeoriond
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

