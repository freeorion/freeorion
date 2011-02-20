cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

set(CMAKE_VERBOSE_MAKEFILE false)

########################################
# Configuration                        #
########################################
project(FreeOrion)

set(FREEORION_VERSION 0.3.15)
set(FREEORION_RELEASE false)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
include(Config)
include(Util)

set(MINIMUM_BOOST_VERSION 1.44.0)
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
    add_definitions(-DFREEORION_LINUX -DENABLE_BINRELOC -Wall -Wno-parentheses)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated")
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

find_package(GiGi)
if (GIGI_FOUND)
    include_directories(${GIGI_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "GiGi library not found.")
endif ()

find_package(Subversion)
set(svn_revision_number ???)
set(tmpLANG $ENV{LANG})
set(ENV{LANG} "C")
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
set($ENV{LANG} ${tmpLANG})


########################################
# Build common code library            #
########################################
# TODO: Figure out why this kills the cmake configuration step for CMake 2.6.4
# Linux on Suse 11.1, and how it might be fixed/contained.
#project(freeorion_lib)
#
include_directories(${CMAKE_SOURCE_DIR} log4cpp)
include_directories(${CMAKE_SOURCE_DIR} UI/ogdf)
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

  string(TOUPPER COMPONENT_${PROJECT_NAME} EXE_COMPONENT)
  install(
    TARGETS ${VARIANT_EXENAME}
    RUNTIME DESTINATION . COMPONENT ${EXE_COMPONENT}
    LIBRARY DESTINATION . COMPONENT ${EXE_COMPONENT}
    ARCHIVE DESTINATION . COMPONENT ${EXE_COMPONENT}
  )
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

########################################
# Win32 SDK-only steps                 #
########################################
if (WIN32)
    # Add icon resource file to freeorion.exe
    if (NOT EXISTS ${CMAKE_BINARY_DIR}/win32_resources.rc)
        file(WRITE ${CMAKE_BINARY_DIR}/win32_resources.rc "IDI_ICON ICON \"${CMAKE_SOURCE_DIR}/client/human/HumanClient.ico\"")
    endif ()

    get_target_property(freeorion_loc freeorion LOCATION)
    get_target_property(freeoriond_loc freeoriond LOCATION)
    get_target_property(freeorionca_loc freeorionca LOCATION)
    add_custom_target(
        COPY_BINARIES
	ALL
        ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/../bin/GiGi.dll ${CMAKE_HOME_DIRECTORY}/../FreeOrion/GiGi.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/../bin/GiGiOgre.dll ${CMAKE_HOME_DIRECTORY}/../FreeOrion/GiGiOgre.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_HOME_DIRECTORY}/../bin/GiGiOgrePlugin_OIS.dll ${CMAKE_HOME_DIRECTORY}/../FreeOrion/GiGiOgrePlugin_OIS.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${freeorion_loc} ${CMAKE_HOME_DIRECTORY}/../FreeOrion/freeorion.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${freeorionca_loc} ${CMAKE_HOME_DIRECTORY}/../FreeOrion/freeorionca.exe
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${freeoriond_loc} ${CMAKE_HOME_DIRECTORY}/../FreeOrion/freeoriond.exe
	DEPENDS freeorion freeoriond freeorionca
    )
endif ()

########################################
# Packaging                            #
########################################
set(BUILD_PACKAGING
    false
    CACHE BOOL
    "Whether to build installer/binary package. (This is the executable installer on Win32, and an RPM/DEB/tarball on Linux."
)

if (WIN32)
    set(BUILD_SDK
        false
        CACHE BOOL
        "Whether to build the Win32 SDK installer."
    )
endif ()

if (BUILD_SDK)
    install(
        DIRECTORY ../FreeOrion
        DESTINATION .
        COMPONENT COMPONENT_FREEORION_SOURCE
        PATTERN "*~" EXCLUDE
    )

    install(
        DIRECTORY ../Boost
        DESTINATION .
        COMPONENT COMPONENT_FREEORION_BUILD_FILES_BOOST
        PATTERN "*~" EXCLUDE
    )

    install(
        DIRECTORY ../include
        DESTINATION .
        COMPONENT COMPONENT_FREEORION_BUILD_FILES_INCLUDE
        PATTERN "*~" EXCLUDE
    )

    install(
        DIRECTORY ../lib
        DESTINATION .
        COMPONENT COMPONENT_FREEORION_BUILD_FILES_LIB
        PATTERN "*~" EXCLUDE
    )

    install(
        FILES
        ../build/GG/CMakeCache.txt
        DESTINATION build/GG
        COMPONENT COMPONENT_FREEORION_BUILD_FILES_GIGI
    )

    install(
        FILES
        ../build/CMakeCache.txt
        DESTINATION build
        COMPONENT COMPONENT_FREEORION_BUILD_FILES_FREEORION
    )

    install(
        FILES
        ../README.txt
        ../vcredist_x86.exe
        ../cmake-2.8.0-win32-x86.exe
        DESTINATION .
        COMPONENT COMPONENT_FREEORION_MISC_SDK_FILES
    )
else ()
    install(
        DIRECTORY default
        DESTINATION default
        COMPONENT COMPONENT_FREEORION_DATA
        PATTERN ".svn" EXCLUDE
        PATTERN "*~" EXCLUDE
    )

    if (WIN32)
        install(
            FILES
            GiGi.dll
            GiGiOgre.dll
            GiGiOgrePlugin_OIS.dll
            OIS.dll
            OgreMain.dll
            OpenAL32.dll
            Plugin_OctreeSceneManager.dll
            Plugin_ParticleFX.dll
            RenderSystem_GL.dll
            alut.dll
            boost_date_time-vc90-mt-1_44.dll
            boost_filesystem-vc90-mt-1_44.dll
            boost_iostreams-vc90-mt-1_44.dll
            boost_python-vc90-mt-1_44.dll
            boost_regex-vc90-mt-1_44.dll
            boost_serialization-vc90-mt-1_44.dll
            boost_signals-vc90-mt-1_44.dll
            boost_system-vc90-mt-1_44.dll
            boost_thread-vc90-mt-1_44.dll
            glew32.dll
            jpeg.dll
            libexpat.dll
            libogg.dll
            libpng13.dll
            libvorbis.dll
            libvorbisfile.dll
            python26.dll
            wrap_oal.dll
            z.dll
            zlib1.dll
            OISInput.cfg
            ogre_plugins.cfg
            DESTINATION .
            COMPONENT COMPONENT_FREEORION_WIN32_RUNTIME_DEPS
        )
    endif ()
endif ()

if (BUILD_SDK)
    set(CPACK_PACKAGE_VERSION "1.0")
else ()
    set(CPACK_PACKAGE_VERSION ${FREEORION_VERSION})
endif ()

set(CPACK_PACKAGE_VENDOR "FreeOrion Community")
set(CPACK_PACKAGE_CONTACT http://freeorion.org/forum)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FreeOrion is a free, open source, turn-based space empire and galactic conquest (4X) computer game being designed and built by the FreeOrion project. FreeOrion is inspired by the tradition of the Master of Orion games, but is not a clone or remake of that series or any other game.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/default/COPYING)
if (BUILD_SDK)
    set(CPACK_RESOURCE_FILE_README ${CMAKE_HOME_DIRECTORY}/../README.txt)
endif ()
if (BUILD_PACKAGING)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
    if (WIN32)
        set(PACKAGE_FILE_SYSTEM_NAME win32)
    else ()
        set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME})
    endif ()
    if (BUILD_SDK)
        set(CPACK_PACKAGE_NAME "FreeOrion SDK")
        set(CPACK_PACKAGE_FILE_NAME "FreeOrion_Win32_SDK")
        string(CONFIGURE "FreeOrion_SDK_${CPACK_PACKAGE_VERSION}" dir)
        set(CPACK_PACKAGE_INSTALL_DIRECTORY ${dir})
    else ()
        set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
        set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${PACKAGE_FILE_SYSTEM_NAME}")
        string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
    endif ()
    set(CPACK_MONOLITHIC_INSTALL ON)
endif ()

# NSIS-specific settings
set(CPACK_NSIS_COMPRESSOR bzip2)
set(CPACK_NSIS_URL_INFO_ABOUT http://freeorion.org)
string(REPLACE "/" "\\\\" cmake_home_directory_with_native_windows_path_separators ${CMAKE_HOME_DIRECTORY})

if (BUILD_SDK)
    string(REGEX REPLACE "/FreeOrion$" "" sdk_root ${CMAKE_HOME_DIRECTORY})
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        !include \\\"WordFunc.nsh\\\"

        StrCpy $7 $INSTDIR
        \\\${WordReplace} $7 \\\"\\\\\\\" \\\"/\\\" \\\"+\\\" $8

	Rename $INSTDIR\\\\build\\\\GG\\\\CMakeCache.txt $INSTDIR\\\\build\\\\GG\\\\CMakeCache.txt.in
        FileOpen $0 $INSTDIR\\\\build\\\\GG\\\\CMakeCache.txt.in r
        FileOpen $5 $INSTDIR\\\\build\\\\GG\\\\CMakeCache.txt w
        ClearErrors
        loop1:
            FileRead $0 $1
            IfErrors done1
            \\\${WordReplace} $1 \\\"${sdk_root}\\\" $8 \\\"+\\\" $2
            FileWrite $5 $2
            Goto loop1
        done1:
        FileClose $0
        FileClose $5
        Delete $INSTDIR\\\\build\\\\GG\\\\CMakeCache.txt.in

	Rename $INSTDIR\\\\build\\\\CMakeCache.txt $INSTDIR\\\\build\\\\CMakeCache.txt.in
        FileOpen $0 $INSTDIR\\\\build\\\\CMakeCache.txt.in r
        FileOpen $5 $INSTDIR\\\\build\\\\CMakeCache.txt w
        ClearErrors
        loop2:
            FileRead $0 $1
            IfErrors done2
            \\\${WordReplace} $1 \\\"${sdk_root}\\\" $8 \\\"+\\\" $2
            FileWrite $5 $2
            Goto loop2
        done2:
        FileClose $0
        FileClose $5
        Delete $INSTDIR\\\\build\\\\CMakeCache.txt.in

        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\FreeOrion SDK.lnk\\\" \\\"$INSTDIR\\\"
        WriteIniStr \\\"$INSTDIR\\\\freeorion.url\\\" \\\"InternetShortcut\\\" \\\"URL\\\" \\\"http://freeorion.org\\\"
        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\README.lnk\\\" \\\"$INSTDIR\\\\README.txt\\\"
        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Website.lnk\\\" \\\"$INSTDIR\\\\freeorion.url\\\"
        File \\\"${cmake_home_directory_with_native_windows_path_separators}\\\\..\\\\cmake-2.8.0-win32-x86.exe\\\"
        ExecWait \\\"$INSTDIR\\\\cmake-2.8.0-win32-x86.exe\\\""
    )
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\FreeOrion SDK.lnk\\\"
        Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\README.lnk\\\"
        Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\Website.lnk\\\"
        RMDir /r /REBOOTOK $INSTDIR"
    )
else ()
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
        File \\\"${cmake_home_directory_with_native_windows_path_separators}\\\\..\\\\vcredist_x86.exe\\\"
        ExecWait \\\"$INSTDIR\\\\vcredist_x86.exe /q\\\"
        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\FreeOrion.lnk\\\" \\\"$INSTDIR\\\\freeorion.exe\\\" \\\"--fullscreen 1\\\"
        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\FreeOrion windowed.lnk\\\" \\\"$INSTDIR\\\\freeorion.exe\\\"
        CreateShortCut \\\"$DESKTOP\\\\FreeOrion.lnk\\\" \\\"$INSTDIR\\\\freeorion.exe\\\" \\\"--fullscreen 1\\\"
        WriteIniStr \\\"$INSTDIR\\\\freeorion.url\\\" \\\"InternetShortcut\\\" \\\"URL\\\" \\\"http://freeorion.org\\\"
        CreateShortCut \\\"$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\Website.lnk\\\" \\\"$INSTDIR\\\\freeorion.url\\\""
    )
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
        Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\Website.lnk\\\"
        Delete \\\"$DESKTOP\\\\FreeOrion.lnk\\\"
        Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\FreeOrion.lnk\\\"
        Delete \\\"$SMPROGRAMS\\\\$START_MENU\\\\FreeOrion windowed.lnk\\\""
    )
endif ()

# RPM-specific settings
set(CPACK_RPM_PACKAGE_LICENSE GPL)
set(CPACK_RPM_PACKAGE_REQUIRES)
# TODO

# Deb-specific settings
set(CPACK_DEBIAN_PACKAGE_SECTION games)
# TODO

if (UNIX)
    set(CPACK_GENERATOR "TGZ;TBZ2")
    find_program(RPMBUILD rpmbuild)
    if (RPMBUILD)
        list(APPEND CPACK_GENERATOR RPM)
    endif ()
    set(RPMBUILD ${RPMBUILD} CACHE INTERNAL "")
    find_program(DPKG dpkg)
    if (DPKG)
        list(APPEND CPACK_GENERATOR DEB)
    endif ()
    set(DPKG ${DPKG} CACHE INTERNAL "")
    set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
elseif (WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif ()

include(CPack)

if (BUILD_SDK)
    cpack_add_component(
        COMPONENT_FREEORION_SOURCE
        DISPLAY_NAME FreeOrion Source Code
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )
    cpack_add_component(
        COMPONENT_FREEORION_BUILD_FILES_BOOST
        DISPLAY_NAME FreeOrion Build Files -- Boost Directory
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORION_BUILD_FILES_INCLUDE
        DISPLAY_NAME FreeOrion Build Files -- Include Directory
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORION_BUILD_FILES_LIB
        DISPLAY_NAME FreeOrion Build Files -- Lib Directory
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORION_BUILD_FILES_GIGI
        DISPLAY_NAME FreeOrion Build Files -- Gigi CMake Cache
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORION_BUILD_FILES_FREEORION
        DISPLAY_NAME FreeOrion Build Files -- FreeOrion CMake Cache
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORION_MISC_SDK_FILES
        DISPLAY_NAME FreeOrion Misc SDK Files
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )
else ()
    if (WIN32)
        cpack_add_component(
            COMPONENT_FREEORION_WIN32_RUNTIME_DEPS
            DISPLAY_NAME FreeOrion Win32 Runtime Dependencies
            DESCRIPTION ""
            HIDDEN
            REQUIRED
        )
    endif ()

    cpack_add_component(
        COMPONENT_FREEORION_DATA
        DISPLAY_NAME FreeOrion Data
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORION
        DISPLAY_NAME FreeOrion Client
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORIOND
        DISPLAY_NAME FreeOrion Server
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )

    cpack_add_component(
        COMPONENT_FREEORIONCA
        DISPLAY_NAME FreeOrion AI
        DESCRIPTION ""
        HIDDEN
        REQUIRED
    )
endif ()
