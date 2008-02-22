# -*- Python -*-
Import('env')

common_sources = [
    'combat/Combat.cpp',
    'Empire/Empire.cpp',
    'Empire/EmpireManager.cpp',
    'Empire/ResourcePool.cpp',
    'network/Message.cpp',
    'network/MessageQueue.cpp',
    'network/Networking.cpp',
    'network/boost/error_code.cpp',
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
    'util/XMLDoc.cpp'
    ]

if env['target_define'] == 'FREEORION_BUILD_SERVER':
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

if env['target_define'] == 'FREEORION_BUILD_AI':
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
        'python/PythonEnumWrapper.cpp'
        ]
    target = 'ai'

if env['target_define'] == 'FREEORION_BUILD_HUMAN':
    target_sources = [
        'client/ClientApp.cpp',
        'client/ClientFSMEvents.cpp',
        'client/human/HumanClientFSM.cpp',
        'client/human/HumanClientApp.cpp',
        'client/human/HumanClientAppSoundOpenAL.cpp',
        'client/human/chmain.cpp',
        'network/ClientNetworking.cpp',
        'UI/About.cpp',
        'UI/BuildDesignatorWnd.cpp',
        'UI/ClientUI.cpp',
        'UI/CUIControls.cpp',
        'UI/CUIDrawUtil.cpp',
        'UI/CUIStyle.cpp',
        'UI/CUIWnd.cpp',
        'UI/CombatWnd.cpp',
        'UI/FleetButton.cpp',
        'UI/FleetWnd.cpp',
        'UI/GalaxySetupWnd.cpp',
        'UI/InGameMenu.cpp',
        'UI/InfoPanels.cpp',
        'UI/IntroScreen.cpp',
        'UI/LinkText.cpp',
        'UI/MapWnd.cpp',
        'UI/MultiplayerLobbyWnd.cpp',
        'UI/OptionsWnd.cpp',
        'UI/DesignWnd.cpp',
        'UI/ProductionWnd.cpp',
        'UI/ResearchWnd.cpp',
        'UI/ServerConnectWnd.cpp',
        'UI/SidePanel.cpp',
        'UI/SitRepPanel.cpp',
        'UI/SystemIcon.cpp',
        'UI/TechTreeWnd.cpp',
        'UI/TurnProgressWnd.cpp',
        'universe/Universe.cpp',
        'util/AppInterface.cpp'
        ]
    target = 'human'

objects = env.Object(common_sources)
objects += [env.Object(target = source.split(".")[0] + '-' + target,
                       source = source,
                       CPPDEFINES = env['CPPDEFINES'] + [env['target_define']])
            for source in target_sources]
Return('objects')
