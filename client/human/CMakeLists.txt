cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorion)

message("-- Configuring freeorion")

#list(APPEND MINIMUM_BOOST_COMPONENTS iostreams)
#find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS ${MINIMUM_BOOST_COMPONENTS})
#if (Boost_FOUND)
#    include_directories(${Boost_INCLUDE_DIRS})
#else ()
#    message(FATAL_ERROR "Boost libraries not found.")
#endif ()

find_package(GiGi)
if (GIGI_FOUND)
    include_directories(${GIGI_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "GiGi library not found.")
endif ()

#find_package(OpenGL)
#if (OPENGL_FOUND)
#    if (NOT OPENGL_GLU_FOUND)
#        message(FATAL_ERROR "OpenGL GLU library not found.")
#    endif ()
#    include_directories(${OPENGL_INCLUDE_DIR})
#else ()
#    message(FATAL_ERROR "OpenGL library not found.")
#endif ()

#find_package(Freetype)
#if (FREETYPE_FOUND)
#    include_directories(${FREETYPE_INCLUDE_DIRS})
#else ()
#    message(FATAL_ERROR "Freetype library not found.")
#endif ()

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

#find_package(OIS)
#if (OIS_FOUND)
#    include_directories(${OIS_INCLUDE_DIR})
#    link_directories(${OIS_LIB_DIR})
#else ()
#    message(FATAL_ERROR "OIS library not found.")
#endif ()

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

find_package(Graphviz)
if (GRAPHVIZ_FOUND)
    include_directories(${GRAPHVIZ_INCLUDE_DIR})
else ()
    message(FATAL_ERROR "Bullet library not found.")
endif ()

# TODO: Are these necessary to link against as well?
#circogen
#common
#dotgen
#fdpgen
#gd
#neatogen
#pack
#plugin
#twopigen

set(sources
    ../../client/ClientApp.cpp
    ../../client/ClientFSMEvents.cpp
    ../../client/human/HumanClientFSM.cpp
    ../../client/human/HumanClientApp.cpp
    ../../client/human/chmain.cpp
    ../../network/ClientNetworking.cpp
    ../../UI/About.cpp
    ../../UI/BuildDesignatorWnd.cpp
    ../../UI/ChatWnd.cpp
    ../../UI/ClientUI.cpp
    ../../UI/CUIControls.cpp
    ../../UI/CUIDrawUtil.cpp
    ../../UI/CUIStyle.cpp
    ../../UI/CUIWnd.cpp
    ../../UI/EncyclopediaDetailPanel.cpp
    ../../UI/FleetButton.cpp
    ../../UI/FleetWnd.cpp
    ../../UI/GalaxySetupWnd.cpp
    ../../UI/InGameMenu.cpp
    ../../UI/InfoPanels.cpp
    ../../UI/IntroScreen.cpp
    ../../UI/LinkText.cpp
    ../../UI/CollisionMeshConverter.cpp
    ../../UI/CombatWnd.cpp
    ../../UI/MapWnd.cpp
    ../../UI/MultiplayerLobbyWnd.cpp
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
    ../../universe/Universe.cpp
    ../../util/AppInterface.cpp
)

add_definitions(-DFREEORION_BUILD_HUMAN -DGL_GLEXT_PROTOTYPES)

add_executable(freeorion ${sources})

set_target_properties(
    freeorion
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CLEAN_DIRECT_OUTPUT 1
    LABELS "${PROJECT_NAME}"
)

target_link_libraries(
    freeorion
    freeorion_lib
    ${Boost_LIBRARIES}
    ${GIGI_GIGI_LIBRARY}
    ${GIGI_GIGIOGRE_LIBRARY}
    ${OPENAL_LIBRARY}
    ${ALUT_LIBRARY}
    ${OGRE_LIBRARIES}
    ${OGG_LIBRARY}
    ${VORBIS_LIBRARIES}
    ${BULLET_LIBRARIES}
    ${GRAPHVIZ_LIBRARIES}
)

#g++ -o freeorion -pthread combat/Combat.o combat/CombatOrder.o combat/OpenSteer/AsteroidBeltObstacle.o combat/OpenSteer/CombatFighter.o combat/OpenSteer/CombatObject.o combat/OpenSteer/CombatShip.o combat/OpenSteer/Missile.o combat/OpenSteer/Obstacle.o combat/OpenSteer/PathingEngine.o combat/OpenSteer/SimpleVehicle.o combat/OpenSteer/Vec3.o combat/OpenSteer/Vec3Utilities.o Empire/Empire.o Empire/EmpireManager.o Empire/ResourcePool.o network/Message.o network/MessageQueue.o network/Networking.o UI/StringTable.o universe/Building.o universe/Condition.o universe/ConditionParser1.o universe/ConditionParser2.o universe/ConditionParser.o universe/Effect.o universe/EffectParser.o universe/Enums.o universe/Fleet.o universe/Meter.o universe/ParserUtil.o universe/Planet.o universe/PopCenter.o universe/Predicates.o universe/ResourceCenter.o universe/Ship.o universe/ShipDesign.o universe/Special.o universe/System.o universe/Tech.o universe/TopLevelParsers.o universe/UniverseObject.o universe/ValueRef.o universe/ValueRefParser.o util/DataTable.o util/GZStream.o util/MultiplayerCommon.o util/OptionsDB.o util/Order.o util/OrderSet.o util/Process.o util/Random.o util/Serialize.o util/SitRepEntry.o util/VarText.o util/Version.o util/binreloc.o util/Directories.o util/XMLDoc.o log4cpp/src/Appender.o log4cpp/src/AppenderSkeleton.o log4cpp/src/BasicLayout.o log4cpp/src/Category.o log4cpp/src/CategoryStream.o log4cpp/src/Configurator.o log4cpp/src/FactoryParams.o log4cpp/src/FileAppender.o log4cpp/src/HierarchyMaintainer.o log4cpp/src/LayoutAppender.o log4cpp/src/Localtime.o log4cpp/src/LoggingEvent.o log4cpp/src/NDC.o log4cpp/src/PatternLayout.o log4cpp/src/Priority.o log4cpp/src/StringUtil.o log4cpp/src/TimeStamp.o log4cpp/src/PThreads.o client/ClientApp-human.o client/ClientFSMEvents-human.o client/human/HumanClientFSM-human.o client/human/HumanClientApp-human.o client/human/chmain-human.o network/ClientNetworking-human.o UI/About-human.o UI/BuildDesignatorWnd-human.o UI/ChatWnd-human.o UI/ClientUI-human.o UI/CUIControls-human.o UI/CUIDrawUtil-human.o UI/CUIStyle-human.o UI/CUIWnd-human.o UI/EncyclopediaDetailPanel-human.o UI/FleetButton-human.o UI/FleetWnd-human.o UI/GalaxySetupWnd-human.o UI/InGameMenu-human.o UI/InfoPanels-human.o UI/IntroScreen-human.o UI/LinkText-human.o UI/CollisionMeshConverter-human.o UI/CombatWnd-human.o UI/MapWnd-human.o UI/MultiplayerLobbyWnd-human.o UI/OptionsWnd-human.o UI/DesignWnd-human.o UI/ProductionWnd-human.o UI/QueueListBox-human.o UI/ResearchWnd-human.o UI/ServerConnectWnd-human.o UI/SidePanel-human.o UI/SitRepPanel-human.o UI/Sound-human.o UI/SystemIcon-human.o UI/TechTreeWnd-human.o UI/TurnProgressWnd-human.o UI/ShaderProgram-human.o UI/PagedGeometry/BatchedGeometry-human.o UI/PagedGeometry/BatchPage-human.o UI/PagedGeometry/GrassLoader-human.o UI/PagedGeometry/ImpostorPage-human.o UI/PagedGeometry/PagedGeometry-human.o UI/PagedGeometry/PropertyMaps-human.o UI/PagedGeometry/StaticBillboardSet-human.o UI/PagedGeometry/TreeLoader2D-human.o UI/PagedGeometry/TreeLoader3D-human.o UI/PagedGeometry/WindBatchedGeometry-human.o UI/PagedGeometry/WindBatchPage-human.o universe/Universe-human.o util/AppInterface-human.o -L/usr/lib -L/usr/local/lib -lGiGiOgre -lbulletdynamics -lbulletcollision -lbulletmath -lalut -lopenal -lvorbisfile -lgvc -lgraph -lcdt -lOgreMain -lGiGi -lboost_serialization
# -lboost_signals -lboost_system -lboost_filesystem -lboost_thread
# -lboost_python -lboost_iostreams -lpython2.6
# -lGL -lGLU -lfreetype -lz -ljpeg -lpng -ltiff
