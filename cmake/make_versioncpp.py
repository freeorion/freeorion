#!/usr/bin/python
import sys
import os
import subprocess as sp
from string import Template

if 4 != len(sys.argv):
    print "ERROR: invalid parameters."
    print "make_versioncpp.py <project rootdir> <build system name> <outfile>"
    quit()

os.chdir(sys.argv[1])
build_sys = sys.argv[2]
outfile = sys.argv[3]

version = "0.4.2"
wc_rev = "???"

try:
    svn_proc = sp.Popen(["svn", "info"], stdout=sp.PIPE)
    svn_info = svn_proc.communicate()[0]

    for line in svn_info.splitlines():
        if line.startswith("Revision:"):
            wc_rev = line.partition(" ")[2]
except:
    try:
        svn_proc = sp.Popen(["SubWCRev.exe", "."], stdout=sp.PIPE)
        svn_info = svn_proc.communicate()[0]

        for line in svn_info.splitlines():
            if line.startswith("Updated to revision"):
                wc_rev = line.rpartition(" ")[2]
    except:
        print "WARNING: No properly installed SVN client found"

if wc_rev == "???" and os.path.exists(outfile):
	print "WARNING: Can't determine SVN working copy revision, %s not updated!" % outfile
	quit()

try:
    template_file = open('cmake/Version.cpp.in')
    template = Template(template_file.read())
    template_file.close()
except:
    print "WARNING: Can't access cmake/Version.cpp.in, %s not updated!" % outfile
    quit()

version_cpp = open(outfile, "w")
version_cpp.write(template.substitute(
    FreeOrion_VERSION=version,
    FreeOrion_WC_REVISION=wc_rev,
    FreeOrion_BUILDSYS=build_sys))
version_cpp.close()

print "Building rev: " + wc_rev
