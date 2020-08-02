find_program(CLANGFORMAT_EXECUTABLE
    NAMES clang-format
    DOC "A tool for C/C++ code formatting"
)

if(CLANG-FORMAT_EXECUTABLE)
    execute_process(COMMAND ${CLANGFORMAT_EXECUTABLE} "-version" OUTPUT_VARIABLE CLANGFORMAT_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX REPLACE "clang-format version ([0-9]+)\\.([0-9]+)\\.([0-9]+).*" "\\1.\\2.\\3" CLANGFORMAT_VERSION ${CLANGFORMAT_VERSION})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ClangFormat REQUIRED_VARS CLANGFORMAT_EXECUTABLE VERSION_VAR CLANGFORMAT_VERSION)

mark_as_advanced(
    CLANGFORMAT_EXECUTABLE
)

if(CLANGFORMAT_FOUND)
    if(NOT TARGET ClangFormat::clang-format)
        add_executable(ClangFormat::clang-format IMPORTED)
        set_property(TARGET ClangFormat::clang-format PROPERTY IMPORTED_LOCATION ${CLANGFORMAT_EXECUTABLE})
    endif()
endif()
