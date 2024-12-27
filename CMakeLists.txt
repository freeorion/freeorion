if(WIN32)
    # file(GET_RUNTIME_DEPENDENCIES and install(... ALL_COMPONENTS)
    set(REQUIRED_CMAKE_VERSION 3.21)
else()
    # Python3_FIND_STRATEGY
    set(REQUIRED_CMAKE_VERSION 3.15)
endif()

cmake_minimum_required(VERSION ${REQUIRED_CMAKE_VERSION} FATAL_ERROR)

if(POLICY CMP0072)
  cmake_policy(SET CMP0072 NEW)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.15 CACHE STRING "Set the minimum OSX deployment target version")
set(CMAKE_OSX_ARCHITECTURES x86_64 CACHE STRING "Set the architecture the universal binaries for OSX should be built for")

IF(CMAKE_GENERATOR STREQUAL "Xcode")
  set(CMAKE_CONFIGURATION_TYPES Release)
ELSE()
  set(CMAKE_CONFIGURATION_TYPES Debug RelWithDebInfo Release)
ENDIF()
IF(NOT CMAKE_BUILD_TYPE)
  MESSAGE(STATUS "Setting build type to 'Release' as none was specified.")
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
ENDIF(NOT CMAKE_BUILD_TYPE)

message(STATUS "Build type CMAKE_BUILD_TYPE set to ${CMAKE_BUILD_TYPE}")

include(UseCompilerCache)
find_compiler_cache(PROGRAM ccache)

list(APPEND GODOT_CPP_CMAKE_VARS "CMAKE_BUILD_TYPE;CMAKE_CONFIGURATION_TYPES;CMAKE_TOOLCHAIN_FILE;PYTHON_EXECUTABLE;Python3_EXECUTABLE")
get_cmake_property(vars CACHE_VARIABLES)
foreach(var ${vars})
  get_property(currentHelpString CACHE "${var}" PROPERTY HELPSTRING)
    if("${currentHelpString}" MATCHES "No help, variable specified on the command line." OR "${currentHelpString}" STREQUAL "")
      if(NOT "${var}" IN_LIST GODOT_CPP_CMAKE_VARS)
        list(APPEND CL_ARGS "-D${var}=${${var}}")
      endif()
    endif()
endforeach()


##
## Global project configuration
##

project(FreeOrion)

use_compiler_cache_with_xcode()

include(GNUInstallDirs)
include(UseCodeCoverage)
include(TargetDependencies)
include(CPackComponent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

option(BUILD_SHARED_LIBS "Build project as shared libraries." ON)
option(BUILD_TESTING "Build the testing tree." OFF)
option(BUILD_CLIENT_GG "Build GiGi client" ON)
option(BUILD_CLIENT_GODOT "Build Godot client (experimental)" OFF)
option(BUILD_SERVER "Build server" ON)
option(BUILD_AI "Build AI" ON)
option(BUILD_PARSERS "Build parsers" ON)

set(GODOT_CPP_ROOT "" CACHE PATH "Path to Godot CPP libraries (empty to build internally)")

if(BUILD_TESTING)
    message( STATUS "Building Tests")
    enable_testing()
    enable_coverage()

    if(NOT TARGET unittest)
        add_custom_target(unittest
            COMMAND ${CMAKE_CTEST_COMMAND} --verbose --output-on-failure --build-config $<CONFIG>
            COMMENT "Run tests for ${CMAKE_PROJECT_NAME}"
        )
    endif()
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

if(WIN32)
    # Location of the FreeOrionSDK root
    set(CMAKE_PREFIX_PATH "${CMAKE_SOURCE_DIR}/.." CACHE PATH "Location of the FreeOrionSDK root")
    # Search dependency DLLs inside FreeOrionSDK
    target_dependencies_add_search_path("${CMAKE_PREFIX_PATH}/bin")
    set(CMAKE_INSTALL_BINDIR ".")
    set(CMAKE_INSTALL_DATADIR ".")
    set(FreeOrion_INSTALL_LIBDIR ".")
else()
    set(CMAKE_INSTALL_DATADIR "${CMAKE_INSTALL_DATADIR}/freeorion")
endif()

if(APPLE)
    set(FreeOrionSDK_VERSION 15)
    set(FreeOrionSDK_FILE "FreeOrionSDK_${FreeOrionSDK_VERSION}_Clang-MacOSX-10.15-x86_64.tar.bz2")
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/${FreeOrionSDK_FILE}")
    file(DOWNLOAD
            "https://github.com/freeorion/freeorion-sdk/releases/download/v${FreeOrionSDK_VERSION}/${FreeOrionSDK_FILE}"
            "${CMAKE_BINARY_DIR}/${FreeOrionSDK_FILE}"
            SHOW_PROGRESS
        )
    endif()
    set(FreeOrionSDK_INSTALL_VERSION 0)
    if(EXISTS "${CMAKE_BINARY_DIR}/dep/sdkversion")
        file(READ "${CMAKE_BINARY_DIR}/dep/sdkversion" FreeOrionSDK_INSTALL_VERSION)
    endif()
    if(NOT "${FreeOrionSDK_VERSION}" EQUAL "${FreeOrionSDK_INSTALL_VERSION}")
        file(REMOVE_RECURSE "${CMAKE_BINARY_DIR}/dep")
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dep")
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xfj "${CMAKE_BINARY_DIR}/${FreeOrionSDK_FILE}" WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/dep")
        file(WRITE "${CMAKE_BINARY_DIR}/dep/sdkversion" "${FreeOrionSDK_VERSION}")
    endif()
    # Location of the FreeOrionSDK root
    set(CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/dep/")
    set(CMAKE_FRAMEWORK_PATH "${CMAKE_BINARY_DIR}/dep/Frameworks")
    set(CMAKE_PROGRAM_PATH "${CMAKE_BINARY_DIR}/dep/bin")
    set(Python3_FIND_FRAMEWORK "LAST")
    set(BOOST_ROOT "${CMAKE_BINARY_DIR}/dep/")
    set(Boost_NO_SYSTEM_PATHS ON)
    set(Python3_ROOT_DIR "${CMAKE_BINARY_DIR}/dep/")
    set(Python3_FIND_STRATEGY "LOCATION")
endif()

if(UNIX)
    set(FreeOrion_INSTALL_LIBDIR "${CMAKE_INSTALL_LIBDIR}/freeorion")
endif()

if(APPLE)
    set(CMAKE_INSTALL_RPATH
        "@loader_path/../SharedSupport"
        "@loader_path/../Frameworks"
    )
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
endif()

if(NOT APPLE)
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/${FreeOrion_INSTALL_LIBDIR}")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

if(MSVC)
    # Remove default debug flags /Zi and /MDd to allow project-specific debug flag
    STRING(REGEX REPLACE " /Zi" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
    STRING(REGEX REPLACE " /Zi" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
    STRING(REGEX REPLACE " /MDd" "" CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
endif()

add_compile_options(
    # Enable (almost) all warnings on compilers
    # https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wall-316
    $<$<CXX_COMPILER_ID:GNU>:-Wall>
    # http://clang.llvm.org/docs/DiagnosticsReference.html#wall
    $<$<CXX_COMPILER_ID:Clang>:-Wall>
    # https://msdn.microsoft.com/en-us/library/thxezb7y.aspx#Anchor_1
    $<$<CXX_COMPILER_ID:MSVC>:/W4>

    # Disable "'<param>': unreferenced formal parameter" warning
    $<$<CXX_COMPILER_ID:MSVC>:/wd4100>
    # Disable "'<class>::<member>': class '<member-type>' needs to have dll-interface to be used by clients of class '<class>'" warning
    $<$<CXX_COMPILER_ID:MSVC>:/wd4251>
    # Disable "'<func>': unreferenced local function has been removed" warning
    $<$<CXX_COMPILER_ID:MSVC>:/wd4505>
    # Disable "declaration of '<type>' hides global declaration" warning
    $<$<CXX_COMPILER_ID:MSVC>:/wd4459>
    # Disable "function '<func>' marked as __forceinline not inlined" warning
    $<$<CXX_COMPILER_ID:MSVC>:/wd4714>

    # Enable C++ Exception unwind mechanics with DLL support
    $<$<CXX_COMPILER_ID:MSVC>:/EHsc>

    # Fix Fatal Error C1128: number of sections exceeded object file format limit : compile with /bigobj on SerializeEmpire.cpp
    $<$<CXX_COMPILER_ID:MSVC>:/bigobj>

    # Produce pdb files in Debug and RelWithDebInfo configuration
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:DEBUG>,$<CONFIG:RELWITHDEBINFO>>>:/Zi>
 
    # Use the multithread-specific and DLL-specific version of the run-time library in Debug Configuration
    $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:DEBUG>>:/MD>
    
    # Set character set to UTF-8
    # https://docs.microsoft.com/en-us/cpp/build/reference/utf-8-set-source-and-executable-character-sets-to-utf-8?view=msvc-170
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)

# Determine minimum Windows target (see FreeOrion SDK)
if(WIN32)
    string(REGEX REPLACE "^v([1-9][0-9]*)(.*)$" "\\1;\\2" _VS_PLATFORM_TOOLSET "${CMAKE_VS_PLATFORM_TOOLSET}")
    message( STATUS "Toolset ${CMAKE_VS_PLATFORM_TOOLSET} regex ${_VS_PLATFORM_TOOLSET}")
    list(GET _VS_PLATFORM_TOOLSET 1 _VS_PLATFORM_TOOLSET_TARGET)
    message( STATUS "Target ${_VS_PLATFORM_TOOLSET_TARGET}")
endif()

set_property(DIRECTORY APPEND
    PROPERTY COMPILE_DEFINITIONS
        # Set minimum Windows target version to Windows10
        # https://msdn.microsoft.com/en-us/library/aa383745.aspx
        # Use numeric to correctly compare it in <boost\log\detail\config.hpp>
        $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=0x0A00>
        # Default to unicode variants when using Win32 API
        # https://msdn.microsoft.com/en-us/library/dybsewaf.aspx
        $<$<PLATFORM_ID:Windows>:_UNICODE>
        # Disable "'<func>' was declared deprecated" warning
        # https://msdn.microsoft.com/en-us/library/ttcz0bys.aspx
        $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS>
        # Disable '<func>': Function call with parameters that may be unsafe" warning
        # https://msdn.microsoft.com/en-us/library/ttcz0bys.aspx
        $<$<PLATFORM_ID:Windows>:_SCL_SECURE_NO_WARNINGS>
        # Disable '<func>': The POSIX name for this item is deprecated." warning
        # https://msdn.microsoft.com/en-us/library/ttcz0bys.aspx
        $<$<PLATFORM_ID:Windows>:_CRT_NONSTDC_NO_WARNINGS>

        # Suppress "Boost.Config is older than your compiler version" warning
        $<$<PLATFORM_ID:Windows>:BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE>

        # Define platform specific macros
        $<$<PLATFORM_ID:Windows>:FREEORION_WIN32>
        $<$<PLATFORM_ID:Darwin>:FREEORION_MACOSX>
        $<$<PLATFORM_ID:Linux>:FREEORION_LINUX>
        $<$<PLATFORM_ID:FreeBSD>:FREEORION_FREEBSD>
        $<$<PLATFORM_ID:OpenBSD>:FREEORION_OPENBSD>
        $<$<PLATFORM_ID:NetBSD>:FREEORION_NETBSD>
        $<$<PLATFORM_ID:DragonFly>:FREEORION_DRAGONFLY>
        $<$<PLATFORM_ID:Haiku>:FREEORION_HAIKU>
        $<$<PLATFORM_ID:Android>:FREEORION_ANDROID>
)

if(WIN32 AND MSVC AND NOT (MSVC_VERSION LESS 1900)) # _MSC_VER >= 1900
    # fix https://bugs.python.org/issue36020
    # ToDo: remove after fix will be applied in python-cmake
    # ToDo: use GREATER_EQUAL after updating to cmake >= 3.7
    set_property(DIRECTORY APPEND
        PROPERTY COMPILE_DEFINITIONS
        HAVE_SNPRINTF
    )
endif()

if(WIN32 AND MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
endif()

if(UNIX AND NOT APPLE AND NOT ANDROID)
    add_definitions(-DENABLE_BINRELOC -DBINPATH=\"${CMAKE_INSTALL_FULL_BINDIR}\" -DSHAREPATH=\"${CMAKE_INSTALL_FULL_DATADIR}\")
endif()

include(TestForBug2795)


##
## Collect project dependencies.
##

set(MINIMUM_PYTHON_VERSION 3.9)
set(MINIMUM_BOOST_VERSION 1.73.0)

find_package(Threads)
if(ANDROID)
    find_package(Python3 ${MINIMUM_PYTHON_VERSION} COMPONENTS Interpreter REQUIRED)
    find_package(Python3 ${MINIMUM_PYTHON_VERSION} COMPONENTS Development REQUIRED)
else()
    find_package(Python3 ${MINIMUM_PYTHON_VERSION} COMPONENTS Interpreter Development REQUIRED)
endif()
message(STATUS "Python library version detected ${Python3_VERSION}")
set(Boost_PYTHON_SUFFIX "${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}")
message(STATUS "Boost python version ${Boost_PYTHON_SUFFIX}")
find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        container
        filesystem
        graph
        iostreams
        locale
        log
        log_setup
        random
        serialization
        system
        thread
    REQUIRED)

message(STATUS "Boost version detected ${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION}")

find_package(Boost OPTIONAL_COMPONENTS python${Boost_PYTHON_SUFFIX})
if(NOT Boost_PYTHON${Boost_PYTHON_SUFFIX}_FOUND)
    # try some strange boost installations
    string(SUBSTRING "${Boost_PYTHON_SUFFIX}" 0 1 Boost_PYTHON_SUFFIX)
    find_package(Boost COMPONENTS python${Boost_PYTHON_SUFFIX} REQUIRED)
endif()

if(BUILD_TESTING)
    find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS unit_test_framework REQUIRED)
endif()

find_package(ZLIB REQUIRED)
if(BUILD_CLIENT_GG)
    set(SDL_NO_MAIN true)
    find_package(SDL REQUIRED)
    find_package(Freetype REQUIRED)
    find_package(OpenGL REQUIRED)
    find_package(OpenAL REQUIRED)
    find_package(Ogg REQUIRED)
    find_package(Vorbis REQUIRED)
endif()

if(APPLE)
    find_library(CORE_FOUNDATION_LIBRARY CoreFoundation)
    find_library(ICONV_LIBRARY iconv)
endif()

if(ANDROID)
    find_library(ICUI18N_LIBRARY icui18n) # Symbol _ZTIN6icu_6210DateFormatE
    find_library(ICUUC_LIBRARY icuuc) # Symbol _ZN6icu_6213UnicodeStringD1Ev
    find_library(ICUDATA_LIBRARY icudata) # Symbol icudt62_dat
    find_library(ICONV_LIBRARY iconv) # Symbol libiconv_open
    find_library(ANDROID_LIBRARY android) # Asset Manager
    find_library(LOG_LIBRARY log) # Android logging
endif()

if(BUILD_TESTING)
    add_subdirectory(test)
endif()

if(BUILD_CLIENT_GG)
    set(BUILD_DEVEL_PACKAGE OFF CACHE INTERNAL "Disables installation of GiGi development files." FORCE)
    set(_ORIG_CMAKE_INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR})
    set(CMAKE_INSTALL_LIBDIR "${FreeOrion_INSTALL_LIBDIR}")
    add_subdirectory(GG)
    set(CMAKE_INSTALL_LIBDIR ${_ORIG_CMAKE_INSTALL_LIBDIR})
    unset(_ORIG_CMAKE_INSTALL_LIBDIR)
endif()

set_property(DIRECTORY APPEND
    PROPERTY COMPILE_DEFINITIONS
        # Boost iostreams doesn't honor BOOST_ALL_NO_LIB when linking zlib.
        # It also tries to link by default to the boost in-source zlib librar,
        # which we don't want.
        # http://www.boost.org/doc/libs/1_59_0/boost/iostreams/detail/config/zlib.hpp
        $<$<PLATFORM_ID:Windows>:BOOST_ZLIB_BINARY=zlib.lib>

        # with boost 1.61 some boost::optional internals were changed. However
        # boost::spirit relies on some API the old implementation provided.
        # This define enables the usage of the old boost::optional
        # implementation.  Boost upstream tracks this bug as #12349
        $<$<AND:$<VERSION_GREATER:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.60>,$<VERSION_LESS:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.67>>:BOOST_OPTIONAL_CONFIG_USE_OLD_IMPLEMENTATION_OF_OPTIONAL>

        # We don't need localized output of Boost date_time and not setting
        # the define causes the inclusion of code, which contains std::tolower.
        # This however causes a macro substitutions caused by the libpython
        # headers, which in turn breaks the build.  Python 2.7.13 should have
        # fixed this with python bug #10910
        $<$<PLATFORM_ID:Darwin>:BOOST_DATE_TIME_NO_LOCALE>
        $<$<PLATFORM_ID:FreeBSD>:BOOST_DATE_TIME_NO_LOCALE>
        # Forcing backtrace using api for boost stacktrace due to lacking on ::Unwind side
        $<$<PLATFORM_ID:FreeBSD>:BOOST_STACKTRACE_USE_LIBC_BACKTRACE_FUNCTION>
        # Workaround for boost 1.81 issue https://github.com/boostorg/phoenix/issues/111
        $<$<VERSION_GREATER_EQUAL:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.81>:BOOST_PHOENIX_STL_TUPLE_H_>
)


##
## Define main project targets.
##

# To run the version generation every compile we need to deferr the
# execution to a separate target and the existing python command
execute_process(
    COMMAND
    "${Python3_EXECUTABLE}"
    "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
    "${CMAKE_SOURCE_DIR}"
    "CMake"
    "${Boost_PYTHON_SUFFIX}"
    RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
        message(FATAL_ERROR "FreeOrion version failed")
endif()

set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/util/Version.cpp
    PROPERTIES
    GENERATED TRUE
)

include(FreeOrionVersion)

if(Apple)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/Xcode/Info.plist"
        PROPERTIES
        GENERATED TRUE
    )
endif()

if(NOT WIN32 AND NOT ANDROID)
    add_library(freeorioncommon "")
endif()


if(WIN32 OR ANDROID)
    add_library(freeorioncommon STATIC "")

    set_property(TARGET freeorioncommon
        PROPERTY
        OUTPUT_NAME Common
    )
endif()

target_compile_options(freeorioncommon
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=512>
        $<$<CXX_COMPILER_ID:AppleClang>:-ftemplate-depth=512>
)

if(APPLE)
    set_target_properties(freeorioncommon
        PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

target_compile_definitions(freeorioncommon
    PUBLIC
        $<$<NOT:$<PLATFORM_ID:Android>>:BOOST_ALL_DYN_LINK>
    PRIVATE
        -DFREEORION_BUILD_COMMON
)

target_include_directories(freeorioncommon SYSTEM
    PUBLIC
        ${Python3_INCLUDE_DIRS}
)

target_link_libraries(freeorioncommon
    PUBLIC
        Boost::boost
        Boost::disable_autolinking
        $<$<NOT:$<PLATFORM_ID:Android>>:Boost::dynamic_linking>
    INTERFACE
        Boost::filesystem
        Boost::iostreams
        Boost::locale
        Boost::log_setup
        Boost::log
        Boost::python${Boost_PYTHON_SUFFIX}
        Boost::thread
        Boost::serialization
        ${Python3_LIBRARIES}
        ZLIB::ZLIB
        ${ICUI18N_LIBRARY}
        ${ICUUC_LIBRARY}
        ${ICUDATA_LIBRARY}
        ${ANDROID_LIBRARY}
        ${LOG_LIBRARY}
    PRIVATE
        # boost::uuid requires the bcrypt Windows API on Windows since boost 1.67
        # see https://github.com/boostorg/uuid/issues/68
        $<$<AND:$<PLATFORM_ID:Windows>,$<VERSION_GREATER:${Boost_MAJOR_VERSION}.${Boost_MINOR_VERSION},1.66>>:bcrypt>
        ${CORE_FOUNDATION_LIBRARY}
)

if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
	target_link_libraries(freeorioncommon PUBLIC execinfo)
endif()
if(CMAKE_SYSTEM_NAME MATCHES "Haiku")
	target_link_libraries(freeorioncommon PUBLIC network)
endif()


add_library(freeorionparseobj OBJECT "")

# Object libraries only understand the modern CMake approach of defining
# dependencies via imported targets and target_link_libraries starting 3.12.
# For now manually unpack the compiler definitions and include directories.
target_compile_definitions(freeorionparseobj
    PUBLIC
        $<TARGET_PROPERTY:Boost::boost,INTERFACE_COMPILE_DEFINITIONS>
)

target_include_directories(freeorionparseobj
    PUBLIC
        $<TARGET_PROPERTY:Boost::boost,INTERFACE_INCLUDE_DIRECTORIES>
        ${Python3_INCLUDE_DIRS}
)

set_property(TARGET freeorionparseobj
    PROPERTY
    POSITION_INDEPENDENT_CODE ON
)

if(MSVC)
    # The debug information produced in this project is over the MSVC linking limit, remove it
    get_target_property(PARSEOBJ_COMPILE_OPTIONS freeorionparseobj COMPILE_OPTIONS)
    list(REMOVE_ITEM PARSEOBJ_COMPILE_OPTIONS 
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:DEBUG>,$<CONFIG:RELWITHDEBINFO>>>:/Zi>"
    )
    set_target_properties(freeorionparseobj 
        PROPERTIES
        COMPILE_OPTIONS "${PARSEOBJ_COMPILE_OPTIONS}"
    )

    target_precompile_headers(freeorionparseobj PRIVATE "msvc2022/Parsers/StdAfx.h")
endif()

target_compile_options(freeorionparseobj
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=512>
        $<$<CXX_COMPILER_ID:AppleClang>:-ftemplate-depth=512>
        $<$<AND:$<NOT:$<BOOL:${BUILD_TESTING}>>,$<CXX_COMPILER_ID:GNU>>:-O3>
)

target_compile_definitions(freeorionparseobj
    PUBLIC
        BOOST_ALL_NO_LIB
        $<$<NOT:$<PLATFORM_ID:Android>>:BOOST_ALL_DYN_LINK>
    PRIVATE
        -DNDEBUG
        -DFREEORION_BUILD_PARSE
)

if(NOT WIN32 AND NOT ANDROID)
    add_library(freeorionparse $<TARGET_OBJECTS:freeorionparseobj>)
endif()

if(WIN32 OR ANDROID)
    add_library(freeorionparse STATIC $<TARGET_OBJECTS:freeorionparseobj>)

    set_property(TARGET freeorionparse
        PROPERTY
        OUTPUT_NAME Parsers
    )
endif()

if(MSVC)
    # The debug information produced in this project is over the MSVC linking limit, remove it
    get_target_property(PARSE_COMPILE_OPTIONS freeorionparse COMPILE_OPTIONS)
    list(REMOVE_ITEM PARSE_COMPILE_OPTIONS 
        "$<$<AND:$<CXX_COMPILER_ID:MSVC>,$<OR:$<CONFIG:DEBUG>,$<CONFIG:RELWITHDEBINFO>>>:/Zi>"
    )
    set_target_properties(freeorionparse
        PROPERTIES
        COMPILE_OPTIONS "${PARSEOBJ_COMPILE_OPTIONS}"
    )
endif()

if(APPLE)
    # Xcode doesn't build freeorionparse because it doesn't have any
    # source code files associated and only links the freeorionparselib
    # objects into a shared library.  Adding an empty file fixes this.
    # https://cmake.org/cmake/help/v3.0/command/add_library.html
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/xcode_dummy.cpp" "")
    target_sources(freeorionparse
        PRIVATE
            "${CMAKE_CURRENT_BINARY_DIR}/xcode_dummy.cpp"
    )

    set_target_properties(freeorionparse
        PROPERTIES
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

if(BUILD_SERVER)
    add_executable(freeoriond "")
    
    if(WIN32)
        set_property(TARGET freeoriond
            PROPERTY
                OUTPUT_NAME FreeOrionD
        )
    endif()
    
    target_compile_definitions(freeoriond
        PRIVATE
            -DFREEORION_BUILD_SERVER
    )

    target_link_libraries(freeoriond
        PRIVATE
            freeorioncommon
            freeorionparse
    )
    
    target_dependencies_copy_to_build(freeoriond)
    target_dependent_data_symlink_to_build(freeoriond ${PROJECT_SOURCE_DIR}/default)
endif()

if(BUILD_AI)
    add_executable(freeorionca "")
    
    if(WIN32)
        set_property(TARGET freeorionca
            PROPERTY
                OUTPUT_NAME FreeOrionCA
        )
    endif()
    
    target_compile_definitions(freeorionca
        PRIVATE
            -DFREEORION_BUILD_AI
    )
    
    target_link_libraries(freeorionca
        PRIVATE
            freeorioncommon
            freeorionparse
    )
    
    target_dependencies_copy_to_build(freeorionca)
    target_dependent_data_symlink_to_build(freeorionca ${PROJECT_SOURCE_DIR}/default)
endif()

if(BUILD_CLIENT_GG)
    add_executable(freeorion "")

    if(WIN32)
        set_property(TARGET freeorion
            PROPERTY
                OUTPUT_NAME FreeOrion
        )
        if(MSVC AND (CMAKE_VERSION VERSION_GREATER "3.6"))
            set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                PROPERTY
                    VS_STARTUP_PROJECT freeorion
            )
        endif()
    endif()

    if(APPLE)
        set_property(TARGET freeorion
            PROPERTY
                OUTPUT_NAME FreeOrion
        )

        list(APPEND FREEORION_ASSETS
            Changelog.md
            default
        )
        target_sources(freeorion
            PRIVATE
                ${FREEORION_ASSETS}
        )
        set_property(
            SOURCE
                ${FREEORION_ASSETS}
            PROPERTY
                MACOSX_PACKAGE_LOCATION "Resources/"
        )
        # Workaround for creating the initial Info.plist, which needs to exist
        # at configuration time.
        execute_process(
            COMMAND
            "${Python3_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
            "${CMAKE_SOURCE_DIR}"
            "CMake"
        )
        set_target_properties(freeorion
            PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
                MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/packaging/Info.plist"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ""
                XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO"
                RESOURCE "${PROJECT_SOURCE_DIR}/client/human/main.xib"
        )
    endif()

    target_compile_definitions(freeorion
        PRIVATE
            -DFREEORION_BUILD_HUMAN
    )

    target_include_directories(freeorion SYSTEM
        PRIVATE
            ${SDL_INCLUDE_DIRS}
            ${OPENGL_INCLUDE_DIR}
            ${OPENAL_INCLUDE_DIR}
            ${OGG_INCLUDE_DIRS}
            ${VORBIS_INCLUDE_DIRS}
            ${FREETYPE_INCLUDE_DIRS}
    )

    target_link_libraries(freeorion
        freeorioncommon
        freeorionparse
        GiGi::GiGi
        ${SDL_LIBRARIES}
        $<IF:$<BOOL:${OPENGL_opengl_LIBRARY}>,${OPENGL_opengl_LIBRARY},${OPENGL_gl_LIBRARY}>
        ${OPENAL_LIBRARY}
        ${OGG_LIBRARIES}
        ${VORBIS_LIBRARIES}
        ${ICONV_LIBRARY}
    )

    target_dependencies_copy_to_build(freeorion)
    target_dependent_data_symlink_to_build(freeorion ${PROJECT_SOURCE_DIR}/default)
endif()

if(BUILD_CLIENT_GODOT)
    add_library(freeoriongodot SHARED "")

    if(CMAKE_BUILD_TYPE MATCHES Debug)
        set(GODOT_CPP_BUILD_TYPE Debug)
    else()
        set(GODOT_CPP_BUILD_TYPE Release)
    endif()

    if(NOT GODOT_CPP_ROOT)
        set(GODOT_CPP_ROOT_BUILT "${CMAKE_BINARY_DIR}/godot-cpp")
        set(GODOT_CPP_GEN_INCLUDE_DIRS "${CMAKE_BINARY_DIR}/godot-cpp-build/include/gen")

        include(ExternalProject)
        ExternalProject_Add(godotcpp
            GIT_REPOSITORY https://github.com/godotengine/godot-cpp
            GIT_TAG 316b91c5f5d89d82ae935513c28df78f9e238e8b
            SOURCE_DIR ${CMAKE_BINARY_DIR}/godot-cpp
            BINARY_DIR ${CMAKE_BINARY_DIR}/godot-cpp-build
            CMAKE_ARGS
                -G ${CMAKE_GENERATOR}
                -D CMAKE_BUILD_TYPE=${GODOT_CPP_BUILD_TYPE}
                -D CMAKE_CONFIGURATION_TYPES=${GODOT_CPP_BUILD_TYPE}
                -D CMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
                -D CMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
                -D PYTHON_EXECUTABLE=${Python3_EXECUTABLE}
                -D Python3_EXECUTABLE=${Python3_EXECUTABLE}
                ${CL_ARGS}
            INSTALL_COMMAND ""
        )

        add_dependencies(freeoriongodot godotcpp)
    else()
        set(GODOT_CPP_ROOT_BUILT "${GODOT_CPP_ROOT}")
        set(GODOT_CPP_GEN_INCLUDE_DIRS "${GODOT_CPP_ROOT_BUILT}/include/gen")
    endif()
    message(STATUS "Godot C++ root ${GODOT_CPP_ROOT_BUILT}")

    # ToDo: remove after fix Godot headers https://github.com/godotengine/godot_headers/pull/81
    set_target_properties(freeoriongodot PROPERTIES CXX_VISIBILITY_PRESET default)

    target_compile_definitions(freeoriongodot
        PRIVATE
            -DFREEORION_BUILD_HUMAN_GODOT
    )

    target_include_directories(freeoriongodot SYSTEM
        PRIVATE
            ${GODOT_CPP_ROOT_BUILT}/include
            ${GODOT_CPP_ROOT_BUILT}/include/core
            ${GODOT_CPP_GEN_INCLUDE_DIRS}
            ${GODOT_CPP_ROOT_BUILT}/godot-headers
    )

    # Create the correct name (godot.os.build_type.system_bits)
    # Synchronized with godot-cpp's CMakeLists.txt

    set(BITS 32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(BITS 64)
    endif(CMAKE_SIZEOF_VOID_P EQUAL 8)

    string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME)
    string(TOLOWER ${GODOT_CPP_BUILD_TYPE} BUILD_TYPE)

    if(ANDROID)
        # Added the android abi after system name
        set(SYSTEM_NAME ${SYSTEM_NAME}.${ANDROID_ABI})
    endif()

    if(CMAKE_VERSION VERSION_GREATER "3.13")
        target_link_directories(freeoriongodot
            PRIVATE
                ${GODOT_CPP_ROOT_BUILT}/bin/
        )
        target_link_libraries(freeoriongodot
            godot-cpp.${SYSTEM_NAME}.${BUILD_TYPE}$<$<NOT:$<PLATFORM_ID:Android>>:.${BITS}>
        )
    else()
        target_link_libraries(freeoriongodot
            ${GODOT_CPP_ROOT_BUILT}/bin/libgodot-cpp.${SYSTEM_NAME}.${BUILD_TYPE}$<$<NOT:$<PLATFORM_ID:Android>>:.${BITS}>.a
        )
    endif()

    target_link_libraries(freeoriongodot
        freeorioncommon
        freeorionparse
        ${ICONV_LIBRARY}
    )

    target_dependencies_copy_to_build(freeoriongodot)
endif()

if(WIN32 AND BUILD_CLIENT_GG)
    add_custom_command(TARGET freeorion POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_PREFIX_PATH}/bin/python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}.dll" "$<TARGET_FILE_DIR:freeorion>"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_PREFIX_PATH}/bin/python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR}.zip" "$<TARGET_FILE_DIR:freeorion>"
    )
endif()

if(APPLE AND BUILD_CLIENT_GG)
    add_custom_command(TARGET freeorion POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:freeorion>/../Executables"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeorionca>" "$<TARGET_FILE_DIR:freeorion>/../Executables"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeoriond>" "$<TARGET_FILE_DIR:freeorion>/../Executables"
        COMMAND
            ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:freeorion>/../SharedSupport/lib"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeorioncommon>" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeorionparse>" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:GiGi>" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_ATOMIC_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_CHRONO_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_CONTAINER_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_DATE_TIME_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_FILESYSTEM_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_GRAPH_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_IOSTREAMS_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_LOCALE_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_LOG_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_LOG_SETUP_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_REGEX_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_PYTHON${Boost_PYTHON_SUFFIX}_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_RANDOM_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_SERIALIZATION_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_SYSTEM_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_THREAD_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:SDL::SDL>" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:freeorion>/../SharedSupport/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}"
        COMMAND
           cp -R "${CMAKE_BINARY_DIR}/dep/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport/lib/python${Python3_VERSION_MAJOR}.${Python3_VERSION_MINOR}"
        COMMAND
           ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:freeorion>/../Frameworks"
        COMMAND
           cp -R "${CMAKE_FRAMEWORK_PATH}" "$<TARGET_FILE_DIR:freeorion>/.."
    )
endif()

if(BUILD_CLIENT_GODOT)
    add_custom_command(TARGET freeoriongodot POST_BUILD
        COMMAND
           ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/godot/bin/${SYSTEM_NAME}.${BITS}
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "$<TARGET_FILE:freeoriongodot>" "${CMAKE_SOURCE_DIR}/godot/bin/${SYSTEM_NAME}.${BITS}/$<TARGET_FILE_NAME:freeoriongodot>"
        COMMAND
            ${CMAKE_COMMAND}
            -DMODE=DATA_LINK
            "-DDESTINATION=${CMAKE_SOURCE_DIR}/godot/assets/"
            "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}/default/data/art"
            -P "${PROJECT_SOURCE_DIR}/cmake/TargetDependencies.cmake"
        COMMAND
            ${CMAKE_COMMAND}
            -DMODE=DATA_LINK
            "-DDESTINATION=${CMAKE_SOURCE_DIR}/godot/assets/"
            "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}/default/data/fonts"
            -P "${PROJECT_SOURCE_DIR}/cmake/TargetDependencies.cmake"
        COMMAND
            ${CMAKE_COMMAND}
            -DMODE=DATA_LINK
            "-DDESTINATION=${CMAKE_SOURCE_DIR}/godot/assets/"
            "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}/default/data/sound"
            -P "${PROJECT_SOURCE_DIR}/cmake/TargetDependencies.cmake"
    )
    if(ANDROID)
        add_custom_command(TARGET freeoriongodot POST_BUILD
            COMMAND
                ${CMAKE_COMMAND} -E copy_if_different "${ICONV_LIBRARY}" "${CMAKE_SOURCE_DIR}/godot/bin/${SYSTEM_NAME}.${BITS}/libiconv.so"
            COMMAND
                ${CMAKE_STRIP} -g -S -d --strip-debug "${CMAKE_SOURCE_DIR}/godot/bin/${SYSTEM_NAME}.${BITS}/libiconv.so"
            COMMAND
                ${CMAKE_STRIP} -g -S -d --strip-debug "${CMAKE_SOURCE_DIR}/godot/bin/${SYSTEM_NAME}.${BITS}/$<TARGET_FILE_NAME:freeoriongodot>"
            COMMAND
                ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/godot/default/
            COMMAND
                ${CMAKE_COMMAND}
                -DMODE=DATA_LINK
                "-DDESTINATION=${CMAKE_SOURCE_DIR}/godot/default/"
                "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}/default/scripting"
                -P "${PROJECT_SOURCE_DIR}/cmake/TargetDependencies.cmake"
            COMMAND
                ${CMAKE_COMMAND}
                -DMODE=DATA_LINK
                "-DDESTINATION=${CMAKE_SOURCE_DIR}/godot/default/"
                "-DSOURCE_PATH=${PROJECT_SOURCE_DIR}/default/stringtables"
                -P "${PROJECT_SOURCE_DIR}/cmake/TargetDependencies.cmake"
        )
    endif()
endif()

##
## Recurse into sources.
##

if(BUILD_AI OR BUILD_CLIENT_GG OR BUILD_CLIENT_GODOT)
    add_subdirectory(client)
endif()
add_subdirectory(combat)
add_subdirectory(Empire)
add_subdirectory(network)
add_subdirectory(parse)
if(BUILD_AI OR BUILD_SERVER)
    add_subdirectory(python)
endif()
if(BUILD_SERVER)
    add_subdirectory(server)
endif()
if(BUILD_CLIENT_GG)
    add_subdirectory(UI)
endif()
add_subdirectory(universe)
add_subdirectory(util)
add_subdirectory(check)
add_subdirectory(doc)


##
## Install targets and package project.
##

cpack_add_component(binaries
    DISPLAY_NAME "Binaries only"
    REQUIRED
)

cpack_add_component(resources
    DISPLAY_NAME "Game resources"
)

if(BUILD_TESTING)
    cpack_add_component(binaries-test
        DISPLAY_NAME "Test binaries only"
    )
endif()

if(WIN32)
    install(CODE [[
        list(APPEND pre_exclude_regexes "api-ms-.*") # windows API
        list(APPEND pre_exclude_regexes "ext-ms-.*") # windows API
        list(APPEND pre_exclude_regexes "ieshims.dll") # windows API
        list(APPEND pre_exclude_regexes "emclient.dll") # windows API
        list(APPEND pre_exclude_regexes "devicelockhelpers.dll") # windows API
        list(APPEND pre_exclude_regexes "wpaxholder.dll") # ...

        list(APPEND post_exclude_regexes ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][\\/][Ss][Yy][Ss][Tt][Ee][Mm]32.*") # windows system dlls
        list(APPEND post_exclude_regexes ".*[Ww][Ii][Nn][Dd][Oo][Ww][Ss][\\/][Ss][Yy][Ss][Ww][Oo][Ww]64.*") # windows system dlls
    ]]
    ALL_COMPONENTS)
endif()

if(BUILD_CLIENT_GG)
    install(
        TARGETS
            freeorion
        COMPONENT binaries
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION .
        RESOURCE
    )
    if(WIN32)
        install(CODE [[
            file(GET_RUNTIME_DEPENDENCIES
                EXECUTABLES $<TARGET_FILE:freeorion>
                RESOLVED_DEPENDENCIES_VAR deps_install_fo
                PRE_EXCLUDE_REGEXES ${pre_exclude_regexes}
                POST_EXCLUDE_REGEXES ${post_exclude_regexes}
            )
        ]]
        ALL_COMPONENTS)
    endif()
endif()

if(NOT APPLE AND NOT ANDROID)
    if(NOT WIN32)
        install(
            TARGETS
                freeorioncommon
                freeorionparse
            LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()
    
    if(BUILD_SERVER)
        install(
            TARGETS
                freeoriond
            COMPONENT binaries
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION .
        )
        if(WIN32)
            install(CODE [[
                file(GET_RUNTIME_DEPENDENCIES
                    EXECUTABLES $<TARGET_FILE:freeoriond>
                    RESOLVED_DEPENDENCIES_VAR deps_install_fod
                    PRE_EXCLUDE_REGEXES ${pre_exclude_regexes}
                    POST_EXCLUDE_REGEXES ${post_exclude_regexes}
                )
            ]]
            ALL_COMPONENTS)
        endif()
    endif()

    if(BUILD_AI)
        install(
            TARGETS
                freeorionca
            COMPONENT binaries
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION .
        )
        if(WIN32)
            install(CODE [[
                file(GET_RUNTIME_DEPENDENCIES
                    EXECUTABLES $<TARGET_FILE:freeorionca>
                    RESOLVED_DEPENDENCIES_VAR deps_install_foca
                    PRE_EXCLUDE_REGEXES ${pre_exclude_regexes}
                    POST_EXCLUDE_REGEXES ${post_exclude_regexes}
                )
            ]]
            ALL_COMPONENTS)
        endif()
    endif()

    if(NOT WIN32)
        if(BUILD_CLIENT_GG)
            install(
                FILES
                ${CMAKE_SOURCE_DIR}/packaging/org.freeorion.FreeOrion.desktop
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
            )
        endif()
    endif()

    if(BUILD_CLIENT_GODOT)
        install(
            TARGETS
                freeoriongodot
            COMPONENT binaries
            LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
        if(WIN32)
            install(CODE [[
                file(GET_RUNTIME_DEPENDENCIES
                    EXECUTABLES $<TARGET_FILE:freeoriongodot>
                    RESOLVED_DEPENDENCIES_VAR deps_install_fog
                    PRE_EXCLUDE_REGEXES ${pre_exclude_regexes}
                    POST_EXCLUDE_REGEXES ${post_exclude_regexes}
                )
            ]]
            ALL_COMPONENTS)
        endif()
    endif()

    if(NOT WIN32)
        install(
            FILES
            ${CMAKE_SOURCE_DIR}/packaging/org.freeorion.FreeOrion.metainfo.xml
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
        )
    endif()

    if(BUILD_CLIENT_GG)
        install(
            DIRECTORY default/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/default
            COMPONENT resources
            PATTERN "*~" EXCLUDE
            PATTERN "*.pyc" EXCLUDE
            PATTERN "tox.ini" EXCLUDE
        )

        if(NOT WIN32)
            foreach(SIZE 16 24 32 64 128 256)
                install(
                    FILES
                    ${CMAKE_SOURCE_DIR}/default/data/art/icons/FO_Icon_${SIZE}x${SIZE}.png
                    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${SIZE}x${SIZE}/apps/
                    RENAME org.freeorion.FreeOrion.png
                )
            endforeach()
        endif()
    else()
        install(
            DIRECTORY default/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/default
            COMPONENT resources
            PATTERN "*~" EXCLUDE
            PATTERN "*.pyc" EXCLUDE
            PATTERN "*.png" EXCLUDE
            PATTERN "*.ogg" EXCLUDE
            PATTERN "tox.ini" EXCLUDE
        )
    endif()
endif()

if(WIN32)
    install(CODE [[
        set(deps_install ${deps_install_fo} ${deps_install_fod} ${deps_install_foca} ${deps_install_fog})
        message(STATUS "Install dependencies ${deps_install}")
        foreach(dep ${deps_install})
            file(INSTALL ${dep} DESTINATION ${CMAKE_INSTALL_PREFIX})
            get_filename_component(dep_name ${dep} NAME_WLE)
            if(${dep_name} MATCHES "^python[0-9]+")
                get_filename_component(dep_dir ${dep} DIRECTORY)
                file(INSTALL "${dep_dir}/${dep_name}.zip" DESTINATION ${CMAKE_INSTALL_PREFIX})
            endif()
        endforeach()
    ]]
    COMPONENT binaries)
    if(BUILD_TESTING)
        if(BUILD_PARSERS)
            install(CODE [[
                file(GET_RUNTIME_DEPENDENCIES
                    EXECUTABLES $<TARGET_FILE:fo_unittest_parse>
                    RESOLVED_DEPENDENCIES_VAR deps_install_fotp
                    PRE_EXCLUDE_REGEXES ${pre_exclude_regexes}
                    POST_EXCLUDE_REGEXES ${post_exclude_regexes}
                )
                list(REMOVE_ITEM deps_install_fotp ${deps_install_fo} ${deps_install_fod} ${deps_install_foca} ${deps_install_fog})
                message(STATUS "Install dependencies ${deps_install_fotp}")
                foreach(dep ${deps_install_fotp})
                    file(INSTALL ${dep} DESTINATION ${CMAKE_INSTALL_PREFIX})
                endforeach()
            ]]
            COMPONENT binaries-test)
            install(TARGETS fo_unittest_parse
                COMPONENT binaries-test
                LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
                RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})             
        endif()
    endif()
endif()

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${FreeOrion_VERSION_FILENAME})
set(CPACK_PACKAGE_VENDOR "FreeOrion Community")
set(CPACK_PACKAGE_CONTACT http://freeorion.org/forum)
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "FreeOrion is a free, open source, turn-based space empire and galactic conquest (4X) computer game being designed and built by the FreeOrion project. FreeOrion is inspired by the tradition of the Master of Orion games, but is not a clone or remake of that series or any other game.")
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_SOURCE_DIR}/default/COPYING)
if(NOT APPLE)
    set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_NAME})
else()
    set(CPACK_SYSTEM_NAME MacOSX)
endif()
if(WIN32)
    set(PACKAGE_FILE_SYSTEM_NAME Win32)
elseif(APPLE)
    set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME}_${CMAKE_OSX_DEPLOYMENT_TARGET})
else()
    set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME})
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_FILE_SYSTEM_NAME}")
set(CPACK_ARCHIVE_BINARIES_FILE_NAME "${CPACK_PACKAGE_NAME}_${CPACK_PACKAGE_VERSION}_${PACKAGE_FILE_SYSTEM_NAME}_BinariesOnly")

# NSIS-specific settings
set(CPACK_NSIS_COMPRESSOR bzip2)
set(CPACK_NSIS_URL_INFO_ABOUT http://freeorion.org)
string(REPLACE "/" "\\\\" cmake_home_directory_with_native_windows_path_separators ${CMAKE_HOME_DIRECTORY})

# RPM-specific settings
set(CPACK_RPM_PACKAGE_LICENSE GPL)
set(CPACK_RPM_PACKAGE_REQUIRES)

# Deb-specific settings
set(CPACK_DEBIAN_PACKAGE_SECTION games)

# DMG-specific settings
set(CPACK_DMG_VOLUME_NAME "${CMAKE_PROJECT_NAME}")
set(CPACK_DMG_FORMAT UDZO)

if(APPLE)
    set(CPACK_GENERATOR DragNDrop)
elseif(UNIX)
    set(CPACK_GENERATOR TGZ TBZ2)
    find_program(RPMBUILD rpmbuild)
    if(RPMBUILD)
        list(APPEND CPACK_GENERATOR RPM)
    endif()
    set(RPMBUILD ${RPMBUILD} CACHE INTERNAL "")
    find_program(DPKG dpkg)
    if(DPKG)
        list(APPEND CPACK_GENERATOR DEB)
    endif()
    set(DPKG ${DPKG} CACHE INTERNAL "")
elseif(WIN32)
    set(CPACK_GENERATOR NSIS)
endif()


########################################
# Source Packaging                     #
########################################

if(UNIX)
    set(CPACK_SOURCE_GENERATOR TGZ)
elseif(WIN32)
    set(CPACK_SOURCE_GENERATOR ZIP)
endif()

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
