cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorion)

message("-- Configuring freeorion")

find_package(OpenGL REQUIRED)

if (NOT OPENGL_GLU_FOUND)
    message(FATAL_ERROR "OpenGL GLU library not found.")
endif ()

find_package(OpenAL REQUIRED)
find_package(OGRE REQUIRED)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)
find_package(Bullet REQUIRED)
find_package(OIS REQUIRED)
if(WIN32)
    find_package(GLEW REQUIRED)
endif()

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${OPENAL_INCLUDE_DIR}
    ${OGRE_INCLUDE_DIRS}
    ${OGG_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR}
    ${BULLET_INCLUDE_DIRS}
    ${OIS_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/PagedGeometry/include
)

if(WIN32)
    include_directories(${GLEW_INCLUDE_DIRS})
endif()

add_definitions(-DFREEORION_BUILD_HUMAN -DGL_GLEXT_PROTOTYPES)

link_directories(${BULLET_LIB_DIR})

if (CMAKE_COMPILER_IS_GNUCXX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
endif ()

set(freeorion_HEADER
    ../ClientApp.h
    ../ClientFSMEvents.h
    HumanClientApp.cpp
    HumanClientApp.h
    ../../network/ClientNetworking.h
    ../../UI/About.h
    ../../UI/BuildDesignatorWnd.h
    ../../UI/ChatWnd.h
    ../../UI/ClientUI.h
    ../../UI/CollisionMeshConverter.h
    ../../UI/CombatCamera.h
    ../../UI/CombatSetupWnd.h
    ../../UI/CombatWndFwd.h
    ../../UI/CombatWnd.h
    ../../UI/CUIControls.h
    ../../UI/CUIDrawUtil.h
    ../../UI/CUISlider.h
    ../../UI/CUISpin.h
    ../../UI/CUIStyle.h
    ../../UI/CUIWnd.h
    ../../UI/DesignWnd.h
    ../../UI/EncyclopediaDetailPanel.h
    ../../UI/Encyclopedia.h
    ../../UI/EntityRenderer.h
    ../../UI/FieldIcon.h
    ../../UI/FleetButton.h
    ../../UI/FleetWnd.h
    ../../UI/GalaxySetupWnd.h
    ../../UI/GLClientAndServerBuffer.h
    ../../UI/GraphControl.h
    ../../UI/Hotkeys.h
    ../../UI/InfoPanels.h
    ../../UI/InGameMenu.h
    ../../UI/IntroScreen.h
    ../../UI/LinkText.h
    ../../UI/MapWnd.h
    ../../UI/MarkupBox.h
    ../../UI/ModeratorActionsWnd.h
    ../../UI/MultiplayerLobbyWnd.h
    ../../UI/ObjectListWnd.h
    ../../UI/OptionsWnd.h
    ../../UI/PlayerListWnd.h
    ../../UI/ProductionWnd.h
    ../../UI/QueueListBox.h
    ../../UI/ResearchWnd.h
    ../../UI/ServerConnectWnd.h
    ../../UI/ShaderProgram.h
    ../../UI/SidePanel.h
    ../../UI/SitRepPanel.h
    ../../UI/Sound.h
    ../../UI/SystemIcon.h
    ../../UI/TechTreeLayout.h
    ../../UI/TechTreeWnd.h
    ../../UI/TurnProgressWnd.h
)

set(freeorion_SOURCES
    chmain.cpp
    ../ClientApp.cpp
    ../ClientFSMEvents.cpp
    HumanClientApp.cpp
    HumanClientFSM.cpp
    ../../combat/CombatSystem.cpp
    ../../network/ClientNetworking.cpp
    ../../UI/About.cpp
    ../../UI/BuildDesignatorWnd.cpp
    ../../UI/ChatWnd.cpp
    ../../UI/ClientUI.cpp
    ../../UI/CollisionMeshConverter.cpp
    ../../UI/CombatCamera.cpp
    ../../UI/CombatSetupWnd.cpp
    ../../UI/CombatWnd.cpp
    ../../UI/CUIControls.cpp
    ../../UI/CUIDrawUtil.cpp
    ../../UI/CUIStyle.cpp
    ../../UI/CUIWnd.cpp
    ../../UI/DesignWnd.cpp
    ../../UI/EncyclopediaDetailPanel.cpp
    ../../UI/EntityRenderer.cpp
    ../../UI/FieldIcon.cpp
    ../../UI/FleetButton.cpp
    ../../UI/FleetWnd.cpp
    ../../UI/GalaxySetupWnd.cpp
    ../../UI/GLClientAndServerBuffer.cpp
    ../../UI/GraphControl.cpp
    ../../UI/Hotkeys.cpp
    ../../UI/InfoPanels.cpp
    ../../UI/InGameMenu.cpp
    ../../UI/IntroScreen.cpp
    ../../UI/LinkText.cpp
    ../../UI/MapWnd.cpp
    ../../UI/ModeratorActionsWnd.cpp
    ../../UI/MultiplayerLobbyWnd.cpp
    ../../UI/ObjectListWnd.cpp
    ../../UI/OptionsWnd.cpp
    ../../UI/PlayerListWnd.cpp
    ../../UI/ProductionWnd.cpp
    ../../UI/QueueListBox.cpp
    ../../UI/ResearchWnd.cpp
    ../../UI/ServerConnectWnd.cpp
    ../../UI/ShaderProgram.cpp
    ../../UI/SidePanel.cpp
    ../../UI/SitRepPanel.cpp
    ../../UI/Sound.cpp
    ../../UI/SystemIcon.cpp
    ../../UI/TechTreeLayout.cpp
    ../../UI/TechTreeWnd.cpp
    ../../UI/TurnProgressWnd.cpp
)

set(freeorion_LINK_LIBS
    freeorioncommon
    freeorionparse
    pagedgeometry
    GiGi
    GiGiOgre
    GiGiOgrePlugin_OIS
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${OPENAL_LIBRARY}
    ${Boost_LIBRARIES}
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
        ${GLEW_LIBRARIES}
    )
    list(APPEND freeorion_SOURCES ${CMAKE_BINARY_DIR}/win32_resources.rc)
endif ()

add_executable(freeorion
    ${freeorion_HEADER}
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

