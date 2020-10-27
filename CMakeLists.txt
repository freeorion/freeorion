cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

cmake_policy(SET CMP0042 NEW)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_HOME_DIRECTORY}/cmake ${CMAKE_HOME_DIRECTORY}/GG/cmake)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.9 CACHE STRING "Set the minimum OSX deployment target version")
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


##
## Global project configuration
##

project(FreeOrion)

use_compiler_cache_with_xcode()

include(GNUInstallDirs)
include(UseCodeCoverage)
include(TargetDependencies)

set(FreeOrion_VERSION 0.4.10+)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

option(BUILD_SHARED_LIBS "Build project as shared libraries." ON)
option(BUILD_TESTING "Build the testing tree." OFF)
option(BUILD_CLIENT_GG "Build GiGi client" ON)
option(BUILD_CLIENT_GODOT "Build Godot client (experimental)" OFF)
if(NOT GODOT_CPP_ROOT)
    set(GODOT_CPP_ROOT "" CACHE STRING "Path to Godot CPP libraries" FORCE)
endif()
option(BUILD_SERVER "Build server" ON)
option(BUILD_AI "Build AI" ON)

if(BUILD_TESTING)
    message( STATUS "Building Tests")
    enable_testing()
    enable_coverage()

    if(NOT TARGET unittest)
        add_custom_target(unittest
            COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure --build-config $<CONFIG>
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
    set(FreeOrion_INSTALL_LIBDIR ".")
endif()

if(APPLE)
    set(FreeOrionSDK_VERSION 11)
    set(FreeOrionSDK_FILE "FreeOrionSDK_${FreeOrionSDK_VERSION}_Clang-MacOSX-10.9-x86_64.tar.bz2")
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
    set(CMAKE_PROGRAM_PATH "${CMAKE_FRAMEWORK_PATH}/Python.framework/Versions/3.5/bin")
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
        # Set minimum Windows target version to WindowsXP or Vista depending on toolset
        # https://msdn.microsoft.com/en-us/library/aa383745.aspx
        # Use numeric to correctly compare it in <boost\log\detail\config.hpp>
        $<$<PLATFORM_ID:Windows>:_WIN32_WINNT=$<IF:$<STREQUAL:${_VS_PLATFORM_TOOLSET_TARGET},_xp>,0x0501,0x0600>>
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
        $<$<PLATFORM_ID:Haiku>:FREEORION_HAIKU>
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

if(UNIX AND NOT APPLE)
    add_definitions(-DENABLE_BINRELOC -DBINPATH=\"${CMAKE_INSTALL_FULL_BINDIR}\" -DSHAREPATH=\"${CMAKE_INSTALL_FULL_DATADIR}\")
endif()

include(TestForBug2795)


##
## Collect project dependencies.
##

set(MINIMUM_PYTHON_VERSION 3.5)
set(MINIMUM_BOOST_VERSION 1.60.0)

find_package(Threads)
find_package(PythonInterp ${MINIMUM_PYTHON_VERSION} REQUIRED)
if(BUILD_AI OR BUILD_SERVER)
    find_package(PythonLibs ${MINIMUM_PYTHON_VERSION} REQUIRED)
endif()
find_package(Boost ${MINIMUM_BOOST_VERSION}
    COMPONENTS
        filesystem
        iostreams
        locale
        log
        log_setup
        serialization
        system
        thread
    REQUIRED)

if(BUILD_AI OR BUILD_SERVER)
    find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} REQUIRED)
    set(Boost_PYTHON_SUFFIX "${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
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
)


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

if(Apple)
    set_source_files_properties(
        "${CMAKE_CURRENT_SOURCE_DIR}/Xcode/Info.plist"
        PROPERTIES
        GENERATED TRUE
    )
endif()

if(NOT WIN32)
    add_library(freeorioncommon "")
endif()


if(WIN32)
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
        -DBOOST_ALL_DYN_LINK
    PRIVATE
        -DFREEORION_BUILD_COMMON
        -DFREEORION_PYTHON_VERSION=\"${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}\"
)

target_link_libraries(freeorioncommon
    PUBLIC
        Boost::boost
        Boost::disable_autolinking
        Boost::dynamic_linking
    INTERFACE
        Boost::filesystem
        Boost::iostreams
        Boost::locale
        Boost::log
        Boost::log_setup
        Boost::thread
        Boost::serialization
        ZLIB::ZLIB
    PRIVATE
        ${CORE_FOUNDATION_LIBRARY}
)

if(WIN32 AND ${Boost_VERSION} GREATER 106699) # boost >= 1.67
    # fix https://github.com/boostorg/uuid/issues/68
    find_library(BCRYPT_LIBRARY bcrypt)
    target_link_libraries(freeorioncommon
        PRIVATE
            ${BCRYPT_LIBRARY}
    )
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Haiku")
	target_link_libraries(freeorioncommon PUBLIC network)
endif()

add_dependencies(freeorioncommon freeorionversion)


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
endif()

target_compile_options(freeorionparseobj
    PRIVATE
        $<$<CXX_COMPILER_ID:Clang>:-ftemplate-depth=512>
        $<$<CXX_COMPILER_ID:AppleClang>:-ftemplate-depth=512>
        $<$<AND:$<NOT:$<BOOL:${BUILD_TESTING}>>,$<CXX_COMPILER_ID:GNU>>:-O3>
)

target_compile_definitions(freeorionparseobj
    PUBLIC
        -DBOOST_ALL_DYN_LINK
    PRIVATE
        -DNDEBUG
        -DFREEORION_BUILD_PARSE
)

if(NOT WIN32)
    add_library(freeorionparse $<TARGET_OBJECTS:freeorionparseobj>)
endif()

if(WIN32)
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
    
    target_include_directories(freeoriond SYSTEM
        PRIVATE
            ${PYTHON_INCLUDE_PATH}
    )
    
    target_link_libraries(freeoriond
        PRIVATE
            freeorioncommon
            freeorionparse
            Boost::python${Boost_PYTHON_SUFFIX}
            ${PYTHON_LIBRARIES}
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
    
    target_include_directories(freeorionca SYSTEM
        PRIVATE
            ${PYTHON_INCLUDE_PATH}
    )
    
    target_link_libraries(freeorionca
        freeorioncommon
        freeorionparse
        Boost::python${Boost_PYTHON_SUFFIX}
        ${PYTHON_LIBRARIES}
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
            "${PYTHON_EXECUTABLE}"
            "${CMAKE_SOURCE_DIR}/cmake/make_versioncpp.py"
            "${CMAKE_SOURCE_DIR}"
            "CMake"
        )
        set_target_properties(freeorion
            PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
                MACOSX_BUNDLE_INFO_PLIST "${PROJECT_SOURCE_DIR}/packaging/Info.plist"
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
        ${OPENGL_gl_LIBRARY}
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

    set_target_properties(freeoriongodot PROPERTIES CXX_VISIBILITY_PRESET default)

    target_compile_definitions(freeoriongodot
        PRIVATE
            -DFREEORION_BUILD_HUMAN_GODOT
    )

    target_include_directories(freeoriongodot SYSTEM
        PRIVATE
            ${GODOT_CPP_ROOT}/include
            ${GODOT_CPP_ROOT}/include/core
            ${GODOT_CPP_ROOT}/include/gen
            ${GODOT_CPP_ROOT}/godot_headers
    )

    if(CMAKE_VERSION VERSION_GREATER "3.13")
        target_link_directories(freeoriongodot
            PRIVATE
                ${GODOT_CPP_ROOT}/bin/
        )
        target_link_libraries(freeoriongodot
            godot-cpp.linux.$<IF:$<CONFIG:Debug>,debug,release>.$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,64,32>
        )
    else()
        target_link_libraries(freeoriongodot
            ${GODOT_CPP_ROOT}/bin/libgodot-cpp.linux.$<IF:$<CONFIG:Debug>,debug,release>.$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,64,32>.a
        )
    endif()

    target_link_libraries(freeoriongodot
        freeorioncommon
        freeorionparse
    )

    target_dependencies_copy_to_build(freeoriongodot)
endif()

if(WIN32 AND BUILD_CLIENT_GG)
    add_custom_command(TARGET freeorion POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_PREFIX_PATH}/bin/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.dll" "$<TARGET_FILE_DIR:freeorion>"
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_PREFIX_PATH}/bin/python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.zip" "$<TARGET_FILE_DIR:freeorion>" 
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
            ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
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
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_DATE_TIME_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_FILESYSTEM_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
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
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_SERIALIZATION_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_SYSTEM_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E copy_if_different "${Boost_THREAD_LIBRARY}" "$<TARGET_FILE_DIR:freeorion>/../SharedSupport"
        COMMAND
           ${CMAKE_COMMAND} -E remove_directory "$<TARGET_FILE_DIR:freeorion>/../Frameworks"
        COMMAND
           cp -R "${CMAKE_FRAMEWORK_PATH}" "$<TARGET_FILE_DIR:freeorion>/.."
    )
endif()


##
## Recurse into sources.
##

if(BUILD_AI OR BUILD_CLIENT_GG)
    add_subdirectory(client)
endif()
add_subdirectory(combat)
add_subdirectory(Empire)
if(BUILD_CLIENT_GODOT)
    add_subdirectory(godot)
endif()
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

if(UNIX AND BUILD_CLIENT_GG)
    install(
        TARGETS
	    freeorion
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        BUNDLE DESTINATION .
        RESOURCE
    )
endif()

if(UNIX AND NOT APPLE)
    install(
        TARGETS
            freeorioncommon
            freeorionparse
        LIBRARY DESTINATION ${FreeOrion_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    
    if(BUILD_SERVER)
        install(
            TARGETS
                freeoriond
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION .
        )
    endif()

    if(BUILD_AI)
        install(
            TARGETS
                freeorionca
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
            BUNDLE DESTINATION .
        )
    endif()

    if(BUILD_CLIENT_GG)
        install(
            FILES
            ${CMAKE_SOURCE_DIR}/packaging/org.freeorion.FreeOrion.desktop
            DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
        )
    endif()

    install(
        FILES
        ${CMAKE_SOURCE_DIR}/packaging/org.freeorion.FreeOrion.metainfo.xml
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/metainfo
    )

    if(BUILD_CLIENT_GG)
        install(
            DIRECTORY default/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/freeorion/default
            PATTERN "*~" EXCLUDE
            PATTERN "*.pyc" EXCLUDE
        )

        foreach(SIZE 16 24 32 64 128 256)
            install(
                FILES
                ${CMAKE_SOURCE_DIR}/default/data/art/icons/FO_Icon_${SIZE}x${SIZE}.png
                DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/${SIZE}x${SIZE}/apps/
                RENAME freeorion.png
            )
        endforeach()
    else()
        install(
            DIRECTORY default/
            DESTINATION ${CMAKE_INSTALL_DATADIR}/freeorion/default
            PATTERN "*~" EXCLUDE
            PATTERN "*.pyc" EXCLUDE
            PATTERN "*.png" EXCLUDE
            PATTERN "*.ogg" EXCLUDE
        )
    endif()
endif()

if(WIN32)
    install(
        FILES
        OpenAL32.dll
        boost_chrono.dll
        boost_date_time.dll
        boost_filesystem.dll
        boost_iostreams.dll
        boost_locale.dll
        boost_log.dll
        boost_log_setup.dll
        boost_python${Boost_PYTHON_SUFFIX}.dll
        boost_regex.dll
        boost_serialization.dll
        boost_system.dll
        boost_thread.dll
        glew32.dll
        libexpat.dll
        libogg.dll
        libpng13.dll
        libvorbis.dll
        libvorbisfile.dll
        python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}.dll
        wrap_oal.dll
        z.dll
        zlib1.dll
        DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
endif()

set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${FreeOrion_VERSION})
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
    set(PACKAGE_FILE_SYSTEM_NAME win32)
elseif(APPLE)
    set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME}_${CMAKE_OSX_DEPLOYMENT_TARGET})
else()
    set(PACKAGE_FILE_SYSTEM_NAME ${CPACK_SYSTEM_NAME})
endif()
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${PACKAGE_FILE_SYSTEM_NAME}")
set(CPACK_MONOLITHIC_INSTALL ON)

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
