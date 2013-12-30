cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(GiGiOgrePlugin_OIS)
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

install(
    TARGETS GiGiOgrePlugin_OIS
    LIBRARY DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGREPLUGIN_OIS
)

if (BUILD_DEVEL_PACKAGE)
    install(
        TARGETS GiGiOgrePlugin_OIS
        ARCHIVE DESTINATION lib${LIB_SUFFIX} COMPONENT COMPONENT_GIGIOGREPLUGIN_OIS_DEVEL
    )
endif ()
