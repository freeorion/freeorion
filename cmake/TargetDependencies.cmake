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

set_property(GLOBAL PROPERTY _TARGET_DEPENDENCIES_SCRIPT "${CMAKE_CURRENT_LIST_FILE}")

function (target_dependencies_add_search_path SEARCH_PATH)
    get_property(PATH_LIST GLOBAL PROPERTY _TARGET_DEPENDENCIES_SEARCH_PATH)
    list(APPEND PATH_LIST "${SEARCH_PATH}")
    list(REMOVE_DUPLICATES PATH_LIST)
    set_property(GLOBAL PROPERTY _TARGET_DEPENDENCIES_SEARCH_PATH "${PATH_LIST}")
endfunction ()

function (target_dependencies_copy_to_build TARGET)
    if (NOT WIN32)
        return ()
    endif ()

    get_property(SCRIPT GLOBAL PROPERTY _TARGET_DEPENDENCIES_SCRIPT)
    get_property(SEARCH_PATH GLOBAL PROPERTY _TARGET_DEPENDENCIES_SEARCH_PATH)

    add_custom_command(TARGET ${TARGET}
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}"
            "-DBINARY=$<TARGET_FILE:${TARGET}>"
            "-DDESTINATION=$<TARGET_FILE_DIR:${TARGET}>"
            "-DSEARCH_PATH=${SEARCH_PATH}"
            -P "${SCRIPT}"
    )
endfunction ()

if (CMAKE_SCRIPT_MODE_FILE)
    include(GetPrerequisites)

    get_filename_component(BINARY_PATH "${BINARY}" PATH)
    list(APPEND SEARCH_PATH "${BINARY_PATH}")

    get_prerequisites("${BINARY}" DEPENDENCIES 1 1 "" "${SEARCH_PATH}")

    foreach (DEPENDENCY ${DEPENDENCIES})
        gp_resolve_item("${BINARY}" "${DEPENDENCY}" "" "${SEARCH_PATH}" DEPENDENCY_FILE)
        if (EXISTS "${DEPENDENCY_FILE}")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${DEPENDENCY_FILE}" "${DESTINATION}")
        endif ()
    endforeach ()
endif ()
