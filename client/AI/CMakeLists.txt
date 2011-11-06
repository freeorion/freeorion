cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorionca)

message("-- Configuring freeorionca")

set(THIS_EXE_SOURCES
    ../../AI/AIInterface.cpp
    ../../AI/PythonAI.cpp
    ../../client/ClientApp.cpp
    ../../client/ClientFSMEvents.cpp
    ../../client/AI/AIClientApp.cpp
    ../../client/AI/camain.cpp
    ../../combat/CombatSystem.cpp
    ../../network/ClientNetworking.cpp
    ../../python/PythonEnumWrapper.cpp
    ../../python/PythonUniverseWrapper.cpp
    ../../python/PythonEmpireWrapper.cpp
    ../../python/PythonLoggingWrapper.cpp
    ../../universe/Universe.cpp
    ../../util/AppInterface.cpp
    ../../util/VarText.cpp
)

list(APPEND MINIMUM_BOOST_COMPONENTS iostreams python)
find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS ${MINIMUM_BOOST_COMPONENTS})
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost libraries not found.")
endif ()

find_package(PythonLibs)
if (PYTHONLIBS_FOUND)
    include_directories(${PYTHON_INCLUDE_PATH})
else ()
    message(FATAL_ERROR " library not found.")
endif ()

add_definitions(-DFREEORION_BUILD_AI)

set(THIS_EXE_LINK_LIBS core_static parse_static ${PYTHON_LIBRARIES})

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
else ()
    set(THIS_EXE_LINK_LIBS ${THIS_EXE_LINK_LIBS} ${Boost_LIBRARIES})
endif ()

executable_all_variants(freeorionca)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
    set_target_properties(freeorionca
        PROPERTIES
        COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
        LINK_FLAGS /NODEFAULTLIB:LIBCMT
    )
endif ()
