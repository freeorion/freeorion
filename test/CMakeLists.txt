add_subdirectory(parse)
add_subdirectory(util)
if (BUILD_AI AND BUILD_SERVER)
    add_subdirectory(system)
endif ()

if (BUILD_CLIENT_GG)
    add_subdirectory(UI)
endif ()
