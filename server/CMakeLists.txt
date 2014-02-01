cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeoriond)

message("-- Configuring freeoriond")

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS python REQUIRED)
find_package(PythonLibs 2 REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH}
)

add_definitions(-DFREEORION_BUILD_SERVER)

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()

set(freeoriond_LINK_LIBS
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
    ${Boost_LIBRARIES}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
endif ()

set (freeoriond_HEADER
    ../network/ServerNetworking.h
    SaveLoad.h
    ServerApp.h
    ServerFSM.h
)

set (freeoriond_SOURCE
    dmain.cpp
    SaveLoad.cpp
    ServerApp.cpp
    ServerFSM.cpp
    ../combat/CombatSystem.cpp
    ../network/ServerNetworking.cpp
    ../python/PythonEnumWrapper.cpp
    ../python/PythonLoggingWrapper.cpp
    ../python/PythonUniverseWrapper.cpp
    ../universe/PythonUniverseGenerator.cpp
    ../universe/UniverseGenerator.cpp
)

add_executable(freeoriond
    ${freeoriond_HEADER}
    ${freeoriond_SOURCE}
)

target_link_libraries(freeoriond
    ${freeoriond_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeoriond
    RUNTIME DESTINATION bin
    COMPONENT COMPONENT_FREEORION
)

