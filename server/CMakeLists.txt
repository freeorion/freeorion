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
    ../util/VarText.cpp
    ../combat/CombatSystem.cpp
)

target_link_libraries(freeoriond
    core
    parse
    log4cpp
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeoriond
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

if (WIN32)
    set_target_properties(freeoriond
        PROPERTIES
        COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
        LINK_FLAGS /NODEFAULTLIB:LIBCMT
    )
endif ()
