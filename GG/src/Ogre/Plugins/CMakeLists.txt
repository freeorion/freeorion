cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgrePlugin_OIS)

if (BUILD_OGRE_OIS_PLUGIN)
    message("-- Configuring Ogre OIS Input Plugin")
    find_package(OIS)
    if (OIS_FOUND)
        include_directories(${OIS_INCLUDE_DIR})
        link_directories(${OIS_LIB_DIR})
    else ()
        set(BUILD_OGRE_OIS_PLUGIN OFF)
        message("     Warning: OIS could not be found.  Disabling the OIS build.")
    endif ()
endif ()

if (BUILD_OGRE_OIS_PLUGIN)
    set(THIS_LIB_SOURCES OgreGUIInputPlugin.cpp OISInput.cpp)
    set(THIS_LIB_LINK_LIBS GiGiOgre ${OIS_LIBRARIES})
    library_all_variants(GiGiOgrePlugin_OIS)

    if (WIN32)
        if (TARGET GiGiOgrePlugin_OIS)
            set_target_properties(GiGiOgrePlugin_OIS
                PROPERTIES
                COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
                LINK_FLAGS /NODEFAULTLIB:LIBCMT
            )
        elseif (TARGET GiGiOgrePlugin_OIS_static)
            set_target_properties(GiGiOgrePlugin_OIS_static
                PROPERTIES
                LINK_FLAGS /NODEFAULTLIB:LIBCMT
            )
        endif ()
    endif ()

    if (TARGET GiGiOgrePlugin_OIS_static)
        set_target_properties(GiGiOgrePlugin_OIS_static
            PROPERTIES
            COMPILE_DEFINITIONS OGRE_STATIC_LIB
        )
    endif ()
endif ()
