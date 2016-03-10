cmake_minimum_required(VERSION 2.8.5)

find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS python log REQUIRED)
find_package(PythonLibs ${PYTHON_VERSION} REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_PATH}
)

add_definitions(-DFREEORION_BUILD_AI)

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()

set(freeorionca_LINK_LIBS
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
    ${Boost_LIBRARIES}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
endif ()

set (freeorionca_HEADER
    AIClientApp.h
    ../ClientApp.h
    ../ClientFSMEvents.h
)

set (freeorionca_SOURCE
    ../../AI/AIInterface.cpp
    AIClientApp.cpp
    camain.cpp
    ../ClientApp.cpp
    ../ClientFSMEvents.cpp
    ../../combat/CombatSystem.cpp
    ../../network/ClientNetworking.cpp
    ../../python/CommonFramework.cpp
    ../../python/AI/AIFramework.cpp
    ../../python/EmpireWrapper.cpp
    ../../python/EnumWrapper.cpp
    ../../python/LoggingWrapper.cpp
    ../../python/UniverseWrapper.cpp
    ../../python/AI/AIWrapper.cpp
)

add_executable(freeorionca
    ${freeorionca_HEADER}
    ${freeorionca_SOURCE}
)

target_link_libraries(freeorionca
    ${freeorionca_LINK_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(
    TARGETS freeorionca
    RUNTIME DESTINATION bin
    COMPONENT COMPONENT_FREEORION
)

