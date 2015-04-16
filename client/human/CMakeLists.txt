cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorion)

message("-- Configuring freeorion")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)

if (NOT OPENGL_GLU_FOUND)
    message(FATAL_ERROR "OpenGL GLU library not found.")
endif ()

find_package(OpenAL REQUIRED)
# Use pkg config to find sdl2 details,
# since there is no official FindSDL2.cmake
INCLUDE(FindPkgConfig)
PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
find_package(Ogg REQUIRED)
find_package(Vorbis REQUIRED)

if(WIN32)
    find_package(GLEW REQUIRED)
endif()

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${OPENAL_INCLUDE_DIR}
    ${SDL2_INCLUDE_DIR}
    ${OGG_INCLUDE_DIR}
    ${VORBIS_INCLUDE_DIR}

    ${CMAKE_SOURCE_DIR}/PagedGeometry/include
)

if(WIN32)
    include_directories(${GLEW_INCLUDE_DIRS})
endif()

add_definitions(-DFREEORION_BUILD_HUMAN)


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
    ../../UI/AccordionPanel.h
    ../../UI/BuildDesignatorWnd.h
    ../../UI/BuildingsPanel.h
    ../../UI/CensusBrowseWnd.h
    ../../UI/ChatWnd.h
    ../../UI/ClientUI.h
    ../../UI/CombatReport/GraphicalSummary.h
    ../../UI/CombatReport/CombatLogWnd.h
    ../../UI/CombatReport/CombatReportData.h
    ../../UI/CombatReport/CombatReportWnd.h
    ../../UI/CUIControls.h
    ../../UI/CUIDrawUtil.h
    ../../UI/CUISlider.h
    ../../UI/CUISpin.h
    ../../UI/CUIStyle.h
    ../../UI/CUIWnd.h
    ../../UI/DesignWnd.h
    ../../UI/EncyclopediaDetailPanel.h
    ../../UI/Encyclopedia.h
    ../../UI/FieldIcon.h
    ../../UI/FleetButton.h
    ../../UI/FleetWnd.h
    ../../UI/GalaxySetupWnd.h
    ../../GG/GG/GLClientAndServerBuffer.h
    ../../UI/GraphControl.h
    ../../UI/Hotkeys.h
    ../../UI/IconTextBrowseWnd.h
    ../../UI/InGameMenu.h
    ../../UI/IntroScreen.h
    ../../UI/LinkText.h
    ../../UI/MapWnd.h
    ../../UI/MarkupBox.h
    ../../UI/MeterBrowseWnd.h
    ../../UI/MilitaryPanel.h
    ../../UI/ModeratorActionsWnd.h
    ../../UI/MultiIconValueIndicator.h
    ../../UI/MultiMeterStatusBar.h
    ../../UI/MultiplayerLobbyWnd.h
    ../../UI/ObjectListWnd.h
    ../../UI/OptionsWnd.h
    ../../UI/PlayerListWnd.h
    ../../UI/PopulationPanel.h
    ../../UI/ProductionWnd.h
    ../../UI/QueueListBox.h
    ../../UI/ResearchWnd.h
    ../../UI/ResourcePanel.h
    ../../UI/SaveFileDialog.h
    ../../UI/ServerConnectWnd.h
    ../../UI/ShaderProgram.h
    ../../UI/ShipDesignPanel.h
    ../../UI/SidePanel.h
    ../../UI/SitRepPanel.h
    ../../UI/Sound.h
    ../../UI/SpecialsPanel.h
    ../../UI/SystemIcon.h
    ../../UI/SystemResourceSummaryBrowseWnd.h
    ../../UI/TechTreeArcs.h
    ../../UI/TechTreeLayout.h
    ../../UI/TechTreeWnd.h
    ../../UI/TextBrowseWnd.h
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
    ../../UI/AccordionPanel.cpp
    ../../UI/BuildDesignatorWnd.cpp
    ../../UI/BuildingsPanel.cpp
    ../../UI/CensusBrowseWnd.cpp
    ../../UI/ChatWnd.cpp
    ../../UI/ClientUI.cpp
    ../../UI/CombatReport/GraphicalSummary.cpp
    ../../UI/CombatReport/CombatLogWnd.cpp
    ../../UI/CombatReport/CombatReportData.cpp
    ../../UI/CombatReport/CombatReportWnd.cpp
    ../../UI/CUIControls.cpp
    ../../UI/CUIDrawUtil.cpp
    ../../UI/CUIStyle.cpp
    ../../UI/CUIWnd.cpp
    ../../UI/DesignWnd.cpp
    ../../UI/EncyclopediaDetailPanel.cpp
    ../../UI/FieldIcon.cpp
    ../../UI/FleetButton.cpp
    ../../UI/FleetWnd.cpp
    ../../UI/GalaxySetupWnd.cpp
    ../../GG/src/GLClientAndServerBuffer.cpp
    ../../UI/GraphControl.cpp
    ../../UI/Hotkeys.cpp
    ../../UI/IconTextBrowseWnd.cpp
    ../../UI/InGameMenu.cpp
    ../../UI/IntroScreen.cpp
    ../../UI/LinkText.cpp
    ../../UI/MapWnd.cpp
    ../../UI/MarkupBox.cpp
    ../../UI/MeterBrowseWnd.cpp
    ../../UI/MilitaryPanel.cpp
    ../../UI/ModeratorActionsWnd.cpp
    ../../UI/MultiIconValueIndicator.cpp
    ../../UI/MultiMeterStatusBar.cpp
    ../../UI/MultiplayerLobbyWnd.cpp
    ../../UI/ObjectListWnd.cpp
    ../../UI/OptionsWnd.cpp
    ../../UI/PlayerListWnd.cpp
    ../../UI/PopulationPanel.cpp
    ../../UI/ProductionWnd.cpp
    ../../UI/QueueListBox.cpp
    ../../UI/ResearchWnd.cpp
    ../../UI/ResourcePanel.cpp
    ../../UI/SaveFileDialog.cpp
    ../../UI/ServerConnectWnd.cpp
    ../../UI/ShaderProgram.cpp
    ../../UI/ShipDesignPanel.cpp
    ../../UI/SidePanel.cpp
    ../../UI/SitRepPanel.cpp
    ../../UI/Sound.cpp
    ../../UI/SpecialsPanel.cpp
    ../../UI/SystemIcon.cpp
    ../../UI/SystemResourceSummaryBrowseWnd.cpp
    ../../UI/TechTreeArcs.cpp
    ../../UI/TechTreeLayout.cpp
    ../../UI/TechTreeWnd.cpp
    ../../UI/TextBrowseWnd.cpp
)

set(freeorion_LINK_LIBS
    freeorioncommon
    freeorionparse
    GiGi
    GiGiSDL
    ${OPENGL_gl_LIBRARY}
    ${OPENGL_glu_LIBRARY}
    ${GLEW_LIBRARIES}
    ${OPENAL_LIBRARY}
    ${Boost_LIBRARIES}
    ${SDL2_LIBRARIES}
    ${OGG_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${ZLIB_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
)

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
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
    RUNTIME DESTINATION bin
    COMPONENT COMPONENT_FREEORION
)

install(
    FILES
    DESTINATION share/freeorion
    COMPONENT COMPONENT_FREEORION
)

