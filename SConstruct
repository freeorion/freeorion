# -*- Python -*-

import os
import pickle
import re
from build_config import *
from build_support import *

env = Environment()

mising_pkg_config = not WhereIs('pkg-config')

##################################################
# create options                                 #
##################################################
options = Options('options.cache')
options.Add(BoolOption('release', 'Build for public release (random numbers are nondeterminisitc, etc.).  This will force debug=0.', 0))
options.Add(BoolOption('debug', 'Generate debug code', 0))
options.Add(BoolOption('multithreaded', 'Generate multithreaded code', 1))
if str(Platform()) == 'win32':
    options.Add(BoolOption('dynamic', 'Generate a dynamic-link code', 1))
if WhereIs('distcc'):
    options.Add(BoolOption('use_distcc', 'Use distcc to build FreeOrion', 0))
if str(Platform()) == 'posix':
    options.Add('prefix', 'Location to install FreeOrion', '/usr/local')
else:
    options.Add('prefix', 'Location to install FreeOrion', 'C:\\')
options.Add('scons_cache_dir', 'Directory to use for SCons object file caching (specifying any directory will enable caching)')
options.Add('bindir', 'Location to install executables', os.path.normpath(os.path.join('$prefix', 'bin')))
options.Add('with_boost', 'Root directory of boost installation')
options.Add('with_boost_include', 'Specify exact include dir for boost headers')
options.Add('with_boost_libdir', 'Specify exact library dir for boost library')
options.Add('boost_lib_suffix', 'Specify the suffix placed on user-compiled Boost libraries (e.g. "-vc71-mt-gd-1_31")')
options.Add('boost_signals_namespace',
            'Specify alternate namespace used for boost::signals (only needed if you changed it using the BOOST_SIGNALS_NAMESPACE define when you built boost)')
options.Add('with_sdl', 'Root directory of SDL installation')
options.Add('with_sdl_include', 'Specify exact include dir for SDL headers')
options.Add('with_sdl_libdir', 'Specify exact library dir for SDL library')
options.Add(BoolOption('with_builtin_sdlnet', 'Use built-in version of SDL_net', 1))
options.Add('with_sdlnet', 'Root directory of SDL_net installation (ignored if $with_builtin_sdlnet is true)')
options.Add('with_sdlnet_include', 'Specify exact include dir for SDL_net headers (ignored if $with_builtin_sdlnet is true)')
options.Add('with_sdlnet_libdir', 'Specify exact library dir for SDL_net library (ignored if $with_builtin_sdlnet is true)')
options.Add('with_ft', 'Root directory of FreeType2 installation')
options.Add('with_ft_include', 'Specify exact include dir for FreeType2 headers')
options.Add('with_ft_libdir', 'Specify exact library dir for FreeType2 library')
options.Add('with_devil', 'Root directory of DevIL installation')
options.Add('with_devil_include', 'Specify exact include dir for DevIL headers')
options.Add('with_devil_libdir', 'Specify exact library dir for DevIL library')
options.Add('with_log4cpp', 'Root directory of Log4cpp installation')
options.Add('with_log4cpp_include', 'Specify exact include dir for Log4cpp headers')
options.Add('with_log4cpp_libdir', 'Specify exact library dir for Log4cpp library')
options.Add('with_fmod', 'Root directory of FMOD installation')
options.Add('with_fmod_include', 'Specify exact include dir for FMOD headers')
options.Add('with_fmod_libdir', 'Specify exact library dir for FMOD library')
options.Add('with_graphviz', 'Root directory of GraphViz installation')
options.Add('with_graphviz_include', 'Specify exact include dir for GraphViz headers')
options.Add('with_graphviz_libdir', 'Specify exact library dir for GraphViz library')
options.Add('with_gg', 'Root directory of GG installation')
options.Add('with_gg_include', 'Specify exact include dir for GG headers')
options.Add('with_gg_libdir', 'Specify exact library dir for GG library')
if str(Platform()) == 'win32':
    options.Add('with_zlib', 'Root directory of zlib installation')
    options.Add('with_zlib_include', 'Specify exact include dir for zlib headers')
    options.Add('with_zlib_libdir', 'Specify exact library dir for zlib library')

##################################################
# build vars                                     #
##################################################
# fill Environment using saved and command-line provided options, save options for next time, and fill environment with
# save configuration values.
import sys
preconfigured = False
force_configure = False
command_line_args = sys.argv[1:]
if 'configure' in command_line_args:
    force_configure = True
elif ('-h' in command_line_args) or ('--help' in command_line_args):
    preconfigured = True # this is just to ensure config gets skipped when help is requested
ms_linker = 'msvs' in env['TOOLS'] or 'msvc' in env['TOOLS']

env_cache_keys = ['CCFLAGS',
                  'CPPDEFINES',
                  'CPPFLAGS',
                  'CPPPATH',
                  'CXXFLAGS',
                  'LIBPATH',
                  'LIBS',
                  'LINKFLAGS']
if not force_configure:
    try:
        f = open('config.cache', 'r')
        up = pickle.Unpickler(f)
        pickled_values = up.load()
        for key, value in pickled_values.items():
            env[key] = value
        preconfigured = True
        if ('-h' not in command_line_args) and ('--help' not in command_line_args):
            print 'Using previous successful configuration; if you want to re-run the configuration step, run "scons configure".'
    except Exception:
        None

options.Update(env)

if env.has_key('use_distcc') and env['use_distcc']:
    env['CC'] = 'distcc %s' % env['CC']
    env['CXX'] = 'distcc %s' % env['CXX']
    for i in ['HOME',
              'DISTCC_HOSTS',
              'DISTCC_VERBOSE',
              'DISTCC_LOG',
              'DISTCC_FALLBACK',
              'DISTCC_MMAP',
              'DISTCC_SAVE_TEMPS',
              'DISTCC_TCP_CORK',
              'DISTCC_SSH'
              ]:
        if os.environ.has_key(i) and not env.has_key(i):
            env['ENV'][i] = os.environ[i]

if env['release']:
    env['debug'] = 0

Help(GenerateHelpText(options, env))
options.Save('options.cache', env)

if str(Platform()) == 'win32':
    if not env['multithreaded']:
        if env['dynamic']:
            print 'Warning: since the Win32 platform does not support multithreaded DLLs, multithreaded static code will be produced instead.'
            env['dynamic'] = 0


##################################################
# check configuration                            #
##################################################
if not env.GetOption('clean'):
    if not preconfigured:
        conf = env.Configure(custom_tests = {'CheckVersionHeader' : CheckVersionHeader,
                                             'CheckPkgConfig' : CheckPkgConfig,
                                             'CheckPkg' : CheckPkg,
                                             'CheckBoost' : CheckBoost,
                                             'CheckBoostLib' : CheckBoostLib,
                                             'CheckSDL' : CheckSDL,
                                             'CheckLibLTDL' : CheckLibLTDL,
                                             'CheckConfigSuccess' : CheckConfigSuccess})

        
        if str(Platform()) == 'posix':
            print 'Configuring for POSIX system...'
        elif str(Platform()) == 'win32':
            print 'Configuring for WIN32 system...'
        else:
            print 'Configuring unknown system (assuming the system is POSIX-like) ...'

        ####################################################################
        # Configure GG requirements; use GG pkg-config first, if available #
        ####################################################################
        # pkg-config
        pkg_config = conf.CheckPkgConfig('0.15.0')

        found_gg_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('GiGiSDL', gigi_version):
                env.ParseConfig('pkg-config --cflags --libs GiGiSDL')
                found_gg_pkg_config = True

        if not found_gg_pkg_config:
            if OptionValue('boost_signals_namespace', env):
                signals_namespace = OptionValue('boost_signals_namespace', env)
                env.Append(CPPDEFINES = [
                    ('BOOST_SIGNALS_NAMESPACE', signals_namespace),
                    ('signals', signals_namespace)
                    ])

            boost_libs = [('boost_signals', 'boost::signals::connection', '#include <boost/signals.hpp>'),
                          ('boost_filesystem', 'boost::filesystem::initial_path', '#include <boost/filesystem/operations.hpp>')]
            if not conf.CheckBoost('1.32.0', boost_libs, conf, not ms_linker):
                Exit(1)

            # pthreads
            if str(Platform()) == 'posix':
                if env['multithreaded']:
                    if conf.CheckCHeader('pthread.h') and conf.CheckLib('pthread', 'pthread_create', autoadd = 0):
                        env.Append(CCFLAGS = ' -pthread')
                        env.Append(LINKFLAGS = ' -pthread')
                    else:
                        Exit(1)

            # GL and GLU
            if str(Platform()) == 'win32':
                env.Append(LIBS = [
                    'opengl32.lib',
                    'glu32.lib'
                    ])
            else:
                if not conf.CheckCHeader('GL/gl.h') or \
                       not conf.CheckCHeader('GL/glu.h') or \
                       not conf.CheckLib('GL', 'glBegin') or \
                       not conf.CheckLib('GLU', 'gluLookAt'):
                    Exit(1)

            # SDL
            sdl_config = WhereIs('sdl-config')
            if not conf.CheckSDL(options, conf, sdl_config, not ms_linker):
                Exit(1)

            # SDL_net (optional)
            if not env['with_builtin_sdlnet']:
                AppendPackagePaths('sdlnet', env)
                if not conf.CheckCHeader('SDL/SDL_net.h') and not conf.CheckCHeader('SDL_net.h') or \
                       not conf.CheckLib('SDL_net', 'SDLNet_Init'):
                    Exit(1)

            # FreeType2
            AppendPackagePaths('ft', env)
            found_it_with_pkg_config = False
            if pkg_config:
                if conf.CheckPkg('freetype2', ft_pkgconfig_version):
                    env.ParseConfig('pkg-config --cflags --libs freetype2')
                    found_it_with_pkg_config = True
            if not found_it_with_pkg_config:
                version_regex = re.compile(r'FREETYPE_MAJOR\s*(\d+).*FREETYPE_MINOR\s*(\d+).*FREETYPE_PATCH\s*(\d+)', re.DOTALL)
                if not conf.CheckVersionHeader('freetype2', 'freetype/freetype.h', version_regex, ft_version, True):
                    Exit(1)
            if not conf.CheckCHeader('ft2build.h'):
                Exit(1)
            if str(Platform()) != 'win32':
                if not conf.CheckLib('freetype', 'FT_Init_FreeType'):
                    Exit(1)
            else:
                env.Append(LIBS = [ft_win32_lib_name])

            # DevIL (aka IL)
            AppendPackagePaths('devil', env)
            version_regex = re.compile(r'IL_VERSION\s*(\d+)')
            if not conf.CheckVersionHeader('DevIL', 'IL/il.h', version_regex, devil_version, True, 'Checking DevIL version >= %s... ' % devil_version_string):
                Exit(1)
            if not conf.CheckCHeader('IL/il.h') or \
                   not conf.CheckCHeader('IL/ilu.h') or \
                   not conf.CheckCHeader('IL/ilut.h'):
                print "Note: since SDL support is disabled, the SDL headers are not in the compiler's search path during tests.  If DevIL was built with SDL support, this may cause the search for ilut.h to fail."
                Exit(1)
            if str(Platform()) != 'win32' and (not conf.CheckLib('IL', 'ilInit') or \
                                               not conf.CheckLib('ILU', 'iluInit') or \
                                               not conf.CheckLib('ILUT', 'ilutInit')):
                Exit(1)
            from sys import stdout
            stdout.write('Checking for DevIL OpenGL support... ')
            devil_gl_check_app = """
#include <IL/ilut.h>
#if !ILUT_USE_OPENGL
#error "DevIL not built with OpenGL support"
#endif
int main() {
    return 0;
}
"""
            if not conf.TryCompile(devil_gl_check_app, '.c'):
                print 'no'
            else:
                print 'yes'

            # ltdl
            if str(Platform()) != 'win32':
                if not conf.CheckLibLTDL():
                    print 'Check libltdl/config.log to see what went wrong.'
                    Exit(1)

        ##########################
        # End of GG requirements #
        ##########################

        # Log4cpp
        AppendPackagePaths('log4cpp', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('log4cpp', log4cpp_version):
                env.ParseConfig('pkg-config --cflags --libs log4cpp')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            version_regex = re.compile(r'LOG4CPP_VERSION\s*\"([^"]*)\"', re.DOTALL)
            if not conf.CheckVersionHeader('log4cpp', 'log4cpp/config.h', version_regex, log4cpp_version, False):
                Exit(1)
        if not conf.CheckCXXHeader('log4cpp/Category.hh'):
            Exit(1)
        if str(Platform()) != 'win32':
            if not conf.CheckLibWithHeader('log4cpp', 'log4cpp/Category.hh', 'C++', 'log4cpp::Category::getRoot();'):
                Exit(1)

        # FMOD
        AppendPackagePaths('fmod', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('fmod', fmod_version):
                env.ParseConfig('pkg-config --cflags --libs fmod')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            version_regex = re.compile(r'FMOD_VERSION\s*(\d+\.\d+)', re.DOTALL)
            if not conf.CheckVersionHeader('fmod', 'fmod.h', version_regex, fmod_version, True):
                Exit(1)
        if not conf.CheckCHeader('fmod.h'):
            Exit(1)
        if str(Platform()) != 'win32':
            if not conf.CheckLib('fmod-' + fmod_version, 'FSOUND_GetVersion', header = '#include <fmod.h>'):
                Exit(1)
        else:
            env.Append(LIBS = [fmod_win32_lib_name])

        # GraphViz
        AppendPackagePaths('graphviz', env)
        if pkg_config:
            if conf.CheckPkg('graphviz', graphviz_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs graphviz')
        if not conf.CheckCHeader('graphviz/dot.h') and not conf.CheckCHeader('dot.h'):
            Exit(1)
        env.Append(LIBS = [
            'cdt',
            'common',
            'dotgen',
            'dotneato',
            'graph',
            'gvrender',
            'pathplan'
            ])
        if str(Platform()) != 'win32':
            old_libs = env['LIBS']
            if not conf.CheckLib('dotgen', 'begin_component', header = '#include <graphviz/dot.h>'):
                Exit(1)

        # GG
        AppendPackagePaths('gg', env)
        if env['with_gg_include']:
            env.Append(CPPPATH = [
                os.path.normpath(os.path.join(env['with_gg_include'], 'net')),
                os.path.normpath(os.path.join(env['with_gg_include'], 'SDL')),
                os.path.normpath(os.path.join(env['with_gg_include'], 'dialogs'))
                ])

        # zlib
        if str(Platform()) == 'win32':
            AppendPackagePaths('zlib', env)

        # finish config and save results for later
        conf.CheckConfigSuccess(True)
        conf.Finish();
        f = open('config.cache', 'w')
        p = pickle.Pickler(f)
        cache_dict = {}
        for i in env_cache_keys:
            cache_dict[i] = env.has_key(i) and env.Dictionary(i) or []
        p.dump(cache_dict)
        if 'configure' in command_line_args:
            Exit(0)

##################################################
# define targets                                 #
##################################################
if env['release']:
    env.Append(CPPDEFINES = [
        'FREEORION_RELEASE'
        ])
if str(Platform()) == 'win32':
    env.Append(CPPDEFINES = [
        'FREEORION_WIN32'
        ])
else:
    env.Append(CPPDEFINES = [
        'FREEORION_LINUX',
        'ENABLE_BINRELOC'
        ])

if str(Platform()) == 'win32':
    if env['multithreaded']:
        if env['dynamic']:
            if env['debug']:
                code_generation_flag = '/MDd'
            else:
                code_generation_flag = '/MD'
        else:
            if env['debug']:
                code_generation_flag = '/MTd'
            else:
                code_generation_flag = '/MT'
    else:
        if env['debug']:
            code_generation_flag = '/MLd'
        else:
            code_generation_flag = '/ML'
    flags = [
        #env['debug'] and '/Od' or '/Ox',
        code_generation_flag,
        '/EHsc',
        '/W3',
        '/Zc:forScope',
        '/GR',
        '/Gd',
        '/Zi',
        '/wd4099', '/wd4251', '/wd4800', '/wd4267', '/wd4275', '/wd4244', '/wd4101', '/wd4258'
        ]
    env.Append(CCFLAGS = flags)
    env.Append(CPPDEFINES = [
        '_DEBUG',
        'WIN32',
        '_WINDOWS'
        ])
    if env['dynamic']:
        env.Append(CPPDEFINES = [
        '_USRDLL',
        '_WINDLL'
        ])
    env.Append(LINKFLAGS = ['/SUBSYSTEM:WINDOWS', '/DEBUG'])
    env.Append(LIBS = [
        'comdlg32',
        'gd',
        'gdi32',
        'GiGi',
        'GiGiNet',
        'GiGiSDL',
        'glu32',
        'jpeg',
        'kernel32',
        'libexpat',
        'log4cpp',
        'opengl32',
        'png',
        'SDL',
        'SDLmain',
        'user32',
        'winspool',
        'wsock32',
        'zdll'
        ])
else:
    if env['debug']:
        env.Append(CCFLAGS = ['-Wall', '-g', '-O0'])
    else:
        env.Append(CCFLAGS = ['-Wall', '-O2'])

Export('env')

# define server objects
env['target_define'] = 'FREEORION_BUILD_SERVER'
server_objects = SConscript(os.path.normpath('SConscript'))
freeoriond = env.Program("freeoriond", server_objects)

# define ai objects
env['target_define'] = 'FREEORION_BUILD_AI'
ai_objects = SConscript(os.path.normpath('SConscript'))
freeorionca = env.Program("freeorionca", ai_objects)

# define human objects
env['target_define'] = 'FREEORION_BUILD_HUMAN'
human_objects = SConscript(os.path.normpath('SConscript'))
freeorion = env.Program("freeorion", human_objects)

# install target
Alias('install', Install(env['bindir'], freeoriond))
Alias('install', Install(env['bindir'], freeorionca))
Alias('install', Install(env['bindir'], freeorion))

# uninstall target
# This is a dirty hack, used here because I don't know how else to do this.  Basically, I've created a Command that
# deletes the uninstallable files.  To keep it dependent on its target and source arguments, I made it depend on
# SConstruct for its source, since it is guaranteed to exist, and a target with a filename which is pretty well
# guaranteed not to exist, and which is in fact never created.
deletions = [
    Delete(os.path.normpath(os.path.join(env['bindir'], str(freeoriond[0])))),
    Delete(os.path.normpath(os.path.join(env['bindir'], str(freeorionca[0])))),
    Delete(os.path.normpath(os.path.join(env['bindir'], str(freeorion[0]))))
    ]
uninstall_cmd = env.Command('.unlikely_filename934765437', 'SConstruct', deletions)
Alias('uninstall', uninstall_cmd)

# default targets
Default(freeoriond, freeorionca, freeorion)

if OptionValue('scons_cache_dir', env):
    CacheDir(OptionValue('scons_cache_dir', env))
