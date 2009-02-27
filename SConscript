# -*- Python -*-
Import('env')

common_sources = [
    'combat/Combat.cpp',
    'combat/CombatOrder.cpp',
    'combat/OpenSteer/CombatFighter.cpp',
    'combat/OpenSteer/CombatShip.cpp',
    'combat/OpenSteer/lq.c',
    'combat/OpenSteer/Obstacle.cpp',
    'combat/OpenSteer/Path.cpp',
    'combat/OpenSteer/PathingEngine.cpp',
    'combat/OpenSteer/Pathway.cpp',
    'combat/OpenSteer/PolylineSegmentedPath.cpp',
    'combat/OpenSteer/PolylineSegmentedPathwaySegmentRadii.cpp',
    'combat/OpenSteer/PolylineSegmentedPathwaySingleRadius.cpp',
    'combat/OpenSteer/SegmentedPath.cpp',
    'combat/OpenSteer/SegmentedPathway.cpp',
    'combat/OpenSteer/SimpleVehicle.cpp',
    'combat/OpenSteer/Vec3.cpp',
    'combat/OpenSteer/Vec3Utilities.cpp',
    'Empire/Empire.cpp',
    'Empire/EmpireManager.cpp',
    'Empire/ResourcePool.cpp',
    'network/Message.cpp',
    'network/MessageQueue.cpp',
    'network/Networking.cpp',
    'UI/StringTable.cpp',
    'universe/Building.cpp',
    'universe/Condition.cpp',
    'universe/ConditionParser1.cpp',
    'universe/ConditionParser2.cpp',
    'universe/ConditionParser.cpp',
    'universe/Effect.cpp',
    'universe/EffectParser.cpp',
    'universe/Enums.cpp',
    'universe/Fleet.cpp',
    'universe/Meter.cpp',
    'universe/ParserUtil.cpp',
    'universe/Planet.cpp',
    'universe/PopCenter.cpp',
    'universe/Predicates.cpp',
    'universe/ResourceCenter.cpp',
    'universe/Ship.cpp',
    'universe/ShipDesign.cpp',
    'universe/Special.cpp',
    'universe/System.cpp',
    'universe/Tech.cpp',
    'universe/TopLevelParsers.cpp',
    'universe/UniverseObject.cpp',
    'universe/ValueRef.cpp',
    'universe/ValueRefParser.cpp',
    'util/DataTable.cpp',
    'util/GZStream.cpp',
    'util/MultiplayerCommon.cpp',
    'util/OptionsDB.cpp',
    'util/Order.cpp',
    'util/OrderSet.cpp',
    'util/Process.cpp',
    'util/Random.cpp',
    'util/Serialize.cpp',
    'util/SitRepEntry.cpp',
    'util/VarText.cpp',
    'util/Version.cpp',
    'util/binreloc.c',
    'util/Directories.cpp',
    'util/XMLDoc.cpp',
    'log4cpp/src/Appender.cpp',
    'log4cpp/src/AppenderSkeleton.cpp',
    'log4cpp/src/BasicLayout.cpp',
    'log4cpp/src/Category.cpp',
    'log4cpp/src/CategoryStream.cpp',
    'log4cpp/src/Configurator.cpp',
    'log4cpp/src/FactoryParams.cpp',
    'log4cpp/src/FileAppender.cpp',
    'log4cpp/src/HierarchyMaintainer.cpp',
    'log4cpp/src/LayoutAppender.cpp',
    'log4cpp/src/Localtime.cpp',
    'log4cpp/src/LoggingEvent.cpp',
    'log4cpp/src/NDC.cpp',
    'log4cpp/src/PatternLayout.cpp',
    'log4cpp/src/Priority.cpp',
    'log4cpp/src/StringUtil.cpp',
    'log4cpp/src/TimeStamp.cpp'
    ]

if str(Platform()) == 'win32':
    common_sources += ['log4cpp/src/MSThreads.cpp']
else:
    common_sources += ['log4cpp/src/PThreads.cpp']


if 'FREEORION_BUILD_SERVER' in env['target_defines']:
    target_sources = [
        'combat/CombatSystem.cpp',
        'network/ServerNetworking.cpp',
        'server/SaveLoad.cpp',
        'server/ServerApp.cpp',
        'server/ServerFSM.cpp',
        'server/dmain.cpp',
        'universe/Universe.cpp',
        'util/AppInterface.cpp'
        ]
    target = 'server'

if 'FREEORION_BUILD_AI' in env['target_defines']:
    target_sources = [
        'client/ClientApp.cpp',
        'client/ClientFSMEvents.cpp',
        'client/AI/AIClientApp.cpp',
        'client/AI/camain.cpp',
        'network/ClientNetworking.cpp',
        'universe/Universe.cpp',
        'util/AppInterface.cpp',
        'AI/AIInterface.cpp',
        'AI/PythonAI.cpp',
        'python/PythonEnumWrapper.cpp',
        'python/PythonUniverseWrapper.cpp',
        'python/PythonEmpireWrapper.cpp',
        'python/PythonLoggingWrapper.cpp'
        ]
    target = 'ai'

libs = []
if 'FREEORION_BUILD_HUMAN' in env['target_defines']:
    # This is here so it doesn't get lost, but it is not part of the normal
    # build -- it only exists to make small changes very quick by avoiding
    # relinking the entire FreeOrion app.
    #lib_obj = env.SharedObject(source = 'UI/CombatWnd.cpp',
    #                           CPPDEFINES = env['CPPDEFINES'] + env['target_defines'])
    #libs.append(env.SharedLibrary('CombatWnd', [lib_obj]))
    target_sources = [
        'client/ClientApp.cpp',
        'client/ClientFSMEvents.cpp',
        'client/human/HumanClientFSM.cpp',
        'client/human/HumanClientApp.cpp',
        'client/human/chmain.cpp',
        'network/ClientNetworking.cpp',
        'UI/About.cpp',
        'UI/BuildDesignatorWnd.cpp',
        'UI/ChatWnd.cpp',
        'UI/ClientUI.cpp',
        'UI/CUIControls.cpp',
        'UI/CUIDrawUtil.cpp',
        'UI/CUIStyle.cpp',
        'UI/CUIWnd.cpp',
        'UI/EncyclopediaDetailPanel.cpp',
        'UI/FleetButton.cpp',
        'UI/FleetWnd.cpp',
        'UI/GalaxySetupWnd.cpp',
        'UI/InGameMenu.cpp',
        'UI/InfoPanels.cpp',
        'UI/IntroScreen.cpp',
        'UI/LinkText.cpp',
        'UI/CollisionMeshConverter.cpp',
        'UI/CombatWnd.cpp',
        'UI/MapWnd.cpp',
        'UI/MultiplayerLobbyWnd.cpp',
        'UI/OptionsWnd.cpp',
        'UI/DesignWnd.cpp',
        'UI/ProductionWnd.cpp',
        'UI/QueueListBox.cpp',
        'UI/ResearchWnd.cpp',
        'UI/ServerConnectWnd.cpp',
        'UI/SidePanel.cpp',
        'UI/SitRepPanel.cpp',
        'UI/Sound.cpp',
        'UI/SystemIcon.cpp',
        'UI/TechTreeWnd.cpp',
        'UI/TurnProgressWnd.cpp',
        'UI/PagedGeometry/BatchedGeometry.cpp',
        'UI/PagedGeometry/BatchPage.cpp',
        'UI/PagedGeometry/GrassLoader.cpp',
        'UI/PagedGeometry/ImpostorPage.cpp',
        'UI/PagedGeometry/PagedGeometry.cpp',
        'UI/PagedGeometry/PropertyMaps.cpp',
        'UI/PagedGeometry/StaticBillboardSet.cpp',
        'UI/PagedGeometry/TreeLoader2D.cpp',
        'UI/PagedGeometry/TreeLoader3D.cpp',
        'UI/PagedGeometry/WindBatchedGeometry.cpp',
        'UI/PagedGeometry/WindBatchPage.cpp',
        'universe/Universe.cpp',
        'util/AppInterface.cpp'
        ]
    target = 'human'

objects = env.Object(common_sources)
objects += [env.Object(target = source.split(".")[0] + '-' + target,
                       source = source,
                       CPPDEFINES = env['CPPDEFINES'] + env['target_defines'])
            for source in target_sources]
Return('objects', 'libs')
