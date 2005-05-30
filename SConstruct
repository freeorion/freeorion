# -*- Python -*-

import os
platform_str = "%s" % Platform()

# posix -- not yet implemented

# windows
if platform_str == 'win32':
    print 'Win32 platform detected ...'
    gg_dir = 'GG/'
    cpppath = ['C:/log4cpp-0.3.4b/include',
               'C:/Boost/include/boost-1_32',
               'C:/SDL-1.2.7/include',
               'C:/log4cpp-0.3.4b/include',
               'C:/freetype-2.1.7/include',
               'C:/zlib/include',
               'C:/graphviz/include',
               'C:/fmodapi374win/api/inc',
               gg_dir + 'include',
               gg_dir + 'include/dialogs',
               gg_dir + 'include/net',
               gg_dir + 'include/SDL']
    libs = ['GiGiNet',
            'GiGiSDL',
            'GiGi',
            'opengl32',
            'glu32',
            'wsock32',
            'kernel32',
            'user32',
            'gdi32.lib',
            'winspool.lib',
            'comdlg32.lib',
            'zdll',
            'SDL',
            'SDLmain',
            'log4cpp',
            'freetype214MT',
            'cdt',
            'common',
            'dotgen',
            'dotneato',
            'gd',
            'gfx',
            'graph',
            'gvrender',
            'pathplan',
            'libexpat',
            'png',
            'jpeg',
            'fmodvc.lib']
    libpath = ['C:/Boost/lib',
               'C:/log4cpp-0.3.4b/msvc6/log4cppDLL/Release',
               'C:/freetype-2.1.7/objs',
               'C:/zlib/lib',
               'C:/SDL-1.2.7/lib',
               'C:/graphviz/lib',
               'C:/fmodapi374win/api/lib',
               gg_dir + 'msvc/GG/GiGi/Release',
               gg_dir + 'msvc/GG/GiGiNet/Release',
               gg_dir + 'msvc/GG/GiGiSDL/Release']
    common_ccflags = '/O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "FREEORION_WIN32" /D "BOOST_SIGNALS_STATIC_LINK" /D "_MBCS" /FD /EHsc /MD /GS /Zc:forScope /GR /W3 /nologo /c /Wp64 /Zi /wd4099 /wd4251 /wd4800 /wd4267 /wd4275 /wd4244 /wd4101 /wd4258'
    freeoriond_ccflags = common_ccflags + ' /D "FREEORION_BUILD_SERVER"'
    freeorionca_ccflags = common_ccflags + ' /D "FREEORION_BUILD_AI"'
    freeorion_ccflags = common_ccflags + ' /D "FREEORION_BUILD_HUMAN"'
    env = Environment(CCFLAGS = '',
                      CPPPATH = cpppath,
                      LIBS = libs,
                      LIBPATH = libpath,
                      LINKFLAGS = '/INCREMENTAL:NO /NOLOGO /NODEFAULTLIB:"LIBCMT" /DEBUG /SUBSYSTEM:CONSOLE /OPT:REF /OPT:ICF /MACHINE:X86')

common_source = ['combat/Combat.cpp',
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
                 'universe/Building.cpp',
                 'universe/Condition.cpp',
                 'universe/Effect.cpp',
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
                 'UI/StringTable.cpp']
common_objects = Flatten([Object(x, CCFLAGS=common_ccflags, CPPPATH=cpppath) for x in common_source])

freeoriond_source = ['server/ServerApp.cpp',
                     'server/dmain.cpp',
                     'combat/CombatSystem.cpp',
                     'Empire/ServerEmpireManager.cpp',
                     'network/ServerNetworkCore.cpp']
freeoriond_objects = Flatten([Object(x, CCFLAGS=freeoriond_ccflags, CPPPATH=cpppath) for x in freeoriond_source])
freeoriond_objects.append(Object('universe/Universe-d', 'universe/Universe.cpp', CCFLAGS=freeoriond_ccflags, CPPPATH=cpppath))
freeoriond_objects.append(Object('util/AppInterface-d', 'util/AppInterface.cpp', CCFLAGS=freeoriond_ccflags, CPPPATH=cpppath))

freeorionca_source = ['client/AI/AIClientApp.cpp',
                      'client/AI/camain.cpp']
freeorionca_objects = Flatten([Object(x, CCFLAGS=freeorionca_ccflags, CPPPATH=cpppath) for x in freeorionca_source])
freeorionca_objects.append(Object('universe/Universe-ca', 'universe/Universe.cpp', CCFLAGS=freeorionca_ccflags, CPPPATH=cpppath))
freeorionca_objects.append(Object('util/AppInterface-ca', 'util/AppInterface.cpp', CCFLAGS=freeorionca_ccflags, CPPPATH=cpppath))
freeorionca_objects.append(Object('network/ClientNetworkCore-ca', 'network/ClientNetworkCore.cpp', CCFLAGS=freeorionca_ccflags, CPPPATH=cpppath))

freeorion_source = ['client/human/HumanClientApp.cpp',
                    'client/human/chmain.cpp',
                    'UI/About.cpp',
                    'UI/ClientUI.cpp',
                    'UI/CUIControls.cpp',
                    'UI/CUIDrawUtil.cpp',
                    'UI/CUI_Wnd.cpp',
                    'UI/CombatWnd.cpp',
                    'UI/FleetButton.cpp',
                    'UI/FleetWindow.cpp',
                    'UI/GalaxySetupWnd.cpp',
                    'UI/InGameOptions.cpp',
                    'UI/IntroScreen.cpp',
                    'UI/LinkText.cpp',
                    'UI/MapWnd.cpp',
                    'UI/MultiplayerLobbyWnd.cpp',
                    'UI/OptionsWnd.cpp',
                    'UI/ResearchWnd.cpp',
                    'UI/ServerConnectWnd.cpp',
                    'UI/SidePanel.cpp',
                    'UI/SitRepPanel.cpp',
                    'UI/SystemIcon.cpp',
                    'UI/TechWnd.cpp',
                    'UI/ToolContainer.cpp',
                    'UI/ToolWnd.cpp',
                    'UI/TurnProgressWnd.cpp']
freeorion_objects = Flatten([Object(x, CCFLAGS=freeorion_ccflags, CPPPATH=cpppath) for x in freeorion_source])
freeorion_objects.append(Object('universe/Universe-h', 'universe/Universe.cpp', CCFLAGS=freeorion_ccflags, CPPPATH=cpppath))
freeorion_objects.append(Object('util/AppInterface-h', 'util/AppInterface.cpp', CCFLAGS=freeorion_ccflags, CPPPATH=cpppath))
freeorion_objects.append(Object('network/ClientNetworkCore-h', 'network/ClientNetworkCore.cpp', CCFLAGS=freeorion_ccflags, CPPPATH=cpppath))

client_app_object = Object('client/ClientApp.cpp', CCFLAGS=common_ccflags, CPPPATH=cpppath)

env.Program('freeoriond', freeoriond_objects + common_objects)
env.Program('freeorionca', freeorionca_objects + client_app_object + common_objects)
env.Program('freeorion', freeorion_objects + client_app_object + common_objects)

CacheDir('scons_cache')
TargetSignatures('content')
SetOption('implicit_cache', 1)
