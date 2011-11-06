cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeoriond)

message("-- Configuring freeoriond")

set(THIS_EXE_SOURCES
    ../combat/CombatSystem.cpp
    ../network/ServerNetworking.cpp
    ../server/SaveLoad.cpp
    ../server/ServerApp.cpp
    ../server/ServerFSM.cpp
    ../server/dmain.cpp
    ../universe/Universe.cpp
    ../util/AppInterface.cpp
    ../util/VarText.cpp
)

add_definitions(-DFREEORION_BUILD_SERVER)

set(THIS_EXE_LINK_LIBS core_static parse_static)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
endif ()

executable_all_variants(freeoriond)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
    set_target_properties(freeoriond
        PROPERTIES
        COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
        LINK_FLAGS /NODEFAULTLIB:LIBCMT
    )
endif ()
