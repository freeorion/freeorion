#.rst:
# TargetDependencies
# ------------------
#
# Functions to enable execution of targets without RPATH support.
#
# This module provides functions to enable the execution of target binaries
# where third party libraries search pathes cannot be provided with RPATH
# support.  This module depdends on :module:`GetPrerequisites` to work.
#
# The following functions are provided by this module:
#
# ::
#
#    TARGET_DEPENDENCIES_ADD_SEARCH_PATH(<path>)
#
# Add <path> to the internal list of directories, where dependent shared
# libraries should be searched in.
#
# ::
#
#    TARGET_DEPENDENCIES_COPY_TO_BUILD(<target>)
#
# Copy the dependent shared libraries of <target>s main binary into <target>s
# build destination directory after a successful build.
#
# ::
#
#    TARGET_DEPENDENT_DATA_SYMLINK_TO_BUILD(<target> <path>)
#
# Create a symbolic link to <path> into <target>s build destination directory
# after a successful build.  Only supports directories as <path>.

set_property(GLOBAL PROPERTY _TARGET_DEPENDENCIES_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")

function(TARGET_DEPENDENCIES_ADD_SEARCH_PATH SEARCH_PATH)
    get_property(PATH_LIST GLOBAL PROPERTY _TARGET_DEPENDENCIES_SEARCH_PATH)
    list(APPEND PATH_LIST "${SEARCH_PATH}")
    list(REMOVE_DUPLICATES PATH_LIST)
    set_property(GLOBAL PROPERTY _TARGET_DEPENDENCIES_SEARCH_PATH "${PATH_LIST}")
endfunction()

function(TARGET_DEPENDENCIES_COPY_TO_BUILD TARGET)
    if(NOT WIN32)
        return()
    endif()

    get_property(SCRIPT GLOBAL PROPERTY _TARGET_DEPENDENCIES_SCRIPT)
    get_property(SEARCH_PATH GLOBAL PROPERTY _TARGET_DEPENDENCIES_SEARCH_PATH)

    add_custom_command(TARGET ${TARGET}
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}"
            -DMODE=DYNAMIC_LIB_COPY
            "-DBINARY=$<TARGET_FILE:${TARGET}>"
            "-DDESTINATION=$<TARGET_FILE_DIR:${TARGET}>"
            "-DSEARCH_PATH=${SEARCH_PATH}"
            -P "${SCRIPT}"
    )
endfunction()

function(TARGET_DEPENDENT_DATA_SYMLINK_TO_BUILD TARGET SOURCE_PATH)
    if(APPLE)
        return()
    endif()

    if(WIN32)
        if(CMAKE_VERSION VERSION_LESS "3.4")
            message(FATAL_ERROR "`target_dependent_data_symlink_to_build`: Requires at least CMake 3.4 to work properly.")
        endif()

        # mklink is a builtin in cmd.exe of Win Vista an above
        if(CMAKE_SYSTEM_VERSION VERSION_LESS "6.0")
            message(FATAL_ERROR "`target_dependent_data_symlink_to_build`: Requires `mklink.exe` to work properly (available on Vista or later).")
        endif()

        if(NOT IS_DIRECTORY "${SOURCE_PATH}")
            message(FATAL_ERROR "`target_dependent_data_symlink_to_build`: Source path must point to a directory.")
        endif()
    endif()

    get_property(SCRIPT GLOBAL PROPERTY _TARGET_DEPENDENCIES_SCRIPT)

    add_custom_command(TARGET ${TARGET}
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}"
            -DMODE=DATA_LINK
            "-DDESTINATION=$<TARGET_FILE_DIR:${TARGET}>"
            "-DSOURCE_PATH=${SOURCE_PATH}"
            -P "${SCRIPT}"
    )
endfunction()

if(CMAKE_SCRIPT_MODE_FILE AND (MODE STREQUAL "DYNAMIC_LIB_COPY"))
    include(GetPrerequisites)

    get_filename_component(BINARY_PATH "${BINARY}" PATH)
    list(APPEND SEARCH_PATH "${BINARY_PATH}")

    get_prerequisites("${BINARY}" DEPENDENCIES 1 1 "" "${SEARCH_PATH}")

    foreach(DEPENDENCY ${DEPENDENCIES})
        gp_resolve_item("${BINARY}" "${DEPENDENCY}" "" "${SEARCH_PATH}" DEPENDENCY_FILE)
        if(EXISTS "${DEPENDENCY_FILE}")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEPENDENCY_FILE}" "${DESTINATION}")
        endif()
    endforeach()
endif()

if(CMAKE_SCRIPT_MODE_FILE AND (MODE STREQUAL "DATA_LINK"))
    if(NOT IS_DIRECTORY "${DESTINATION}")
        message(FATAL_ERROR "`target_dependent_data_symlink_to_build`: Destination is not a directory.")
    endif()

    get_filename_component(TARGET_NAME "${SOURCE_PATH}" NAME)

    if("${DESTINATION}/${TARGET_NAME}" STREQUAL "${SOURCE_PATH}")
        return()
    elseif(IS_SYMLINK "${DESTINATION}/${TARGET_NAME}")
        return()
    elseif(EXISTS "${DESTINATION}/${TARGET_NAME}")
        message(FATAL_ERROR "`target_dependent_data_symlink_to_build`: Target path exists and is not a symlink.")
    endif()

    if(WIN32)
        execute_process(COMMAND mklink /J "${DESTINATION}" "${SOURCE_PATH}")
    else()
        execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink "${SOURCE_PATH}" "${DESTINATION}/${TARGET_NAME}")
    endif()
endif()
