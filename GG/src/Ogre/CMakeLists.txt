cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgre)
message("-- Configuring GiGiOgre")

if (USE_STATIC_LIBS)
    add_definitions(-DOGRE_STATIC_LIB)
endif ()

find_package(OGRE REQUIRED)

include_directories(${OGRE_INCLUDE_DIR})
link_directories(${OGRE_LIB_DIR})

set(GiGiOgre_LINK_LIBS GiGi ${OGRE_LIBRARIES})

add_library(GiGiOgre OgreGUI.cpp)

if (USE_STATIC_LIBS)
    set_target_properties(GiGiOgre
        PROPERTIES
        LINK_SEARCH_END_STATIC true
    )
endif ()

target_link_libraries(GiGiOgre ${GiGiOgre_LINK_LIBS})

install(
    TARGETS GiGiOgre
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGRE
)

if (BUILD_DEVEL_PACKAGE)
    install(
        TARGETS GiGiOgre
        ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGRE_DEVEL
    )
endif ()

if (UNIX AND BUILD_DEVEL_PACKAGE)
    get_pkg_config_libs(pkg_config_libs ${GiGiOgre_LINK_LIBS})

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/GiGiOgre.pc.in
        ${CMAKE_BINARY_DIR}/GiGiOgre.pc
        @ONLY
    )

    install(
        FILES ${CMAKE_BINARY_DIR}/GiGiOgre.pc
        DESTINATION lib${LIB_SUFFIX}/pkgconfig
        COMPONENT COMPONENT_GIGI_DEVEL
        OPTIONAL
    )
endif ()

if (BUILD_OGRE_OIS_PLUGIN)
    add_subdirectory(Plugins)
endif ()
