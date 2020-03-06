add_subdirectory(parse)
add_subdirectory(util)
add_subdirectory(system)

if (NOT BUILD_HEADLESS)
    add_subdirectory(UI)
endif ()
