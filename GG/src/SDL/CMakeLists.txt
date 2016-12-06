project(GiGiSDL)
message("-- Configuring GiGiSDL")

set(SDL_BUILDING_LIBRARY true)

find_package(SDL2 REQUIRED)

include_directories(
    SYSTEM
    ${SDL2_INCLUDE_DIR}
)

set(GiGiSDL_LINK_LIBS GiGi ${SDL2_LIBRARY})
add_library(GiGiSDL SDLGUI.cpp)

if (USE_STATIC_LIBS)
    set_target_properties(GiGiSDL
        PROPERTIES
        LINK_SEARCH_END_STATIC true
    )
endif ()

target_link_libraries(GiGiSDL ${GiGiSDL_LINK_LIBS})

install(
    TARGETS GiGiSDL
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT COMPONENT_GIGISDL
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT COMPONENT_GIGISDL
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT COMPONENT_GIGISDL_DEVEL
)

if (UNIX AND BUILD_DEVEL_PACKAGE)
    get_pkg_config_libs(pkg_config_libs ${GiGiSDL_LINK_LIBS})

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/GiGiSDL.pc.in
        ${CMAKE_BINARY_DIR}/GiGiSDL.pc
        @ONLY
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/GiGiSDL.pc
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
        COMPONENT COMPONENT_GIGI_DEVEL
        OPTIONAL
    )
endif ()
