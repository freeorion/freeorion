cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeorionca)

message("-- Configuring freeorionca")

set(THIS_EXE_SOURCES
    ../../combat/CombatOrder.cpp
    ../../combat/OpenSteer/AsteroidBeltObstacle.cpp
    ../../combat/OpenSteer/CombatFighter.cpp
    ../../combat/OpenSteer/CombatObject.cpp
    ../../combat/OpenSteer/CombatShip.cpp
    ../../combat/OpenSteer/Missile.cpp
    ../../combat/OpenSteer/Obstacle.cpp
    ../../combat/OpenSteer/PathingEngine.cpp
    ../../combat/OpenSteer/SimpleVehicle.cpp
    ../../combat/OpenSteer/Vec3.cpp
    ../../combat/OpenSteer/Vec3Utilities.cpp
    ../../Empire/Empire.cpp
    ../../Empire/EmpireManager.cpp
    ../../Empire/ResourcePool.cpp
    ../../network/Message.cpp
    ../../network/MessageQueue.cpp
    ../../network/Networking.cpp
    ../../UI/StringTable.cpp
    ../../universe/Building.cpp
    ../../universe/Condition.cpp
    ../../universe/ConditionParser1.cpp
    ../../universe/ConditionParser2.cpp
    ../../universe/ConditionParser3.cpp
    ../../universe/ConditionParser.cpp
    ../../universe/Effect.cpp
    ../../universe/EffectAccounting.cpp
    ../../universe/EffectParser.cpp
    ../../universe/Enums.cpp
    ../../universe/Fleet.cpp
    ../../universe/Meter.cpp
    ../../universe/ObjectMap.cpp
    ../../universe/ParserUtil.cpp
    ../../universe/Planet.cpp
    ../../universe/PopCenter.cpp
    ../../universe/Predicates.cpp
    ../../universe/ResourceCenter.cpp
    ../../universe/Ship.cpp
    ../../universe/ShipDesign.cpp
    ../../universe/Special.cpp
    ../../universe/Species.cpp
    ../../universe/System.cpp
    ../../universe/Tech.cpp
    ../../universe/TopLevelParsers.cpp
    ../../universe/Universe.cpp
    ../../universe/UniverseObject.cpp
    ../../universe/ValueRef.cpp
    ../../universe/ValueRefParser.cpp
    ../../util/DataTable.cpp
    ../../util/GZStream.cpp
    ../../util/Math.cpp
    ../../util/MultiplayerCommon.cpp
    ../../util/OptionsDB.cpp
    ../../util/Order.cpp
    ../../util/OrderSet.cpp
    ../../util/Process.cpp
    ../../util/Random.cpp
    ../../util/SerializeEmpire.cpp
    ../../util/SerializeMultiplayerCommon.cpp
    ../../util/SerializeOrderSet.cpp
    ../../util/SerializePathingEngine.cpp
    ../../util/SerializeUniverse.cpp
    ../../util/SitRepEntry.cpp
    ../../util/VarText.cpp
    ../../util/binreloc.c
    ../../util/Directories.cpp
    ../../util/XMLDoc.cpp
    ../../log4cpp/src/Appender.cpp
    ../../log4cpp/src/AppenderSkeleton.cpp
    ../../log4cpp/src/BasicLayout.cpp
    ../../log4cpp/src/Category.cpp
    ../../log4cpp/src/CategoryStream.cpp
    ../../log4cpp/src/Configurator.cpp
    ../../log4cpp/src/FactoryParams.cpp
    ../../log4cpp/src/FileAppender.cpp
    ../../log4cpp/src/HierarchyMaintainer.cpp
    ../../log4cpp/src/LayoutAppender.cpp
    ../../log4cpp/src/Localtime.cpp
    ../../log4cpp/src/LoggingEvent.cpp
    ../../log4cpp/src/NDC.cpp
    ../../log4cpp/src/PatternLayout.cpp
    ../../log4cpp/src/Priority.cpp
    ../../log4cpp/src/StringUtil.cpp
    ../../log4cpp/src/TimeStamp.cpp
    ../../client/ClientApp.cpp
    ../../client/ClientFSMEvents.cpp
    ../../client/AI/AIClientApp.cpp
    ../../client/AI/camain.cpp
    ../../network/ClientNetworking.cpp
    ../../util/AppInterface.cpp
    ../../AI/AIInterface.cpp
    ../../AI/PythonAI.cpp
    ../../python/PythonEnumWrapper.cpp
    ../../python/PythonUniverseWrapper.cpp
    ../../python/PythonEmpireWrapper.cpp
    ../../python/PythonLoggingWrapper.cpp

    ${CMAKE_BINARY_DIR}/util/Version.cpp
)

if (WIN32)
    list(APPEND THIS_EXE_SOURCES ../../log4cpp/src/MSThreads.cpp)
else ()
    list(APPEND THIS_EXE_SOURCES ../../log4cpp/src/PThreads.cpp)
endif ()

list(APPEND MINIMUM_BOOST_COMPONENTS iostreams python)
find_package(Boost ${MINIMUM_BOOST_VERSION} COMPONENTS ${MINIMUM_BOOST_COMPONENTS})
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
else ()
    message(FATAL_ERROR "Boost libraries not found.")
endif ()

find_package(PythonLibs)
if (PYTHONLIBS_FOUND)
    include_directories(${PYTHON_INCLUDE_PATH})
else ()
    message(FATAL_ERROR " library not found.")
endif ()

add_definitions(-DFREEORION_BUILD_AI)

set(THIS_EXE_LINK_LIBS ${PYTHON_LIBRARIES}) # freeorion_lib

if (WIN32)
    link_directories(${BOOST_LIBRARYDIR})
    set(THIS_EXE_LINK_LIBS ${THIS_EXE_LINK_LIBS} ${GIGI_GIGI_LIBRARY} ${ZLIB_LIBRARY})
else ()
    set(THIS_EXE_LINK_LIBS ${THIS_EXE_LINK_LIBS} ${Boost_LIBRARIES} ${ZLIB_LIBRARY})
endif ()

executable_all_variants(freeorionca)

if (WIN32)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE -D_SCL_SECURE_NO_DEPRECATE)
    set_target_properties(freeorionca
        PROPERTIES
        COMPILE_DEFINITIONS BOOST_ALL_DYN_LINK
        LINK_FLAGS /NODEFAULTLIB:LIBCMT
    )
endif ()
