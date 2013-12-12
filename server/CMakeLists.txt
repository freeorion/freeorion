cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeoriond)

message("-- Configuring freeoriond")

list(APPEND MINIMUM_BOOST_COMPONENTS python)
find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS ${MINIMUM_BOOST_COMPONENTS})
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost libraries not found.")
endif ()

find_package(PythonLibs 2)
if (PYTHONLIBS_FOUND)
    include_directories(${PYTHON_INCLUDE_PATH})
else ()
    message(FATAL_ERROR "Python2 library not found.")
endif ()


add_definitions(-DFREEORION_BUILD_SERVER)

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()

set(freeoriond_LINK_LIBS
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
else ()
    list(APPEND freeoriond_LINK_LIBS ${Boost_LIBRARIES})
endif ()

add_executable(freeoriond
    ../network/ServerNetworking.cpp
    ../server/SaveLoad.cpp
    ../server/ServerApp.cpp
    ../server/ServerFSM.cpp
    ../server/dmain.cpp
    ../universe/UniverseServer.cpp
    ../combat/CombatSystem.cpp
    ../universe/PythonUniverseGenerator.cpp
    ../python/PythonEnumWrapper.cpp
    ../python/PythonLoggingWrapper.cpp
)

target_link_libraries(freeoriond
    ${freeoriond_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeoriond
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

