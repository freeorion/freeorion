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
    set(GiGiSDL_LINK_LIBS GiGi ${SDL_LIBRARY})
    add_library(GiGiSDL SDLGUI.cpp)

    if (USE_STATIC_LIBS)
        set_target_properties(GiGiSDL
            PROPERTIES
            LINK_SEARCH_END_STATIC true
        )
    endif ()

    target_link_libraries(GiGiSDL ${GiGiSDL_LINK_LIBS})

    # Installation of this library variant
    if (((NOT BUILD_SHARED_LIBS AND NOT RUNTIME_ONLY_PACKAGE) OR
       (BUILD_SHARED_LIBS AND NOT DEVEL_ONLY_PACKAGE)) AND (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiSDL OR ${PACKAGE_TYPE} STREQUAL GiGiSDLDevel))
        install(
            TARGETS GiGiSDL
            LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGISDL
            ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGISDL_DEVEL
        )
    endif ()

    if (UNIX)
        get_pkg_config_libs(pkg_config_libs ${GiGiSDL_LINK_LIBS})
        configure_file(
            ${CMAKE_HOME_DIRECTORY}/cmake/GiGiSDL.pc.in
            ${CMAKE_BINARY_DIR}/GiGiSDL.pc
            @ONLY
        )
        if (NOT RUNTIME_ONLY_PACKAGE)
            if (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiSDL)
                install(
                FILES ${CMAKE_BINARY_DIR}/GiGiSDL.pc
                DESTINATION lib${LIB_SUFFIX}/pkgconfig
                COMPONENT COMPONENT_GIGI_DEVEL
                OPTIONAL
            )
            endif ()
        endif ()
    endif ()
endif ()
