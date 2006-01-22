# -*- Python -*-
Import('env')
common_sources = [
    'combat/Combat.cpp',
    'Empire/ClientEmpireManager.cpp',
    'Empire/Empire.cpp',
    'Empire/EmpireManager.cpp',
    'Empire/ResourcePool.cpp',
    'network/Message.cpp',
    'network/NetworkCore.cpp',
    'network/XDiff.cpp',
    'network/XHash.cpp',
    'network/XLut.cpp',
    'network/XParser.cpp',
    'network/XTree.cpp',
    'UI/StringTable.cpp',
    'universe/Building.cpp',
    'universe/Condition.cpp',
    'universe/Effect.cpp',
    'universe/Enums.cpp',
    'universe/Fleet.cpp',
    'universe/Meter.cpp',
    'universe/Planet.cpp',
    'universe/PopCenter.cpp',
    'universe/Predicates.cpp',
    'universe/ResourceCenter.cpp',
    'universe/Ship.cpp',
    'universe/ShipDesign.cpp',
    'universe/Special.cpp',
    'universe/System.cpp',
    'universe/Tech.cpp',
    'universe/UniverseObject.cpp',
    'universe/ValueRef.cpp',
    'util/DataTable.cpp',
    'util/GZStream.cpp',
    'util/md5.c',
    'util/MultiplayerCommon.cpp',
    'util/OptionsDB.cpp',
    'util/Order.cpp',
    'util/OrderSet.cpp',
    'util/Process.cpp',
    'util/Random.cpp',
    'util/SitRepEntry.cpp',
    'util/VarText.cpp',
    'util/binreloc.c',
    'util/Directories.cpp',
    'util/XMLDoc.cpp'
    ]

if env['target_define'] == 'FREEORION_BUILD_SERVER':
    target_sources = [
        'Empire/ServerEmpireManager.cpp',
        'combat/CombatSystem.cpp',
        'network/ServerNetworkCore.cpp',
        'server/ServerApp.cpp',
        'server/dmain.cpp',
        'universe/Universe.cpp',
        'util/AppInterface.cpp'
        ]
    target = 'server'

if env['target_define'] == 'FREEORION_BUILD_AI':
    target_sources = [
        'client/ClientApp.cpp',
        'client/AI/AIClientApp.cpp',
        'client/AI/camain.cpp',
        'network/ClientNetworkCore.cpp',
        'universe/Universe.cpp',
        'util/AppInterface.cpp'
        ]
    target = 'ai'

if env['target_define'] == 'FREEORION_BUILD_HUMAN':
    target_sources = [
        'client/ClientApp.cpp',
        'client/human/HumanClientApp.cpp',
        'client/human/chmain.cpp',
        'network/ClientNetworkCore.cpp',
        'UI/About.cpp',
        'UI/BuildDesignatorWnd.cpp',
        'UI/ClientUI.cpp',
        'UI/CUIControls.cpp',
        'UI/CUIDrawUtil.cpp',
        'UI/CUIStyle.cpp',
        'UI/CUITabbedPages.cpp',
        'UI/CUIWnd.cpp',
        'UI/CombatWnd.cpp',
        'UI/FleetButton.cpp',
        'UI/FleetWindow.cpp',
        'UI/FocusSelector.cpp',
        'UI/GalaxySetupWnd.cpp',
        'UI/InGameMenu.cpp',
        'UI/IntroScreen.cpp',
        'UI/LinkText.cpp',
        'UI/MapWnd.cpp',
        'UI/MultiplayerLobbyWnd.cpp',
        'UI/OptionsWnd.cpp',
        'UI/ProductionWnd.cpp',
        'UI/ResearchWnd.cpp',
        'UI/ServerConnectWnd.cpp',
        'UI/SidePanel.cpp',
        'UI/SitRepPanel.cpp',
        'UI/Splash.cpp',
        'UI/SystemIcon.cpp',
        'UI/TechWnd.cpp',
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
