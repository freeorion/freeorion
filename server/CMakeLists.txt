cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6.4)

project(freeoriond)

message("-- Configuring freeoriond")

set(sources
    ../combat/Combat.cpp
    ../combat/CombatOrder.cpp
    ../combat/OpenSteer/AsteroidBeltObstacle.cpp
    ../combat/OpenSteer/CombatFighter.cpp
    ../combat/OpenSteer/CombatObject.cpp
    ../combat/OpenSteer/CombatShip.cpp
    ../combat/OpenSteer/Missile.cpp
    ../combat/OpenSteer/Obstacle.cpp
    ../combat/OpenSteer/PathingEngine.cpp
    ../combat/OpenSteer/SimpleVehicle.cpp
    ../combat/OpenSteer/Vec3.cpp
    ../combat/OpenSteer/Vec3Utilities.cpp
    ../Empire/Empire.cpp
    ../Empire/EmpireManager.cpp
    ../Empire/ResourcePool.cpp
    ../network/Message.cpp
    ../network/MessageQueue.cpp
    ../network/Networking.cpp
    ../UI/StringTable.cpp
    ../universe/Building.cpp
    ../universe/Condition.cpp
    ../universe/ConditionParser1.cpp
    ../universe/ConditionParser2.cpp
    ../universe/ConditionParser.cpp
    ../universe/Effect.cpp
    ../universe/EffectParser.cpp
    ../universe/Enums.cpp
    ../universe/Fleet.cpp
    ../universe/Meter.cpp
    ../universe/ParserUtil.cpp
    ../universe/Planet.cpp
    ../universe/PopCenter.cpp
    ../universe/Predicates.cpp
    ../universe/ResourceCenter.cpp
    ../universe/Ship.cpp
    ../universe/ShipDesign.cpp
    ../universe/Special.cpp
    ../universe/System.cpp
    ../universe/Tech.cpp
    ../universe/TopLevelParsers.cpp
    ../universe/UniverseObject.cpp
    ../universe/ValueRef.cpp
    ../universe/ValueRefParser.cpp
    ../util/DataTable.cpp
    ../util/GZStream.cpp
    ../util/MultiplayerCommon.cpp
    ../util/OptionsDB.cpp
    ../util/Order.cpp
    ../util/OrderSet.cpp
    ../util/Process.cpp
    ../util/Random.cpp
    ../util/Serialize.cpp
    ../util/SitRepEntry.cpp
    ../util/VarText.cpp
    ../util/binreloc.c
    ../util/Directories.cpp
    ../util/XMLDoc.cpp
    ../log4cpp/src/Appender.cpp
    ../log4cpp/src/AppenderSkeleton.cpp
    ../log4cpp/src/BasicLayout.cpp
    ../log4cpp/src/Category.cpp
    ../log4cpp/src/CategoryStream.cpp
    ../log4cpp/src/Configurator.cpp
    ../log4cpp/src/FactoryParams.cpp
    ../log4cpp/src/FileAppender.cpp
    ../log4cpp/src/HierarchyMaintainer.cpp
    ../log4cpp/src/LayoutAppender.cpp
    ../log4cpp/src/Localtime.cpp
    ../log4cpp/src/LoggingEvent.cpp
    ../log4cpp/src/NDC.cpp
    ../log4cpp/src/PatternLayout.cpp
    ../log4cpp/src/Priority.cpp
    ../log4cpp/src/StringUtil.cpp
    ../log4cpp/src/TimeStamp.cpp
    ../log4cpp/src/PThreads.cpp
    ../combat/CombatSystem.cpp
    ../network/ServerNetworking.cpp
    ../server/SaveLoad.cpp
    ../server/ServerApp.cpp
    ../server/ServerFSM.cpp
    ../server/dmain.cpp
    ../universe/Universe.cpp
    ../util/AppInterface.cpp

    ${CMAKE_BINARY_DIR}/util/Version.cpp
)

add_definitions(-DFREEORION_BUILD_SERVER)

add_executable(freeoriond ${sources})

set_target_properties(
    freeoriond
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
    CLEAN_DIRECT_OUTPUT 1
    LABELS "${PROJECT_NAME}"
)

target_link_libraries(freeoriond ${Boost_LIBRARIES} ${ZLIB_LIBRARIES})
