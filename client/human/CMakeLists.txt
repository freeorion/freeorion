cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorion)

message("-- Configuring freeorion")

if (WIN32)
   find_package(OpenGL)
   if (OPENGL_FOUND)
       if (NOT OPENGL_GLU_FOUND)
           message(FATAL_ERROR "OpenGL GLU library not found.")
       endif ()
       include_directories(${OPENGL_INCLUDE_DIR})
   else ()
       message(FATAL_ERROR "OpenGL library not found.")
   endif ()
endif ()

find_package(OpenAL)
if (OPENAL_FOUND)
    include_directories(${OPENAL_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "OpenAL library not found.")
endif ()

find_package(ALUT)
if (ALUT_FOUND)
    include_directories(${ALUT_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "ALUT OpenAL utility library not found.")
endif ()

find_package(OGRE)
if (OGRE_FOUND)
    include_directories(${OGRE_INCLUDE_DIR})
    link_directories(${OGRE_LIB_DIR})
else ()
    message(FATAL_ERROR "Ogre library not found.")
endif ()

find_package(Ogg)
if (OGG_FOUND)
    include_directories(${OGG_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "Ogg library not found.")
endif ()

find_package(Vorbis)
if (VORBIS_FOUND)
    include_directories(${VORBIS_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "Vorbis library not found.")
endif ()

find_package(Bullet)
if (BULLET_FOUND)
    include_directories(${BULLET_INCLUDE_DIR})
    link_directories(${BULLET_LIB_DIR})
else ()
    message(FATAL_ERROR "Bullet library not found.")
endif ()

set(THIS_EXE_SOURCES
    ../../client/ClientApp.cpp
    ../../client/ClientFSMEvents.cpp
    ../../client/human/HumanClientFSM.cpp
    ../../client/human/HumanClientApp.cpp
    ../../client/human/chmain.cpp
    ../../combat/CombatSystem.cpp
    ../../network/ClientNetworking.cpp
    ../../UI/About.cpp
    ../../UI/BuildDesignatorWnd.cpp
    ../../UI/ChatWnd.cpp
    ../../UI/PlayerListWnd.cpp
    ../../UI/ClientUI.cpp
    ../../UI/CollisionMeshConverter.cpp
    ../../UI/CombatCamera.cpp
    ../../UI/CombatWnd.cpp
    ../../UI/CombatSetupWnd.cpp
    ../../UI/CUIControls.cpp
    ../../UI/CUIDrawUtil.cpp
    ../../UI/CUIStyle.cpp
    ../../UI/CUIWnd.cpp
    ../../UI/EncyclopediaDetailPanel.cpp
    ../../UI/EntityRenderer.cpp
    ../../UI/FleetButton.cpp
    ../../UI/FleetWnd.cpp
    ../../UI/GalaxySetupWnd.cpp
    ../../UI/InGameMenu.cpp
    ../../UI/InfoPanels.cpp
    ../../UI/IntroScreen.cpp
    ../../UI/LinkText.cpp
    ../../UI/MapWnd.cpp
    ../../UI/GLClientAndServerBuffer.cpp
    ../../UI/MultiplayerLobbyWnd.cpp
    ../../UI/ObjectListWnd.cpp
    ../../UI/OptionsWnd.cpp
    ../../UI/DesignWnd.cpp
    ../../UI/ProductionWnd.cpp
    ../../UI/QueueListBox.cpp
    ../../UI/ResearchWnd.cpp
    ../../UI/ServerConnectWnd.cpp
    ../../UI/SidePanel.cpp
    ../../UI/SitRepPanel.cpp
    ../../UI/Sound.cpp
    ../../UI/StringTable.cpp
    ../../UI/SystemIcon.cpp
    ../../UI/TechTreeWnd.cpp
    ../../UI/TechTreeLayout.cpp
    ../../UI/TurnProgressWnd.cpp
    ../../UI/ShaderProgram.cpp
    ../../UI/PagedGeometry/BatchedGeometry.cpp
    ../../UI/PagedGeometry/BatchPage.cpp
    ../../UI/PagedGeometry/GrassLoader.cpp
    ../../UI/PagedGeometry/ImpostorPage.cpp
    ../../UI/PagedGeometry/PagedGeometry.cpp
    ../../UI/PagedGeometry/PropertyMaps.cpp
    ../../UI/PagedGeometry/StaticBillboardSet.cpp
    ../../UI/PagedGeometry/TreeLoader2D.cpp
    ../../UI/PagedGeometry/TreeLoader3D.cpp
    ../../UI/PagedGeometry/WindBatchedGeometry.cpp
    ../../UI/PagedGeometry/WindBatchPage.cpp
    ../../util/AppInterface.cpp
    ../../util/VarText.cpp
)

add_definitions(-DFREEORION_BUILD_HUMAN -DGL_GLEXT_PROTOTYPES)

set(THIS_EXE_LINK_LIBS
    core_static
    parse_static
    ${GIGI_GIGI_LIBRARY}
    ${GIGI_GIGIOGRE_LIBRARY}
    ${OPENAL_LIBRARY}
    ${ALUT_LIBRARY}
    ${OGRE_LIBRARIES}
    ${OGG_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${ZLIB_LIBRARY}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
    set(THIS_EXE_LINK_LIBS
        ${THIS_EXE_LINK_LIBS}
        ${OPENGL_gl_LIBRARY}
        ${OPENGL_glu_LIBRARY}
        jpeg.lib libpng.lib glew32.lib
    )
    set(THIS_EXE_SOURCES ${THIS_EXE_SOURCES} ${CMAKE_BINARY_DIR}/win32_resources.rc)
else ()
    set(THIS_EXE_LINK_LIBS ${THIS_EXE_LINK_LIBS} ${Boost_LIBRARIES})
endif ()

executable_all_variants(freeorion)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
    set_target_properties(freeorion
        PROPERTIES
        COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
        LINK_FLAGS /NODEFAULTLIB:LIBCMT
    )
endif ()
