cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgrePlugin_OIS)

if (BUILD_OGRE_OIS_PLUGIN)
    message("-- Configuring Ogre OIS Input Plugin")

    find_package(OIS REQUIRED)

    include_directories(${OIS_INCLUDE_DIRS})

    add_library(GiGiOgrePlugin_OIS
        OgreGUIInputPlugin.cpp
        OISInput.cpp
    )

    if (USE_STATIC_LIBS)
        set_target_properties(GiGiOgrePlugin_OIS
            PROPERTIES
            LINK_SEARCH_END_STATIC true
        )
    endif ()

    target_link_libraries(GiGiOgrePlugin_OIS
        GiGiOgre
        ${OIS_LIBRARIES}
    )

    # Installation of this library variant
    if (((NOT BUILD_SHARED_LIBS AND NOT RUNTIME_ONLY_PACKAGE) OR
        (BUILD_SHARED_LIBS AND NOT DEVEL_ONLY_PACKAGE)) AND (${PACKAGE_TYPE} STREQUAL NoPackage OR ${PACKAGE_TYPE} MATCHES Full OR ${PACKAGE_TYPE} MATCHES GiGiOgrePlugin_OIS OR ${PACKAGE_TYPE} STREQUAL GiGiOgrePlugin_OISDevel))
        install(
            TARGETS GiGiOgrePlugin_OIS
            LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGREPLUGIN_OIS
            ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGREPLUGIN_OIS_DEVEL
        )
    endif ()
endif ()
