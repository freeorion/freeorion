cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorionca)

message("-- Configuring freeorionca")

list(APPEND MINIMUM_BOOST_COMPONENTS iostreams python)
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

add_definitions(-DFREEORION_BUILD_AI)

set(freeorionca_LINK_LIBS core parse log4cpp ${PYTHON_LIBRARIES})

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
else ()
    list(APPEND freeorionca_LINK_LIBS ${Boost_LIBRARIES})
endif ()

add_executable(freeorionca
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
    ../../util/AppInterface.cpp
    ../../util/VarText.cpp
)

target_link_libraries(freeorionca
    ${freeorionca_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeorionca
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

