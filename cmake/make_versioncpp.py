#!/usr/bin/python

import sys
import os
from string import Template
from subprocess import check_output
from platform import system
from glob import glob


def default_generator(generated_file, version, branch, build_no, build_sys):
    generated_file.write(template.substitute(
        FreeOrion_VERSION=version,
        FreeOrion_BRANCH=branch,
        FreeOrion_BUILD_NO=build_no,
        FreeOrion_BUILDSYS=build_sys))


def nsis_inst_script_generator(generated_file, version, branch, build_no, build_sys):
    dll_files = glob("boost_*.dll")
    if not dll_files:
        print "No boost dll libraries found, NSIS installer script not updated"
        return
    dll_version = dll_files[0].partition("-")[2].rpartition(".")[0]
    if dll_version:
        print "Boost DLL version:", dll_version
    else:
        print "Can't determine boost dll version, NSIS installer script not updated"
    generated_file.write(template.substitute(
        FreeOrion_VERSION=version,
        FreeOrion_BRANCH=branch,
        FreeOrion_BUILD_NO=build_no,
        FreeOrion_BUILDSYS=build_sys,
        Boost_DLL_VERSION=dll_version))


if 3 != len(sys.argv):
    print "ERROR: invalid parameters."
    print "make_versioncpp.py <project rootdir> <build system name>"
    quit()

os.chdir(sys.argv[1])
build_sys = sys.argv[2]

# A list of tuples containing generator function, input and output files.
# Tuple index 0 contains generator function
# Tuple index 1 contains the input file.
# Tuple index 2 contains the output file.
io_files = [(default_generator, 'util/Version.cpp.in', 'util/Version.cpp')]

if system() == 'Windows':
    io_files.append((
        nsis_inst_script_generator,
        'Installer/FreeOrion_Install_Script.nsi.in',
        'Installer/FreeOrion_Install_Script.nsi'))

if system() == 'Darwin':
    io_files.append((
        default_generator,
        'Xcode/Info.plist.in',
        'Xcode/Info.plist'))

version = "0.4.4+"
build_no = "???"

try:
    branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
    if branch == "master":
        branch = ""
    build_no = check_output(['git', 'show', '-s', '--format=%cd.%h', '--date=short', 'HEAD']).strip()
except:
    print "WARNING: git not installed"


for generator, infile, outfile in io_files:
    if os.path.isfile(outfile):
        if build_no == "???":
            print "WARNING: Can't determine git commit, %s not updated!" % outfile
            continue
        with open(outfile) as check_file:
            if build_no in check_file.read():
                pass # continue

    try:
        template_file = open(infile)
        template = Template(template_file.read())
        template_file.close()
    except:
        print "WARNING: Can't access %s, %s not updated!" % (infile, outfile)
        continue

    print "Writing file: %s" % outfile

    with open(outfile, "w") as f:
        generator(f, version, branch, build_no, build_sys)

print "Building v%s build %s" %(version, build_no)
