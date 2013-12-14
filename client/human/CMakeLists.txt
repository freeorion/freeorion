cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorion)

message("-- Configuring freeorion")

find_package(OpenGL REQUIRED)

if (NOT OPENGL_GLU_FOUND)
    message(FATAL_ERROR "OpenGL GLU library not found.")
endif ()

find_package(OpenAL REQUIRED)
find_package(ALUT REQUIRED)
find_package(OGRE REQUIRED)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Bullet REQUIRED)

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${ALUT_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIRS}
    ${OGG_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR}
    ${BULLET_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/PagedGeometry/include
)

add_definitions(-DFREEORION_BUILD_HUMAN -DGL_GLEXT_PROTOTYPES)

link_directories(${BULLET_LIB_DIR})

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()

set(freeorion_SOURCES
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
    ../../UI/FieldIcon.cpp
    ../../UI/FleetButton.cpp
    ../../UI/FleetWnd.cpp
    ../../UI/GalaxySetupWnd.cpp
    ../../UI/GraphControl.cpp
    ../../UI/InGameMenu.cpp
    ../../UI/InfoPanels.cpp
    ../../UI/IntroScreen.cpp
    ../../UI/LinkText.cpp
    ../../UI/MapWnd.cpp
    ../../UI/ModeratorActionsWnd.cpp
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
    ../../UI/SystemIcon.cpp
    ../../UI/TechTreeWnd.cpp
    ../../UI/TechTreeLayout.cpp
    ../../UI/TurnProgressWnd.cpp
    ../../UI/ShaderProgram.cpp
    ../../UI/Hotkeys.cpp
)

set(freeorion_LINK_LIBS
    freeorioncommon
    freeorionparse
    pagedgeometry
    GiGi
    GiGiOgre
    GiGiOgrePlugin_OIS
    ${OPENAL_LIBRARY}
    ${ALUT_LIBRARY}
    ${OGRE_LIBRARIES}
    ${OGG_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
    list(APPEND
        freeorion_LINK_LIBS
        ${OPENGL_gl_LIBRARY}
        ${OPENGL_glu_LIBRARY}
        jpeg.lib libpng.lib glew32.lib
    )
    list(APPEND freeorion_SOURCES ${CMAKE_BINARY_DIR}/win32_resources.rc)
else ()
    list(APPEND freeorion_LINK_LIBS ${Boost_LIBRARIES})
endif ()

add_executable(freeorion
    ${freeorion_SOURCES}
)

target_link_libraries(freeorion
    ${freeorion_LINK_LIBS}
)

install(
    TARGETS freeorion
    RUNTIME DESTINATION .
    COMPONENT COMPONENT_FREEORION
)

