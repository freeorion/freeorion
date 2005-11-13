# -*- Python -*-

sources = {
  # Files in this list are used by server and client alike, and do not contain #ifdef FREEORION_BUILD directives
  "common": [
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
    'UI/StringTable.cpp',
    # StringTable needs this
    'util/binreloc.c',
    'util/Directories.cpp',
  ],

  # Files in the following sections are either unique to a particular
  # binary or must be recompiled for every binary because they contain
  # conditional compilation directives like FREEORION_BUILD_SERVER
  "server": [
    'universe/Universe.cpp',
    'util/AppInterface.cpp',
    'server/ServerApp.cpp',
    'server/dmain.cpp',
    'combat/CombatSystem.cpp',
    'Empire/ServerEmpireManager.cpp',
    'network/ServerNetworkCore.cpp'
  ],

  "ai": [
    'universe/Universe.cpp',
    'util/AppInterface.cpp',
    'network/ClientNetworkCore.cpp',
    'client/ClientApp.cpp',
    'client/AI/AIClientApp.cpp',
    'client/AI/camain.cpp'
  ],

  "human": [
    'universe/Universe.cpp',
    'util/AppInterface.cpp',
    'network/ClientNetworkCore.cpp',
    'client/ClientApp.cpp',
    'client/human/HumanClientApp.cpp',
    'client/human/chmain.cpp',
    'UI/About.cpp',
    'UI/BuildDesignatorWnd.cpp',
    'UI/ClientUI.cpp',
    'UI/CUIControls.cpp',
    'UI/CUIDrawUtil.cpp',
    'UI/CUITabbedPages.cpp',
    'UI/CUI_Wnd.cpp',
    'UI/CombatWnd.cpp',
    'UI/FleetButton.cpp',
    'UI/FleetWindow.cpp',
    'UI/FocusSelector.cpp',
    'UI/GalaxySetupWnd.cpp',
    'UI/InGameOptions.cpp',
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
    'UI/ToolContainer.cpp',
    'UI/ToolWnd.cpp',
    'UI/TurnProgressWnd.cpp'
  ]
}

if str(Platform()) == "posix":
    
    # Minimal linux config, might need manual adjusment
    env = Environment(CPPDEFINES = ["FREEORION_LINUX","ENABLE_BINRELOC",
                                    #"FREEORION_BUILD_AUTOPACKAGE",
                                    "FREEORION_RELEASE"
                                    ],
                      CPPPATH = ["/usr/local/include/GG",
                                 "/usr/local/include/GG/dialogs",
                                 "/usr/local/include/GG/net",
                                 #"/usr/include/python2.3",
                                 "/usr/local/include/GG/SDL"
                                 ],
                      CCFLAGS = ["-O2"]
                      )

    try:
        env["CXX"] = ARGUMENTS["CXX"]
    except KeyError:
        pass

    env.ParseConfig("sdl-config --cflags --libs")
    env.ParseConfig("freetype-config --cflags --libs")
    #Help(opts.GenerateHelpText(env))
    env.Append(LIBS = ["log4cpp",
                       #"boost_signals",
                       "GiGi",
                       "GiGiSDL",
                       "GiGiNet",
                       "fmod-3.74",
                       'cdt',
                       'common',
                       'dotgen',
                       'dotneato',
                       'graph',
                       #'boost_python',
                       'gvrender'
                       ])
    env.Append(LIBPATH = "/usr/lib/graphviz")
    env.Append(LINKFLAGS = ["-Wl,-rpath,/usr/lib/graphviz"])

# windows
elif str(Platform()) == 'win32':
    print 'Win32 platform detected ...'
    gg_dir = 'GG/'
    cpppath = [
        'C:/log4cpp-0.3.4b/include',
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
        gg_dir + 'include/SDL'
        ]
    libs = [
        'GiGiNet',
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
        'fmodvc.lib'
        ]
    libpath = [
        'C:/Boost/lib',
        'C:/log4cpp-0.3.4b/msvc6/log4cppDLL/Release',
        'C:/freetype-2.1.7/objs',
        'C:/zlib/lib',
        'C:/SDL-1.2.7/lib',
        'C:/graphviz/lib',
        'C:/fmodapi374win/api/lib',
        gg_dir
        ]
    ccflags = [
        '/O2',
        '/MD',
        '/EHsc',
        '/W3',
        '/Zc:forScope',
        '/GR',
        '/Gd',
        '/Zi',
        '/nologo',
        '/wd4099', '/wd4251', '/wd4800', '/wd4267', '/wd4275', '/wd4244', '/wd4101', '/wd4258'
        ]
    defines = [
        '_DEBUG',
        'WIN32',
        '_WINDOWS',
        'FREEORION_WIN32',
        'BOOST_SIGNALS_STATIC_LINK',
        '_MBCS'
        ]
    linkflags = [
        '/INCREMENTAL:NO',
        '/NOLOGO',
        '/NODEFAULTLIB:"LIBCMT"',
        '/DEBUG',
        '/SUBSYSTEM:CONSOLE',
        '/OPT:REF',
        '/OPT:ICF',
        '/MACHINE:X86'
        ]
    env = Environment(CCFLAGS = ccflags,
                      CPPDEFINES = defines,
                      CPPPATH = cpppath,
                      LIBS = libs,
                      LIBPATH = libpath,
                      LINKFLAGS = linkflags)
else:
    print "Platform '%s' unrecognized" % Platform()
    Exit(1)

# We reasonably assume every complete source file path contains exactly one dot!

objects = {}
for exe in sources:
    env2 = env.Copy()
    env2.Append(CPPDEFINES = "FREEORION_BUILD_" + exe.upper())
    objects[exe] = [env2.Object(target = name.split(".")[0] + '-' + exe,
                                source = name) for name in sources[exe]]


env.Program("freeorionca", objects["ai"] + objects["common"])
env.Program("freeorion", objects["human"] + objects["common"])
env.Program("freeoriond", objects["server"] + objects["common"])

#env.Install(ARGUMENTS["IDIR"],Split("freeorion freeoriond freeorionca"))
#Execute(Copy(ARGUMENTS["IDIR"]+"/default/","default/"))


#CacheDir('scons_cache')
#TargetSignatures('content')
#SetOption('implicit_cache', 1)
