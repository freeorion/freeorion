#!/usr/bin/python
import sys
import os
import platform
from string import Template

if 2 != len(sys.argv):
    print "ERROR: invalid parameters."
    print "make_ogre_plugins.py <project rootdir>"
    quit()

os.chdir(sys.argv[1])
infile = 'ogre_plugins.cfg.in'
outfile = 'ogre_plugins.cfg'

try:
    template_file = open(infile)
    template = Template(template_file.read())
    template_file.close()
except:
    print "WARNING: Can't access %s, %s not updated!" % (infile, outfile)
    quit()

print "Writing file: %s" % outfile

content = template.substitute(OGRE_PLUGIN_DIR_REL=".")
if platform.system() == 'Darwin': # On mac ogre doesn't need a plugin folder set.
    result = []
    for line in content.split('\n'):
        if line.startswith("PluginFolder"):
            result.append("# Plugins are automatically found within the application bundle's")
            result.append("# frameworks so the PluginFolder statement is not necessary (setting it")
            result.append("# would actually cause OGRE to look in the wrong place).")
            result.append("# " + line)
        else:
            result.append(line)
    content = "\n".join(result)

with open(outfile, "w") as ogre_plugins_cfg:
    ogre_plugins_cfg.write(content)

