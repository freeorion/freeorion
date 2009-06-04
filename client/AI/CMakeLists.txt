cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorionca)

message("-- Configuring freeorionca")

set(sources
    ../../client/ClientApp.cpp
    ../../client/ClientFSMEvents.cpp
    ../../client/AI/AIClientApp.cpp
    ../../client/AI/camain.cpp
    ../../network/ClientNetworking.cpp
    ../../universe/Universe.cpp
    ../../util/AppInterface.cpp
    ../../AI/AIInterface.cpp
    ../../AI/PythonAI.cpp
    ../../python/PythonEnumWrapper.cpp
    ../../python/PythonUniverseWrapper.cpp
    ../../python/PythonEmpireWrapper.cpp
    ../../python/PythonLoggingWrapper.cpp
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

add_executable(freeorionca ${sources})

set_target_properties(
    freeorionca
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CLEAN_DIRECT_OUTPUT 1
    LABELS "${PROJECT_NAME}"
)

target_link_libraries(freeorionca freeorion_lib ${Boost_LIBRARIES} ${PYTHON_LIBRARIES})
