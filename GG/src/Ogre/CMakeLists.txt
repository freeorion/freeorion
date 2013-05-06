cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgre)

if (USE_STATIC_LIBS)
    add_definitions(-DOGRE_STATIC_LIB)
endif ()

if (BUILD_OGRE_DRIVER)
    message("-- Configuring GiGiOgre")
    find_package(OGRE)
    if (OGRE_FOUND)
        include_directories(${OGRE_INCLUDE_DIR})
        link_directories(${OGRE_LIB_DIR})
    else ()
        set(BUILD_OGRE_DRIVER OFF)
        message("     Warning: Ogre could not be found.  Disabling the Ogre build.")
    endif ()
endif ()

if (BUILD_OGRE_DRIVER)
    set(GiGiOgre_LINK_LIBS GiGi ${OGRE_LIBRARIES})

    add_library(GiGiOgre OgreGUI.cpp)

    if (USE_STATIC_LIBS)
        set_target_properties(GiGiOgre
            PROPERTIES
            LINK_SEARCH_END_STATIC true
        )
    endif ()

    target_link_libraries(GiGiOgre ${GiGiOgre_LINK_LIBS})

    # Installation of this library variant
    if (((NOT BUILD_SHARED_LIBS AND NOT RUNTIME_ONLY_PACKAGE) OR
        (BUILD_SHARED_LIBS AND NOT DEVEL_ONLY_PACKAGE)) AND (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiOgre OR ${PACKAGE_TYPE} STREQUAL GiGiOgreDevel))
        install(
            TARGETS GiGiOgre
            LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGRE
            ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGRE_DEVEL
        )
    endif ()

    if (UNIX)
        get_pkg_config_libs(pkg_config_libs ${GiGiOgre_LINK_LIBS})
        configure_file(
            ${CMAKE_HOME_DIRECTORY}/cmake/GiGiOgre.pc.in
            ${CMAKE_BINARY_DIR}/GiGiOgre.pc
            @ONLY
        )
        if (NOT RUNTIME_ONLY_PACKAGE)
            if (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiOgre)
                install(
                FILES ${CMAKE_BINARY_DIR}/GiGiOgre.pc
                DESTINATION lib${LIB_SUFFIX}/pkgconfig
                COMPONENT COMPONENT_GIGI_DEVEL
                OPTIONAL
            )
            endif ()
        endif ()
    endif ()
endif ()

add_subdirectory(Plugins)
