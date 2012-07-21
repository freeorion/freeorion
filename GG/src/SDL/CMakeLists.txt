cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiSDL)

if (BUILD_SDL_DRIVER)
    message("-- Configuring GiGiSDL")
    set(SDL_BUILDING_LIBRARY true)
    find_package(SDL)
    if (SDL_FOUND)
        include_directories(${SDL_INCLUDE_DIR})
    else ()
        set(BUILD_SDL_DRIVER OFF)
        message("     Warning: SDL could not be found.  Disabling the SDL build.")
    endif ()
endif ()

if (BUILD_SDL_DRIVER)
    set(THIS_LIB_SOURCES SDLGUI.cpp)
    set(THIS_LIB_LINK_LIBS GiGi ${SDL_LIBRARY})
    library_all_variants(GiGiSDL)

    if (WIN32)
        if (TARGET GiGiSDL)
            set_target_properties(GiGiSDL
                PROPERTIES
                COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
                LINK_FLAGS /NODEFAULTLIB:LIBCMT
            )
        elseif (TARGET GiGiSDL_static)
            set_target_properties(GiGiSDL_static
                PROPERTIES
                LINK_FLAGS /NODEFAULTLIB:LIBCMT
            )
        endif ()
    endif ()

    if (UNIX)
        get_pkg_config_libs(pkg_config_libs ${THIS_LIB_LINK_LIBS})
        configure_file(
            ${CMAKE_HOME_DIRECTORY}/cmake/GiGiSDL.pc.in
            ${CMAKE_BINARY_DIR}/GiGiSDL.pc
            @ONLY
        )
    endif ()
endif ()
