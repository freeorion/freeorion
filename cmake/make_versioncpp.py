#!/usr/bin/python
import sys
import os
import subprocess as sp
from string import Template

if 3 != len(sys.argv):
    print "ERROR: invalid parameters."
    print "make_versioncpp.py <project rootdir> <build system name>"
    quit()

os.chdir(sys.argv[1])
build_sys = sys.argv[2]
infile = 'util/Version.cpp.in'
outfile = 'util/Version.cpp'

version = "0.4.4+"
wc_rev = "???"

try:
    git_proc = sp.Popen(["git", "describe", "--always"], stdout=sp.PIPE)
    wc_rev = git_proc.communicate()[0].strip()

except:
    try:
        git_proc = sp.Popen(["SubWCRev.exe", "."], stdout=sp.PIPE)
        git_info = git_proc.communicate()[0]

        for line in git_info.splitlines():
            if line.startswith("Last committed at revision"):
                wc_rev = line.rpartition(" ")[2]
    except:
        print "WARNING: No properly installed Git client found"

if wc_rev == "???" and os.path.exists(outfile):
        print "WARNING: Can't determine Git working copy revision, %s not updated!" % outfile
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
    FreeOrion_WC_REVISION=wc_rev,
    FreeOrion_BUILDSYS=build_sys))
version_cpp.close()

print "Building rev: " + wc_rev
