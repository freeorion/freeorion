#!/usr/bin/python

import sys
import os
from string import Template
from subprocess import check_output
from datetime import datetime
from platform import system

if 3 != len(sys.argv):
    print "ERROR: invalid parameters."
    print "make_versioncpp.py <project rootdir> <build system name>"
    quit()

os.chdir(sys.argv[1])
build_sys = sys.argv[2]
infile = 'util/Version.cpp.in'
outfile = 'util/Version.cpp'

version = "0.4.4+"
build_no = "???"

try:
    branch = check_output(['git', 'rev-parse', '--abbrev-ref', 'HEAD']).strip()
    if branch == "master":
        branch = ""
    commit = check_output(["git", "show", "-s", "--format=%h", "HEAD"]).strip()
    timestamp = float(check_output(["git", "show", "-s", "--format=%ct", "HEAD"]).strip())
    build_no = ".".join([datetime.utcfromtimestamp(timestamp).strftime("%Y%m%d"), commit])
except:
    print "WARNING: git not installed"

if build_no == "???" and os.path.exists(outfile):
   print "WARNING: Can't determine git commit, %s not updated!" % outfile
   quit()

try:
    template_file = open(infile)
    template = Template(template_file.read())
    template_file.close()
except:
    print "WARNING: Can't access %s, %s not updated!" % (infile, outfile)
    quit()

print "Writing file: %s" % outfile

version_cpp = open(outfile, "w")
version_cpp.write(template.substitute(
    FreeOrion_VERSION=version,
    FreeOrion_BRANCH=branch,
    FreeOrion_BUILD_NO=build_no,
    FreeOrion_BUILDSYS=build_sys))
version_cpp.close()

if system() == "Windows":
    infile = "Installer/FreeOrion_Install_Script.nsi.in"
    outfile = "Installer/FreeOrion_Install_Script.nsi"
    try:
        template_file = open(infile)
        template = Template(template_file.read())
        template_file.close()
    except:
        print "WARNING: Can't access %s, %s not updated!" % (infile, outfile)
        quit()

    print "Writing file: %s" % outfile

    installer_script = open(outfile, "w")
    installer_script.write(template.substitute(
        FreeOrion_VERSION=version,
        FreeOrion_BUILD_NO=build_no))
    installer_script.close()

if system() == "Darwin":
    infile = "Xcode/Info.plist.in"
    outfile = "Xcode/Info.plist"
    try:
        template_file = open(infile)
        template = Template(template_file.read())
        template_file.close()
    except:
        print "WARNING: Can't access %s, %s not updated!" % (infile, outfile)
        quit()

    print "Writing file: %s" % outfile

    info_plist = open(outfile, "w")
    info_plist.write(template.substitute(
        FreeOrion_VERSION=version,
        FreeOrion_BRANCH=branch,
        FreeOrion_BUILD_NO=build_no))
    info_plist.close()

print "Building v%s build %s" %(version, build_no)
