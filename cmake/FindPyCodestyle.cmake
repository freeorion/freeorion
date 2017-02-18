find_program(PYCODESTYLE_EXECUTABLE
    NAMES pycodestyle pep8 # Old name
    DOC "Python style guide checker"
)

if(PYCODESTYLE_EXECUTABLE)
    execute_process(COMMAND ${PYCODESTYLE_EXECUTABLE} "--version" OUTPUT_VARIABLE PYCODESTYLE_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pycodestyle REQUIRED_VARS PYCODESTYLE_EXECUTABLE VERSION_VAR PYCODESTYLE_VERSION)

mark_as_advanced(
    PYCODESTYLE_EXECUTABLE
)
