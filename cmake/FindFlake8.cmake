find_program(FLAKE8_EXECUTABLE
    NAMES flake8
    DOC "Python tool for Style Guide Enforcement"
)

if(FLAKE8_EXECUTABLE)
    execute_process(COMMAND ${FLAKE8_EXECUTABLE} "--version" OUTPUT_VARIABLE FLAKE8_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REPLACE " " ";" FLAKE8_VERSION ${FLAKE8_VERSION})
    list(GET FLAKE8_VERSION 0 FLAKE8_VERSION)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Flake8 REQUIRED_VARS FLAKE8_EXECUTABLE VERSION_VAR FLAKE8_VERSION)

mark_as_advanced(
    FLAKE8_EXECUTABLE
)

if(FLAKE8_FOUND)
    if(NOT TARGET Flake8::flake8)
        add_executable(Flake8::flake8 IMPORTED)
        set_property(TARGET Flake8::flake8 PROPERTY IMPORTED_LOCATION ${FLAKE8_EXECUTABLE})
    endif()
endif()
