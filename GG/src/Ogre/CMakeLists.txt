cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgre)

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
    set(THIS_LIB_SOURCES OgreGUI.cpp)
    set(THIS_LIB_LINK_LIBS GiGi ${OGRE_LIBRARIES})
    library_all_variants(GiGiOgre)

    if (WIN32)
        if (TARGET GiGiOgre)
            set_target_properties(GiGiOgre
                PROPERTIES
                COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
                LINK_FLAGS /NODEFAULTLIB:LIBCMT
            )
        elseif (TARGET GiGiOgre_static)
            set_target_properties(GiGiOgre_static
                PROPERTIES
                LINK_FLAGS /NODEFAULTLIB:LIBCMT
            )
        endif ()
    endif ()

    if (TARGET GiGiOgre_static)
        set_target_properties(GiGiOgre_static
            PROPERTIES
            COMPILE_DEFINITIONS OGRE_STATIC_LIB
        )
    endif ()

    if (UNIX)
        get_pkg_config_libs(pkg_config_libs ${THIS_LIB_LINK_LIBS})
        configure_file(
            ${CMAKE_HOME_DIRECTORY}/cmake/GiGiOgre.pc.in
            ${CMAKE_BINARY_DIR}/GiGiOgre.pc
            @ONLY
        )
    endif ()
endif ()

add_subdirectory(Plugins)
