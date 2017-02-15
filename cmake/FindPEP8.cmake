find_program(PEP8_EXECUTABLE
    NAMES pep8
    DOC "Python style guide checker"
)

if(PEP8_EXECUTABLE)
    execute_process(COMMAND ${PEP8_EXECUTABLE} "--version" OUTPUT_VARIABLE PEP8_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE ".*Blender ([0-9]+).([0-9]+) \\(sub ([0-9]+)\\).*" "\\1.\\2.\\3" PEP8_VERSION ${PEP8_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(pep8 REQUIRED_VARS PEP8_EXECUTABLE VERSION_VAR PEP8_VERSION)

mark_as_advanced(
    PEP8_EXECUTABLE
)
