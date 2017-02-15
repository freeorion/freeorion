find_program(CPPCHECK_EXECUTABLE
    NAMES cppcheck
    DOC "A tool for static C/C++ code analysis"
)

if(CPPCHECK_EXECUTABLE)
    execute_process(COMMAND ${CPPCHECK_EXECUTABLE} "--version" OUTPUT_VARIABLE CPPCHECK_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "Cppcheck ([0-9]+).([0-9]+).*" "\\1.\\2" CPPCHECK_VERSION ${CPPCHECK_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cppcheck REQUIRED_VARS CPPCHECK_EXECUTABLE VERSION_VAR CPPCHECK_VERSION)

mark_as_advanced(
    CPPCHECK_EXECUTABLE
)
