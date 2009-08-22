cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

set(CMAKE_VERBOSE_MAKEFILE false)

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
set(BUILD_MULTI_THREADED ON)
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
#
include_directories(${CMAKE_SOURCE_DIR} log4cpp)
#
#if (FREEORION_RELEASE)
#    add_definitions(-DFREEORION_RELEASE)
#endif ()
#
#set(THIS_LIB_SOURCES
#    combat/Combat.cpp
#    combat/CombatOrder.cpp
#    combat/OpenSteer/AsteroidBeltObstacle.cpp
#    combat/OpenSteer/CombatFighter.cpp
#    combat/OpenSteer/CombatObject.cpp
#    combat/OpenSteer/CombatShip.cpp
#    combat/OpenSteer/Missile.cpp
#    combat/OpenSteer/Obstacle.cpp
#    combat/OpenSteer/PathingEngine.cpp
#    combat/OpenSteer/SimpleVehicle.cpp
#    combat/OpenSteer/Vec3.cpp
#    combat/OpenSteer/Vec3Utilities.cpp
#    Empire/Empire.cpp
#    Empire/EmpireManager.cpp
#    Empire/ResourcePool.cpp
#    network/Message.cpp
#    network/MessageQueue.cpp
#    network/Networking.cpp
#    UI/StringTable.cpp
#    universe/Building.cpp
#    universe/Condition.cpp
#    universe/ConditionParser1.cpp
#    universe/ConditionParser2.cpp
#    universe/ConditionParser.cpp
#    universe/Effect.cpp
#    universe/EffectParser.cpp
#    universe/Enums.cpp
#    universe/Fleet.cpp
#    universe/Meter.cpp
#    universe/ParserUtil.cpp
#    universe/Planet.cpp
#    universe/PopCenter.cpp
#    universe/Predicates.cpp
#    universe/ResourceCenter.cpp
#    universe/Ship.cpp
#    universe/ShipDesign.cpp
#    universe/Special.cpp
#    universe/System.cpp
#    universe/Tech.cpp
#    universe/TopLevelParsers.cpp
#    universe/UniverseObject.cpp
#    universe/ValueRef.cpp
#    universe/ValueRefParser.cpp
#    util/DataTable.cpp
#    util/GZStream.cpp
#    util/MultiplayerCommon.cpp
#    util/OptionsDB.cpp
#    util/Order.cpp
#    util/OrderSet.cpp
#    util/Process.cpp
#    util/Random.cpp
#    util/Serialize.cpp
#    util/SitRepEntry.cpp
#    util/VarText.cpp
#    util/binreloc.c
#    util/Directories.cpp
#    util/XMLDoc.cpp
#    log4cpp/src/Appender.cpp
#    log4cpp/src/AppenderSkeleton.cpp
#    log4cpp/src/BasicLayout.cpp
#    log4cpp/src/Category.cpp
#    log4cpp/src/CategoryStream.cpp
#    log4cpp/src/Configurator.cpp
#    log4cpp/src/FactoryParams.cpp
#    log4cpp/src/FileAppender.cpp
#    log4cpp/src/HierarchyMaintainer.cpp
#    log4cpp/src/LayoutAppender.cpp
#    log4cpp/src/Localtime.cpp
#    log4cpp/src/LoggingEvent.cpp
#    log4cpp/src/NDC.cpp
#    log4cpp/src/PatternLayout.cpp
#    log4cpp/src/Priority.cpp
#    log4cpp/src/StringUtil.cpp
#    log4cpp/src/TimeStamp.cpp
#    ${CMAKE_BINARY_DIR}/util/Version.cpp
#)
#
#if (WIN32)
#    list(APPEND THIS_LIB_SOURCES log4cpp/src/MSThreads.cpp)
#else ()
#    list(APPEND THIS_LIB_SOURCES log4cpp/src/PThreads.cpp)
#endif ()
#
#set(BUILD_STATIC_TMP ${BUILD_STATIC})
#set(BUILD_SHARED_TMP ${BUILD_SHARED})
#set(BUILD_STATIC ON)
#set(BUILD_SHARED OFF)
#set(THIS_LIB_LINK_LIBS ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
#library_all_variants(freeorion_lib)
#set(BUILD_STATIC ${BUILD_STATIC_TMP})
#set(BUILD_SHARED ${BUILD_SHARED_TMP})


# This macro is an internal utility macro that builds a particular variant of
# an executable.
#
#   executable_variant(exename 
#                      feature1 feature2 ...)
#
# where exename is the name of the executable (e.g., "GiGiSDL") and feature1,
# feature2, ... are the features that will be used in this variant.
#
# This macro will define a new executable target based on exename and the
# specific variant name, which depends on the utility target exename. The
# compilation and linking flags for this executable are defined by
# THIS_EXE_COMPILE_FLAGS, THIS_EXE_LINK_FLAGS, THIS_EXE_LINK_LIBS, and all of
# the compile and linking flags implied by the features provided.
#
# If any of the features listed conflict with this executable, no new targets
# will be built.
macro (executable_variant EXENAME)
  set(THIS_VARIANT_COMPILE_FLAGS "${THIS_EXE_COMPILE_FLAGS}")
  set(THIS_VARIANT_LINK_FLAGS "${THIS_EXE_LINK_FLAGS}")
  set(THIS_VARIANT_LINK_LIBS ${THIS_EXE_LINK_LIBS})

  # Determine if it is okay to build this variant
  set(THIS_VARIANT_OKAY TRUE)
  foreach (ARG ${ARGN})
    # If the user specified that we should not build any variants of
    # this kind, don't. For example, if the BUILD_SHARED option is
    # off, don't build shared executables.
    if (NOT BUILD_${ARG})
      set(THIS_VARIANT_OKAY FALSE)
    endif (NOT BUILD_${ARG})

    # Accumulate compile and link flags
    set(THIS_VARIANT_COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS} ${THIS_EXE_${ARG}_COMPILE_FLAGS} ${${ARG}_COMPILE_FLAGS}")
    set(THIS_VARIANT_LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS} ${THIS_EXE_${ARG}_LINK_FLAGS} ${${ARG}_LINK_FLAGS}")
    set(THIS_VARIANT_LINK_LIBS ${THIS_VARIANT_LINK_LIBS} ${THIS_EXE_${ARG}_LINK_LIBS} ${${ARG}_LINK_LIBS})
  endforeach (ARG ${ARGN})

  # Handle feature interactions
  feature_interactions(THIS_VARIANT ${ARGN})

  if (THIS_VARIANT_OKAY)
    if (IS_STATIC)
      set(VARIANT_EXENAME ${EXENAME}_static)
    else ()
      set(VARIANT_EXENAME ${EXENAME})
    endif ()

    # We handle static vs. dynamic executables differently
    list_contains(THIS_EXE_IS_STATIC STATIC ${ARGN})
    if (THIS_EXE_IS_STATIC)
      # Add the executable itself
      add_executable(${VARIANT_EXENAME} ${THIS_EXE_SOURCES})

      # Set properties on this executable
      set_target_properties(${VARIANT_EXENAME}
        PROPERTIES
        OUTPUT_NAME "${EXENAME}"
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LINK_SEARCH_END_STATIC true
        LABELS "${PROJECT_NAME}"
        )
    else (THIS_EXE_IS_STATIC)
      add_executable(${VARIANT_EXENAME} ${THIS_EXE_SOURCES})
      # Set properties on this executable
      set_target_properties(${VARIANT_EXENAME}
        PROPERTIES
        OUTPUT_NAME ${EXENAME}
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
        CLEAN_DIRECT_OUTPUT 1
        COMPILE_FLAGS "${THIS_VARIANT_COMPILE_FLAGS}"
        LINK_FLAGS "${THIS_VARIANT_LINK_FLAGS}"
        LABELS "${PROJECT_NAME}"
        )
    endif (THIS_EXE_IS_STATIC)

    # Link against whatever libraries this executable depends on
    target_link_libraries(${VARIANT_EXENAME} ${THIS_VARIANT_LINK_LIBS})
    foreach (dependency ${THIS_EXE_DEPENDS})
      target_link_libraries(${VARIANT_EXENAME} "${dependency}")
    endforeach (dependency)
  endif ()
endmacro ()

macro (executable_all_variants EXENAME)
    executable_variant(${EXENAME} STATIC DEBUG MULTI_THREADED)
    executable_variant(${EXENAME} STATIC DEBUG SINGLE_THREADED)
    executable_variant(${EXENAME} STATIC RELEASE MULTI_THREADED)
    executable_variant(${EXENAME} STATIC RELEASE SINGLE_THREADED)
    executable_variant(${EXENAME} SHARED DEBUG MULTI_THREADED)
    executable_variant(${EXENAME} SHARED DEBUG SINGLE_THREADED)
    executable_variant(${EXENAME} SHARED RELEASE MULTI_THREADED)
    executable_variant(${EXENAME} SHARED RELEASE SINGLE_THREADED)
endmacro ()

########################################
# Recurse Into Sources                 #
########################################
add_subdirectory(server)
add_subdirectory(client/AI)
add_subdirectory(client/human)
