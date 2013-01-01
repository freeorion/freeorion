import sys
import os
import subprocess as sp
from string import Template

os.chdir(sys.argv[1])

try:

  svn_proc = sp.Popen(["SubWCRev.exe", "."], stdout=sp.PIPE)
  svn_info = svn_proc.communicate()[0]

  for line in svn_info.splitlines():
    if line.startswith("Updated to revision"):
      wc_rev = line.rpartition(" ")[2]

except:

  try:

    svn_proc = sp.Popen(["svn", "info"], stdout=sp.PIPE)
    svn_info = svn_proc.communicate()[0]
    
    for line in svn_info.splitlines():
    	if line.startswith("Revision:"):
    		wc_rev = line.partition(" ")[2]

  except:

    wc_rev = ""
    print "WARNING: No properly installed SVN client found"

if wc_rev == "":
	print "WARNING: Can't determine SVN working copy revision, Version.cpp not updated!"
	quit()

try:
  template_file = open("Version.template")
  template = Template(template_file.read())
  template_file.close()
except:
  print "WARNING: Can't access template file for Version.cpp, Version.cpp not updated!"
  quit()

version_cpp = open("Version.cpp", "w")
version_cpp.write(template.substitute(WCREV=wc_rev))
version_cpp.close()

print "Building rev: " + wc_rev