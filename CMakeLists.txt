cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

set(CMAKE_VERBOSE_MAKEFILE true)

########################################
# Configuration                        #
########################################
set(FREEORION_VERSION 0.3.13)
set(FREEORION_RELEASE false)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
include(Config)
include(Util)

set(MINIMUM_BOOST_VERSION 1.36.0)
set(MINIMUM_BOOST_COMPONENTS date_time filesystem serialization signals system thread)

set(BOOST_SIGNALS_NAMESPACE "signals" CACHE STRING
    "Specifies alternate namespace used for boost::signals (only needed if you changed it using the BOOST_SIGNALS_NAMESPACE define when you built boost).")

option(BUILD_DEBUG "Controls generation of debug code; OFF implies release code." OFF)

if (BUILD_DEBUG)
    set(BUILD_RELEASE OFF)
else ()
    set(BUILD_RELEASE ON)
endif ()

set(BUILD_STATIC OFF)
set(BUILD_SHARED ON)
set(BUILD_SINGLE_THREADED OFF)

if (WIN32)
    add_definitions(-DFREEORION_WIN32)
    if (MSVC)
        add_definitions(/wd4146 /wd4099 /wd4251 /wd4800 /wd4267 /wd4275
                        /wd4244 /wd4101 /wd4258 /wd4351 /wd4996)
    endif ()
elseif (APPLE)
    add_definitions(-DFREEORION_MACOSX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(-DFREEORION_LINUX -DENABLE_BINRELOC -Wall -Wno-parentheses -Wno-deprecated)
else ()
    message(FATAL_ERROR "Unknown platform type! Your setup is not a supported platform for FreeOrion.")
endif ()


########################################
# Dependencies                         #
########################################
set(Boost_USE_STATIC_LIBS ${BUILD_STATIC})
set(Boost_USE_MULTITHREADED ${BUILD_MULTI_THREADED})
find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS ${MINIMUM_BOOST_COMPONENTS})
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost libraries not found.")
endif ()

if (NOT BOOST_SIGNALS_NAMESPACE STREQUAL "signals")
    add_definitions(
        -DBOOST_SIGNALS_NAMESPACE=${BOOST_SIGNALS_NAMESPACE}
        -Dsignals=${BOOST_SIGNALS_NAMESPACE}
    )
endif ()

find_package(ZLIB)
if (ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "ZLib library not found.")
endif ()

find_package(Subversion)
set(svn_revision_number ???)
if (Subversion_FOUND)
   execute_process(
       COMMAND ${Subversion_SVN_EXECUTABLE} info
       WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}
       RESULT_VARIABLE result
       OUTPUT_VARIABLE output
   )
   if (NOT result)
       string(REGEX REPLACE ".*Last Changed Rev: ([0-9]*).*" "\\1" svn_revision_number ${output})
   endif ()
endif ()
configure_file(
    ${CMAKE_SOURCE_DIR}/cmake/Version.cpp.in
    ${CMAKE_BINARY_DIR}/util/Version.cpp
    @ONLY
)


########################################
# Build common code library            #
########################################
# TODO: Figure out why this kills the cmake configuration step for CMake 2.6.4
# Linux on Suse 11.1, and how it might be fixed/contained.
#project(freeorion_lib)

include_directories(${CMAKE_SOURCE_DIR} log4cpp)

if (FREEORION_RELEASE)
    add_definitions(-DFREEORION_RELEASE)
endif ()

set(common_sources
    combat/Combat.cpp
    combat/CombatOrder.cpp
    combat/OpenSteer/AsteroidBeltObstacle.cpp
    combat/OpenSteer/CombatFighter.cpp
    combat/OpenSteer/CombatObject.cpp
    combat/OpenSteer/CombatShip.cpp
    combat/OpenSteer/Missile.cpp
    combat/OpenSteer/Obstacle.cpp
    combat/OpenSteer/PathingEngine.cpp
    combat/OpenSteer/SimpleVehicle.cpp
    combat/OpenSteer/Vec3.cpp
    combat/OpenSteer/Vec3Utilities.cpp
    Empire/Empire.cpp
    Empire/EmpireManager.cpp
    Empire/ResourcePool.cpp
    network/Message.cpp
    network/MessageQueue.cpp
    network/Networking.cpp
    UI/StringTable.cpp
    universe/Building.cpp
    universe/Condition.cpp
    universe/ConditionParser1.cpp
    universe/ConditionParser2.cpp
    universe/ConditionParser.cpp
    universe/Effect.cpp
    universe/EffectParser.cpp
    universe/Enums.cpp
    universe/Fleet.cpp
    universe/Meter.cpp
    universe/ParserUtil.cpp
    universe/Planet.cpp
    universe/PopCenter.cpp
    universe/Predicates.cpp
    universe/ResourceCenter.cpp
    universe/Ship.cpp
    universe/ShipDesign.cpp
    universe/Special.cpp
    universe/System.cpp
    universe/Tech.cpp
    universe/TopLevelParsers.cpp
    universe/UniverseObject.cpp
    universe/ValueRef.cpp
    universe/ValueRefParser.cpp
    util/DataTable.cpp
    util/GZStream.cpp
    util/MultiplayerCommon.cpp
    util/OptionsDB.cpp
    util/Order.cpp
    util/OrderSet.cpp
    util/Process.cpp
    util/Random.cpp
    util/Serialize.cpp
    util/SitRepEntry.cpp
    util/VarText.cpp
    util/binreloc.c
    util/Directories.cpp
    util/XMLDoc.cpp
    log4cpp/src/Appender.cpp
    log4cpp/src/AppenderSkeleton.cpp
    log4cpp/src/BasicLayout.cpp
    log4cpp/src/Category.cpp
    log4cpp/src/CategoryStream.cpp
    log4cpp/src/Configurator.cpp
    log4cpp/src/FactoryParams.cpp
    log4cpp/src/FileAppender.cpp
    log4cpp/src/HierarchyMaintainer.cpp
    log4cpp/src/LayoutAppender.cpp
    log4cpp/src/Localtime.cpp
    log4cpp/src/LoggingEvent.cpp
    log4cpp/src/NDC.cpp
    log4cpp/src/PatternLayout.cpp
    log4cpp/src/Priority.cpp
    log4cpp/src/StringUtil.cpp
    log4cpp/src/TimeStamp.cpp
    ${CMAKE_BINARY_DIR}/util/Version.cpp
)

if (WIN32)
    list(APPEND common_sources log4cpp/src/MSThreads.cpp)
else ()
    list(APPEND common_sources log4cpp/src/PThreads.cpp)
endif ()

add_library(freeorion_lib STATIC ${common_sources})

set_target_properties(
    freeorion_lib
    PROPERTIES
    OUTPUT_NAME freeorion
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CLEAN_DIRECT_OUTPUT 1
    LABELS "${PROJECT_NAME}"
)

target_link_libraries(freeorion_lib ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})

########################################
# Recurse Into Sources                 #
########################################
add_subdirectory(server)
add_subdirectory(client/AI)
add_subdirectory(client/human)
