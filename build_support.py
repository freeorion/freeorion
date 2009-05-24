# -*- Python -*-

import os
import re
from build_config import *

def OptionValue(key, env):
    return env.subst('$' + key)

def GenerateHelpText(options, env):
    import textwrap
    retval = ''
    lines = [
        '',
        'Targets:',
        '========',
        '',
        'The default target builds freeorion, freeorionca and freeoriond based on the options you specify.'
        '',
        'install',
        'Installs the default targets in the installation directory, building it first if needed.',
        '',
        'uninstall',
        'Removes the files installed by the "install" targtet from the installation directory.',
        '',
        '',
        'Options:',
        '========',
        '',
        'Each of the following options should be specified with an equals sign (e.g., foo=bar).  Dashes are not required.  Once an option has been specified, it is saved in options.cache for later use; this means an option only needs to be set once.  To change or remove a single option, edit the file.  To clear all options delete the file.',
        ''
        ]
    for i in lines:
        if not i:
            retval += '\n'
        else:
            wrapped_text = textwrap.wrap(i,
                                         width = 78)
            for j in wrapped_text:
                retval += j + '\n'
    for i in options.options:
        retval += i.key + '\n'
        wrapped_text = textwrap.wrap(i.help,
                                     width = 78,
                                     initial_indent = '    ',
                                     subsequent_indent = '    ')
        for j in wrapped_text:
            retval += j + '\n'
        retval += '    value=' + str(OptionValue(i.key, env)) + '\n'
        retval += '    default=' + str(i.default) + '\n\n'
    return retval

def DirHeaders(dir):
    from fnmatch import fnmatchcase
    return [i for i in os.listdir(dir) if fnmatchcase(i, '*.h')]

def AppendPackagePaths(package, env):
    root = OptionValue('with_' + package, env)
    inc = OptionValue('with_%s_include' % package, env)
    if not inc and root:
        inc =  os.path.normpath(os.path.join(root, 'include'))
    lib = OptionValue('with_%s_libdir' % package, env)
    if not lib and root:
        lib = os.path.normpath(os.path.join(root, 'lib'))
    if inc:
        env.AppendUnique(CPPPATH = [os.path.abspath(inc)])
    if lib:
        env.AppendUnique(LIBPATH = [os.path.abspath(lib)])

def CheckPkgConfig(context, version):
    context.Message('Checking for pkg-config... ')
    ret = context.TryAction('pkg-config --atleast-pkgconfig-version %s' % version)[0]
    context.Result(ret)
    return ret

def CheckPkg(context, name, version):
    context.Message('Checking for %s >= %s... ' % (name, version))
    ret = context.TryAction('pkg-config %s --atleast-version %s' % (name, version))[0]
    context.Result(ret)
    return ret

def VersionLEQ(lhs, rhs):
    lhs = lhs.split('.')
    rhs = rhs.split('.')
    try:
        for i in range(len(lhs)):
            if int(rhs[i]) < int(lhs[i]):
                return False
            elif int(lhs[i]) < int(rhs[i]):
                return True
    except Exception:
        for j in range(i, len(lhs)):
            if int(lhs[j]):
                return False
        return True
    return True

def FindRegexMatchesInHeader(regex, filename, env = None):
    f = None
    if env and env.has_key('CPPPATH'):
        for i in env['CPPPATH']:
            try:
                f = open(os.path.normpath(os.path.join(i, filename)), 'r')
                break
            except Exception:
                pass
    if not f:
        for i in ['.', '/usr/include', '/usr/local/include']:
            try:
                f = open(os.path.normpath(os.path.join(i, filename)), 'r')
                break
            except Exception:
                pass
    if f:
        return regex.findall(f.read())
    else:
        return []

def CheckVersionHeader(context, package, header, regex, comparison_string, version_leq_check, message = None):
    retval = False
    if message:
        context.Message(message)
    else:
        if version_leq_check:
            context.Message('Checking %s version >= %s... ' % (package, comparison_string))
        else:
            context.Message('Checking %s version == %s... ' % (package, comparison_string))
    matches = FindRegexMatchesInHeader(regex, header, context.env)
    if len(matches) == 1:
        import types
        if type(matches[0]) == types.StringType:
            match_str = matches[0]
        else:
            match_str = '.'.join(matches[0])
        if version_leq_check:
            retval = VersionLEQ(comparison_string, match_str)
        else:
            retval = comparison_string == match_str
    context.Result(retval and 'yes' or 'no')
    return retval

def CheckBoostLib(context, lib_tuple, conf):
    ret = ""
    lib_name = lib_tuple[0]
    suffix = OptionValue('boost_lib_suffix', context.env)
    if suffix:
        lib_name += suffix
    print 'Looking for boost lib %s... ' % lib_name
    if conf.CheckLibWithHeader(lib_name, lib_tuple[1], 'C++', lib_tuple[2]):
        ret = lib_name
    else:
        lib_name_mt = lib_name + '-mt'
        if conf.CheckLibWithHeader(lib_name_mt, lib_tuple[1], 'C++', lib_tuple[2]):
            ret = lib_name_mt
        else:
            lib_name_py = lib_name + '-py25'
            if conf.CheckLibWithHeader(lib_name_py, lib_tuple[1], 'C++', lib_tuple[2]):
                ret = lib_name_py        

    return ret

def CheckBGL(context, conf):
    if not conf.CheckHeader('boost/graph/dijkstra_shortest_paths.hpp', '<>', 'C++'):
        context.Message('Boost configuration... ')
        context.Result(False)
        return False
    else:
        return True

def CheckBoost(context, required_version, lib_tuples, conf, check_libs):
    AppendPackagePaths('gg', context.env)
    AppendPackagePaths('boost', context.env)
    if not conf.CheckCXXHeader('boost/shared_ptr.hpp'):
        context.Message('Boost configuration... ')
        context.Result(False)
        return False
    version_regex = re.compile(r'BOOST_VERSION\s*(\d+)')
    if not CheckVersionHeader(context, 'Boost', 'boost/version.hpp', version_regex, boost_version, True, 'Checking Boost version >= %s... ' % boost_version_string):
        context.Message('Boost configuration... ')
        context.Result(False)
        return False
    if check_libs:
        if not CheckBGL(context, conf):
            return False
        for i in lib_tuples:
            lib = CheckBoostLib(context, i, conf)
            if not lib:
                context.Message('Boost configuration... ')
                context.Result(False)
                return False
    context.Message('Boost configuration... ')
    context.Result('ok')
    return True

def BoostLibWin32Name(name, env):
    # For now, assume VC90 is used
    toolset_tag = '-vc90'
    if env['multithreaded']:
        if env['dynamic']:
            if env['debug']:
                threading_and_abi_tag = '-mt-gd'
            else:
                threading_and_abi_tag = '-mt'
        else:
            if env['debug']:
                threading_and_abi_tag = '-mt-sgd'
            else:
                threading_and_abi_tag = '-mt-sd'
    else:
        if env['debug']:
            threading_and_abi_tag = '-sgd'
        else:
            threading_and_abi_tag = '-sd'
    version_tag = '-' + boost_version_string.replace('.', '_')
    return 'boost_' + name + toolset_tag + threading_and_abi_tag + version_tag

def CheckLibLTDL(context):
    retval = True
    context.Message('Generating GG/libltdl/config.h using GG/libltdl/configure... ')
    initial_dir = os.getcwd()
    os.chdir(os.path.join('GG', 'libltdl'))
    configure_run = os.system('./configure > /dev/null')
    code = os.WEXITSTATUS(configure_run)
    if code:
        retval = False
    os.chdir(initial_dir)
    context.Result(retval and 'ok' or 'failed')
    return retval

def CheckConfigSuccess(context, complete):
    context.Message('Configuration successful... ')
    context.Result(complete and 'yes' or 'no')
    return complete

def TraverseHeaderTree(dir, header_root, headers, current_path, op, nodes):
    if headers[0]:
        current_path.append(headers[0])
    sub_path = ""
    for i in current_path:
        sub_path = os.path.normpath(os.path.join(sub_path, i))
    for i in range(1, len(headers)):
        if isinstance(headers[i],list):
            TraverseHeaderTree(dir, header_root, headers[i], current_path, op, nodes)
        else:
            nodes.append(op(os.path.normpath(os.path.join(dir, sub_path)),
                            os.path.normpath(os.path.join(header_root, sub_path, headers[i]))))
    if headers[0]:
        current_path.pop()

def InstallHeaderTree(dir, header_root, headers, current_path, op):
    install_nodes = []
    TraverseHeaderTree(dir, header_root, headers, current_path, op, install_nodes)
    return install_nodes

def GetRepositoryRevision():
    "Try to determine the current revision from SVN."
    try:
        from os import popen
        inf = popen("svn info")
        for i in inf:
            if i.startswith("Revision: "):
                return ' [Rev ' + i[10:-1] + ']'
    except: 
        return None
