#!/usr/bin/env python2


import sys
import os
from string import Template
from datetime import datetime
from subprocess import check_output
from platform import system
from glob import glob


INVALID_BUILD_NO = "???"

required_boost_libraries = [
    "boost_chrono",
    "boost_date_time",
    "boost_filesystem",
    "boost_iostreams",
    "boost_locale",
    "boost_log",
    "boost_log_setup",
    "boost_python",
    "boost_regex",
    "boost_serialization",
    "boost_signals",
    "boost_system",
    "boost_thread",
]


class Generator(object):
    def __init__(self, infile, outfile):
        self.infile = infile
        self.outfile = outfile

    def compile_output(self, template, version, branch, build_no, build_sys):
        return template.substitute(
            FreeOrion_VERSION=version,
            FreeOrion_BRANCH=branch,
            FreeOrion_BUILD_NO=build_no,
            FreeOrion_BUILDSYS=build_sys)

    def execute(self, version, branch, build_no, build_sys):
        if build_no == INVALID_BUILD_NO:
            print "WARNING: Can't determine git commit!"

        if os.path.isfile(self.outfile):
            with open(self.outfile) as check_file:
                check_file_contents = check_file.read()
                if build_no == INVALID_BUILD_NO:
                    if version in check_file_contents:
                        print "Version matches version in existing Version.cpp, skip regenerating it"
                        return
                elif build_no in check_file_contents:
                    print "Build number matches build number in existing Version.cpp, skip regenerating it"
                    return

        try:
            with open(self.infile) as template_file:
                template = Template(template_file.read())
        except:
            print "WARNING: Can't access %s, %s not updated!" % (self.infile, self.outfile)
            return

        print "Writing file: %s" % self.outfile
        with open(self.outfile, "w") as generated_file:
            generated_file.write(self.compile_output(template, version, branch, build_no, build_sys))


class NsisInstScriptGenerator(Generator):
    def compile_dll_list(self):
        all_dll_files = glob("*.dll")
        accepted_dll_files = set(["GiGi.dll", "GiGiSDL.dll"])
        for dll_file in all_dll_files:
            if dll_file.startswith("boost_"):
                if dll_file.partition(".")[0] in required_boost_libraries:
                    accepted_dll_files.add(dll_file)
            else:
                accepted_dll_files.add(dll_file)
        return sorted(accepted_dll_files)

    def compile_output(self, template, version, branch, build_no, build_sys):
        dll_files = self.compile_dll_list()
        if dll_files:
            return template.substitute(
                FreeOrion_VERSION=version,
                FreeOrion_BRANCH=branch,
                FreeOrion_BUILD_NO=build_no,
                FreeOrion_BUILDSYS=build_sys,
                FreeOrion_DLL_LIST_INSTALL="\n  ".join(['File "..\\' + fname + '"' for fname in dll_files]),
                FreeOrion_DLL_LIST_UNINSTALL="\n  ".join(['Delete "$INSTDIR\\' + fname + '"' for fname in dll_files]))
        else:
            print "WARNING: no dll files for installer package found"
            return template.substitute(
                FreeOrion_VERSION=version,
                FreeOrion_BRANCH=branch,
                FreeOrion_BUILD_NO=build_no,
                FreeOrion_BUILDSYS=build_sys,
                FreeOrion_DLL_LIST_INSTALL="",
                FreeOrion_DLL_LIST_UNINSTALL="")


if 3 != len(sys.argv):
    print "ERROR: invalid parameters."
    print "make_versioncpp.py <project rootdir> <build system name>"
    quit()

os.chdir(sys.argv[1])
build_sys = sys.argv[2]

# A list of tuples containing generators
generators = [
    Generator('util/Version.cpp.in', 'util/Version.cpp')
]
if system() == 'Windows':
    generators.append(NsisInstScriptGenerator('Installer/FreeOrion_Install_Script.nsi.in',
                                              'Installer/FreeOrion_Install_Script.nsi'))
if system() == 'Darwin':
    generators.append(Generator('Xcode/Info.plist.in', 'Xcode/Info.plist'))

version = "0.4.8+"
branch = ""
build_no = INVALID_BUILD_NO

try:
    branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
    if (branch == "master") or (branch[:7] == "release"):
        branch = ""
    else:
        branch += " "
    commit = check_output(["git", "show", "-s", "--format=%h", "--abbrev=7", "HEAD"]).strip()
    timestamp = float(check_output(["git", "show", "-s", "--format=%ct", "HEAD"]).strip())
    build_no = ".".join([datetime.utcfromtimestamp(timestamp).strftime("%Y-%m-%d"), commit])
except:
    print "WARNING: git not installed or not setup correctly"

for generator in generators:
    generator.execute(version, branch, build_no, build_sys)

print "Building v%s %sbuild %s" % (version, branch, build_no)
