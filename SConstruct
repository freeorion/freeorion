# -*- Python -*-

import os
import pickle
import re
from build_config import *
from build_support import *

env = Environment()
# Do not put a .sconsign file into every directory
env.SConsignFile()

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
if WhereIs('ccache'):
    options.Add(BoolOption('use_ccache', 'Use ccache to build GG', 0))
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
options.Add('with_openal', 'Root directory of OpenAL installation')
options.Add('with_openal_include', 'Specify exact include dir for OpenAL headers')
options.Add('with_openal_libdir', 'Specify exact library dir for OpenAL library')
options.Add('with_alut', 'Root directory of ALUT/FreeALUT installation')
options.Add('with_alut_include', 'Specify exact include dir for ALUT/FreeALUT headers')
options.Add('with_alut_libdir', 'Specify exact library dir for ALUT/FreeALUT library')
options.Add('with_vorbis', 'Root directory of Vorbis installation')
options.Add('with_vorbis_include', 'Specify exact include dir for Vorbis headers')
options.Add('with_vorbis_libdir', 'Specify exact library dir for Vorbis library')
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

if env.has_key('use_ccache') and env['use_ccache']:
    env['CC'] = 'ccache %s' % env['CC']
    env['CXX'] = 'ccache %s' % env['CXX']
    for i in ['HOME',
              'CCACHE_DIR',
              'CCACHE_TEMPDIR',
              'CCACHE_LOGFILE',
              'CCACHE_PATH',
              'CCACHE_CC',
              'CCACHE_PREFIX',
              'CCACHE_DISABLE',
              'CCACHE_READONLY',
              'CCACHE_CPP2',
              'CCACHE_NOSTATS',
              'CCACHE_NLEVELS',
              'CCACHE_HARDLINK',
              'CCACHE_RECACHE',
              'CCACHE_UMASK',
              'CCACHE_HASHDIR',
              'CCACHE_UNIFY',
              'CCACHE_EXTENSION']:
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

        # Python
        import distutils.sysconfig
        if str(Platform()) == 'win32':
            env['with_python'] = distutils.sysconfig.PREFIX
            env['with_python_include'] = os.path.join(env['with_python'], 'include')
            env['with_python_libdir'] = os.path.join(env['with_python'], 'libs')
            AppendPackagePaths('python', env)
            env.AppendUnique(LIBS = [
                python_win32_libname
                ])
        else:
            env['with_python'] = ''
            env['with_python_include'] = distutils.sysconfig.get_python_inc()
            env['with_python_libdir'] = distutils.sysconfig.get_config_var('LIBDIR')
            AppendPackagePaths('python', env)
            regex = re.compile(r'lib(.+)\.so.*')
            matches = regex.findall(distutils.sysconfig.get_config_var('LDLIBRARY'))
            if len(matches):
                python_libname = matches[0]
            else:
                print 'Unable to determine the name of the Python runtime library.  Terminating....'
                Exit(1)
            if not conf.CheckLibWithHeader(python_libname, 'Python.h', 'C', 'Py_Initialize();'):
                Exit(1)

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

        freeorion_boost_libs = [
            ('boost_serialization', 'boost/archive/binary_iarchive.hpp', 'boost::archive::binary_iarchive::is_saving();'),
            ('boost_iostreams', 'boost/iostreams/filtering_stream.hpp', ''),
            ('boost_python', 'boost/python.hpp', 'boost::python::throw_error_already_set();')
            ]

        if found_gg_pkg_config:
            if not conf.CheckBoost(boost_version_string, freeorion_boost_libs, conf, not ms_linker):
                Exit(1)
        else:
            if OptionValue('boost_signals_namespace', env):
                signals_namespace = OptionValue('boost_signals_namespace', env)
                env.AppendUnique(CPPDEFINES = [
                    ('BOOST_SIGNALS_NAMESPACE', signals_namespace),
                    ('signals', signals_namespace)
                    ])

            boost_libs = freeorion_boost_libs + [
                ('boost_signals', 'boost/signals.hpp', 'boost::signals::connection();'),
                ('boost_filesystem', 'boost/filesystem/operations.hpp', 'boost::filesystem::initial_path();')
                ]
            if not conf.CheckBoost(boost_version_string, boost_libs, conf, not ms_linker):
                Exit(1)
            if str(Platform()) == 'win32' and ms_linker:
                env.AppendUnique(LIBS = [
                    BoostLibWin32Name('python', env)
                    ])

            # pthreads
            if str(Platform()) == 'posix':
                if env['multithreaded']:
                    if conf.CheckCHeader('pthread.h') and conf.CheckLib('pthread', 'pthread_create', autoadd = 0):
                        env.AppendUnique(CCFLAGS = ' -pthread')
                        env.AppendUnique(LINKFLAGS = ' -pthread')
                    else:
                        Exit(1)

            # GL and GLU
            if str(Platform()) == 'win32':
                env.AppendUnique(LIBS = [
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
                env.AppendUnique(LIBS = [ft_win32_lib_name])

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
#ifndef ILUT_USE_OPENGL
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

        found_it_with_pkg_config = False

        # OpenAL & ALUT/FreeALUT
        AppendPackagePaths('openal', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('openal', openal_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs openal')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            if str(Platform()) == 'win32':
                env.AppendUnique(LIBS = [
                    'OpenAL32.lib'
                    ])
            else:
                if not conf.CheckLibWithHeader('openal', 'AL/alc.h', 'C', 'alcGetCurrentContext();'):
                    Exit(1)
        AppendPackagePaths('alut', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('freealut', alut_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs freealut')
                found_it_with_pkg_config = True
            elif conf.CheckPkg('alut', alut_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs alut')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            if str(Platform()) == 'win32':
                env.AppendUnique(LIBS = [
                    'ALut.lib'
                    ])
            else:
                if not conf.CheckLibWithHeader('freealut', 'AL/alut.h', 'C', 'alutInitWithoutContext(0,0);') \
                       and not conf.CheckLibWithHeader('alut', 'AL/alut.h', 'C', 'alutInitWithoutContext(0,0);'):
                    Exit(1)

        # Vorbis
        AppendPackagePaths('vorbisfile', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('vorbisfile', vorbis_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs vorbisfile')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            if str(Platform()) == 'win32':
                env.AppendUnique(LIBS = [
                    'libogg.lib',
                    'libvorbis.lib',
                    'libvorbisfile.lib'
                    ])
            else:
                if not conf.CheckLibWithHeader('vorbisfile', 'vorbis/vorbisfile.h', 'C', 'ov_clear(0);'):
                    Exit(1)

        # GraphViz
        AppendPackagePaths('graphviz', env)
        found_it_with_pkg_config = False
        if pkg_config:
            if conf.CheckPkg('libgraph', graphviz_pkgconfig_version) and conf.CheckPkg('libgvc', graphviz_pkgconfig_version):
                env.ParseConfig('pkg-config --cflags --libs libgraph')
                env.ParseConfig('pkg-config --cflags --libs libgvc')
                found_it_with_pkg_config = True
        if not found_it_with_pkg_config:
            if not conf.CheckCHeader('gvc.h'):
                Exit(1)
            if str(Platform()) == 'win32':
                env.AppendUnique(LIBS = [
                    'cdt',
                    'circogen',
                    'common',
                    'dotgen',
                    'fdpgen',
                    'gd',
                    'graph',
                    'gvc',
                    'libexpat',
                    'neatogen',
                    'pack',
                    'pathplan',
                    'plugin',
                    'twopigen'
                    ])
            else:
                if not conf.CheckLib('gvc', 'gvContext', header = '#include <gvc.h>'):
                    Exit(1)
                env.AppendUnique(LIBS = [
                    'cdt',
                    'graph',
                    'gvc'
                    ])

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

        # GG
        AppendPackagePaths('gg', env)

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
    env.AppendUnique(CPPDEFINES = [
        'FREEORION_RELEASE'
        ])
if str(Platform()) == 'win32':
    env.AppendUnique(CPPDEFINES = [
        'FREEORION_WIN32'
        ])
else:
    env.AppendUnique(CPPDEFINES = [
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
        '/wd4099', '/wd4251', '/wd4800', '/wd4267', '/wd4275', '/wd4244', '/wd4101', '/wd4258', '/wd4351', '/wd4996'
        ]
    env.AppendUnique(CCFLAGS = flags)
    env.AppendUnique(CPPDEFINES = [
        (env['debug'] and '_DEBUG' or 'NDEBUG'),
        'WIN32',
        '_WINDOWS'
        ])
    if env['dynamic']:
        env.AppendUnique(CPPDEFINES = [
        '_USRDLL',
        '_WINDLL'
        ])
    env.AppendUnique(LINKFLAGS = ['/SUBSYSTEM:CONSOLE', '/DEBUG'])
    env.AppendUnique(LIBS = [
        'comdlg32',
        'gdi32',
        'GiGi',
        'GiGiNet',
        'GiGiSDL',
        'glu32',
        'jpeg',
        'kernel32',
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
        env.AppendUnique(CCFLAGS = ['-Wall', '-g', '-O0'])
    else:
        env.AppendUnique(CCFLAGS = ['-Wall', '-O2'])

# generate Version.cpp
version_cpp_in = open('util/Version.cpp.in', 'r')
version_cpp = open('util/Version.cpp', 'w')
values = {
    "freeorion_version" : freeorion_version,
    "freeorion_repository_revision" : GetRepositoryRevision()
    }
version_cpp.write(version_cpp_in.read() % values)
version_cpp.close()
version_cpp_in.close()

# On Win32, assume we're using the SDK, and copy the installed GG DLLs to the FreeOrion directory.
if str(Platform()) == 'win32':
    import shutil
    shutil.copy(os.path.join('..', 'lib', 'GiGi.dll'), '.')
    shutil.copy(os.path.join('..', 'lib', 'GiGiNet.dll'), '.')
    shutil.copy(os.path.join('..', 'lib', 'GiGiSDL.dll'), '.')

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
if str(Platform()) == 'win32':
    rc_file = open('win32_resources.rc', 'w')
    rc_file.write('IDI_ICON ICON "client/human/HumanClient.ico"')
    rc_file.close()
    env.RES('win32_resources.rc', CPPPATH = [], CPPDEFINES = [])
    env.Depends('win32_resources.res', 'client/human/HumanClient.ico')
    env.Command('icon.rbj', 'win32_resources.res', ['cvtres /out:icon.rbj /machine:ix86 win32_resources.res'])
    freeorion = env.Program("freeorion", human_objects + ['icon.rbj'])
else:
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
