cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

if(POLICY CMP0044)
    cmake_policy(SET CMP0044 OLD)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "Setting build type to 'Release' as none was specified.")
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
ENDIF(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build type CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}")


##
## Global project configuration
##

project(FreeOrion)

include(GNUInstallDirs)
include(UseCodeCoverage)

set(FreeOrion_VERSION 0.4.6+)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_TESTING "Build the testing tree." OFF)
option(BUILD_PACKAGING "Build the installer/binary package" OFF)

if (BUILD_TESTING)
    message( STATUS "Building Tests")
    enable_testing()
    enable_coverage()

    if (NOT TARGET unittest)
        add_custom_target(unittest
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
            COMMENT "Run tests for ${CMAKE_PROJECT_NAME}"
        )
    endif ()
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(WIN32)
    set(CMAKE_INSTALL_BINDIR ".")
    set(FreeOrion_INSTALL_LIBDIR ".")
else()
    set(FreeOrion_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/freeorion")
endif()

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${FreeOrion_INSTALL_LIBDIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

add_compile_options(
    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wall-316
    $<$<CXX_COMPILER_ID:gnu>:-Wall>
    # http://clang.llvm.org/docs/DiagnosticsReference.html#wall
    $<$<CXX_COMPILER_ID:clang>:-Wall>
    # https://msdn.microsoft.com/en-us/library/thxezb7y.aspx#Anchor_1
    $<$<CXX_COMPILER_ID:msvc>:/W4>
)

if (WIN32)
    add_definitions(
        -D_CRT_SECURE_NO_DEPRECATE
        -D_SCL_SECURE_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_DEPRECATE
        -DFREEORION_WIN32
        -D_WIN32_WINNT=0x0501
    )
    if (MSVC)
        set (CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/../")
        set (BOOST_ROOT "${CMAKE_BINARY_DIR}/../Boost")
        set (CMAKE_STATIC_LIBRARY_PREFIX "lib")
        set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
        set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    endif ()
elseif (APPLE)
    add_definitions(-DFREEORION_MACOSX)
elseif (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_definitions(
        -DFREEORION_LINUX
        -DENABLE_BINRELOC
    )
elseif (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    add_definitions(
        -DFREEORION_FREEBSD
        -DENABLE_BINRELOC
        -DBOOST_DATE_TIME_NO_LOCALE
    )
else ()
    message(FATAL_ERROR "Unknown platform type! Your setup is not a supported platform for FreeOrion.")
endif ()

# Need to know if the boost libraries are linked statically or dynamically
# If dynamically, add some macro definitions to prevent potential link errors
option(FO_LINK_STATIC_BOOST_LIBS "Link static boost libraries." OFF)

if (NOT FO_LINK_STATIC_BOOST_LIBS)
    add_definitions(
        -DBOOST_ALL_NO_LINK
        -DBOOST_ALL_DYN_LINK
        -DBOOST_LOG_DYN_LINK
    )
endif ()


##
## Collect project dependencies.
##

set(MINIMUM_PYTHON_VERSION 2.7)
set(MINIMUM_BOOST_VERSION 1.56.0)

find_package(Threads)
find_package(PythonInterp ${MINIMUM_PYTHON_VERSION} REQUIRED)
find_package(PythonLibs ${MINIMUM_PYTHON_VERSION} REQUIRED)
find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        chrono
        date_time
        filesystem
        iostreams
        locale
        log
        regex
        serialization
        python
        signals
        system
        thread
    REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)

if (NOT OPENGL_GLU_FOUND)
    message(FATAL_ERROR "OpenGL GLU library not found.")
endif ()

find_package(SDL2 REQUIRED)
find_package(OpenAL REQUIRED)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)


##
## Define main project targets.
##

# To run the version generation every compile we need to deferr the
# execution to a separate target and the existing python command
add_custom_target(freeorionversion
    COMMAND
    "${PYTHON_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
    "${CMAKE_SOURCE_DIR}"
    "CMake"
)

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/util/Version.cpp
    PROPERTIES
    GENERATED TRUE
)

if (BUILD_TESTING)
    add_subdirectory(test)
endif ()

set(BUILD_DEVEL_PACKAGE OFF CACHE INTERNAL "Disables installation of GiGi development files." FORCE)
set(_ORIG_CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_LIBDIR "${FreeOrion_INSTALL_LIBDIR}")
add_subdirectory(GG)
set(CMAKE_INSTALL_LIBDIR ${_ORIG_CMAKE_INSTALL_LIBDIR})
unset(_ORIG_CMAKE_INSTALL_LIBDIR)

if(${Boost_VERSION} LESS "105600")
    # Starting with boost 1.56 boost::spirit uses boost::phoenix v3 exclusively.
    # Before that version there spirit provides an translation layer, which can
    # be enabled by defining BOOST_SPIRIT_USE_PHOENIX_V3.
    add_definitions(
        -DBOOST_SPIRIT_USE_PHOENIX_V3
    )
endif()

if(${Boost_VERSION} EQUAL "106100" OR ${Boost_VERSION} GREATER "106100")
    # with boost 1.61 some boost::optional internals were changed. However
    # boost::spirit relies on some API the old implementation provided.  This
    # define enables the usage of the old boost::optional implementation.
    # boost upstream tracks this bug as #12349
    add_definitions(
        -DBOOST_OPTIONAL_CONFIG_USE_OLD_IMPLEMENTATION_OF_OPTIONAL
    )
endif()

link_directories(
    ${Boost_LIBRARY_DIRS}
)


add_library(freeorioncommon "")

target_compile_options(freeorioncommon
    PRIVATE
        $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
        $<$<CXX_COMPILER_ID:clang>:-ftemplate-depth=512>
)

target_compile_definitions(freeorioncommon
    PRIVATE
    -DFREEORION_BUILD_COMMON
)

target_include_directories(freeorioncommon SYSTEM
    PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/GG
        ${ZLIB_INCLUDE_DIR}
)

target_link_libraries(freeorioncommon
    ${Boost_CHRONO_LIBRARY}
    ${Boost_DATE_TIME_LIBRARY}
    ${Boost_FILESYSTEM_LIBRARY}
    ${Boost_IOSTREAMS_LIBRARY}
    ${Boost_LOG_LIBRARY}
    ${Boost_REGEX_LIBRARY}
    ${Boost_SERIALIZATION_LIBRARY}
    ${Boost_SIGNALS_LIBRARY}
    ${Boost_SYSTEM_LIBRARY}
    ${Boost_THREAD_LIBRARY}
    ${ZLIB_LIBRARIES}
)

add_dependencies(freeorioncommon freeorionversion)


add_library(freeorionparseobj OBJECT "")

target_include_directories(freeorionparseobj SYSTEM
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/GG
)

set_property(TARGET freeorionparseobj
    PROPERTY
    POSITION_INDEPENDENT_CODE ON
)

target_compile_options(
    freeorionparseobj PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
    $<$<AND:$<NOT:$<BOOL:${BUILD_TESTING}>>,$<CXX_COMPILER_ID:gnu>>:-O3>
    $<$<CXX_COMPILER_ID:clang>:-ftemplate-depth=512>
)

target_compile_definitions(freeorionparseobj
    PRIVATE
        -DNDEBUG
        -DFREEORION_BUILD_PARSE
)


add_library(freeorionparse $<TARGET_OBJECTS:freeorionparseobj>)

target_compile_options(
    freeorionparse PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
)


add_executable(freeoriond "")

target_compile_options(freeoriond
    PRIVATE
        $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
)

target_compile_definitions(freeoriond
    PRIVATE
        -DFREEORION_BUILD_SERVER
)

target_include_directories(freeoriond SYSTEM
    PRIVATE
        ${PROJECT_SOURCE_DIR}/GG
        ${PYTHON_INCLUDE_PATH}
)

target_link_libraries(freeoriond
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
    ${Boost_PYTHON_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)


add_executable(freeorionca "")

target_compile_options(
    freeorionca PRIVATE
    $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
)

target_compile_definitions(freeorionca
    PRIVATE
        -DFREEORION_BUILD_AI
)

target_include_directories(freeorionca SYSTEM
    PRIVATE
        ${Boost_INCLUDE_DIRS}
        ${PYTHON_INCLUDE_PATH}
        ${PROJECT_SOURCE_DIR}/GG
)

target_link_libraries(freeorionca
    freeorioncommon
    freeorionparse
    ${PYTHON_LIBRARIES}
    ${Boost_PYTHON_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)


add_executable(freeorion "")

target_compile_options(freeorion
    PRIVATE
        $<$<CXX_COMPILER_ID:gnu>:-fvisibility=hidden>
)

target_compile_definitions(freeorion
    PRIVATE
        -DFREEORION_BUILD_HUMAN
)

target_include_directories(freeorion SYSTEM
    PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${OPENAL_INCLUDE_DIR}
        ${SDL2_INCLUDE_DIR}
        ${OGG_INCLUDE_DIR}
        ${VORBIS_INCLUDE_DIR}
        ${FREETYPE_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/GG
)

target_link_libraries(freeorion
    freeorioncommon
    freeorionparse
    GiGi
    GiGiSDL
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${OPENAL_LIBRARY}
    ${SDL2_LIBRARY}
    ${OGG_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${Boost_LOCALE_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)


##
## Recurse into sources.
##

add_subdirectory(AI)
add_subdirectory(client)
add_subdirectory(combat)
add_subdirectory(Empire)
add_subdirectory(network)
add_subdirectory(parse)
add_subdirectory(python)
add_subdirectory(server)
add_subdirectory(UI)
add_subdirectory(universe)
add_subdirectory(util)
add_subdirectory(check)
add_subdirectory(doc)


##
## Install targets and package project.
##

install(
    DIRECTORY default/
    DESTINATION ${CMAKE_INSTALL_DATADIR}/freeorion/default
    COMPONENT COMPONENT_FREEORION_DATA
    PATTERN "*~" EXCLUDE
    PATTERN "*.pyc" EXCLUDE
)

install(
    TARGETS freeorioncommon
    LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

install(
    TARGETS freeorionparse
    LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

install(
    TARGETS freeoriond
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

install(
    TARGETS freeorionca
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

install(
    TARGETS freeorion
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_FREEORION
)

install(
    FILES
    ${CMAKE_SOURCE_DIR}/freeorion.desktop
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
    COMPONENT COMPONENT_FREEORION
)

foreach(SIZE 16 24 32 64 128 256)
    install(
        FILES
        ${CMAKE_SOURCE_DIR}/default/data/art/icons/FO_Icon_${SIZE}x${SIZE}.png
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${SIZE}x${SIZE}/apps/
        RENAME freeorion.png
        COMPONENT COMPONENT_FREEORION
    )
endforeach()

if (WIN32)
    install(
        FILES
        OpenAL32.dll
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
        libexpat.dll
        libogg.dll
        libpng13.dll
        libvorbis.dll
        libvorbisfile.dll
        python26.dll
        wrap_oal.dll
        z.dll
        zlib1.dll
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT COMPONENT_FREEORION_WIN32_RUNTIME_DEPS
    )
endif ()

set(CPACK_PACKAGE_VERSION ${FreeOrion_VERSION})

set(CPACK_PACKAGE_VENDOR "FreeOrion Community")
set(CPACK_PACKAGE_CONTACT http://freeorion.org/forum)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FreeOrion is a free, open source, turn-based space empire and galactic conquest (4X) computer game being designed and built by the FreeOrion project. FreeOrion is inspired by the tradition of the Master of Orion games, but is not a clone or remake of that series or any other game.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_HOME_DIRECTORY}/default/COPYING)
if (BUILD_PACKAGING)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
    if (WIN32)
        set(PACKAGE_FILE_SYSTEM_NAME win32)
    else ()
        set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME})
    endif ()
    set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
    set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${PACKAGE_FILE_SYSTEM_NAME}")
    string(TOLOWER ${CPACK_PACKAGE_FILE_NAME} CPACK_PACKAGE_FILE_NAME)
    set(CPACK_MONOLITHIC_INSTALL ON)
endif ()

# NSIS-specific settings
set(CPACK_NSIS_COMPRESSOR bzip2)
set(CPACK_NSIS_URL_INFO_ABOUT http://freeorion.org)
string(REPLACE "/" "\\\\" cmake_home_directory_with_native_windows_path_separators ${CMAKE_HOME_DIRECTORY})

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
elseif (WIN32)
    set(CPACK_GENERATOR "NSIS")
endif ()


########################################
# Source Packaging                     #
########################################

if (UNIX)
    set(CPACK_SOURCE_GENERATOR "TGZ")
elseif (WIN32)
    set(CPACK_SOURCE_GENERATOR "ZIP")
endif ()

set(CPACK_SOURCE_IGNORE_FILES
    "~$"
    "\\\\.asm$"
    "\\\\.bz2$"
    "/CMakeCache\\\\.txt$"
    "/CMakeFiles/"
    "/cmake_install\\\\.cmake$"
    "/CPackConfig.cmake$"
    "/_CPack_Packages/"
    "/CPackSourceConfig.cmake$"
    "/CTestTestfile\\\\.cmake$"
    "\\\\.dll$"
    "\\\\.exe$"
    "\\\\.exp$"
    "/freeorion$"
    "/freeorionca$"
    "/freeoriond$"
    "GG/GG/Config.h$"
    "\\\\.git/"
    "\\\\.gz$"
    "\\\\.lib$"
    "/Makefile$"
    "\\\\.pc$"
    "\\\\.pdb$"
    "\\\\.pyc$"
    "/Release/"
    "\\\\.so$"
    "\\\\.swp$"
)

set(CPACK_SOURCE_PACKAGE_FILE_NAME
    "${CMAKE_PROJECT_NAME}-v${FreeOrion_VERSION}-${FreeOrion_WC_REVISION}-source"
)

include(CPack)

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
