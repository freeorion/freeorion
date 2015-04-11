#!/usr/bin/python
import sys
import os
from datetime import datetime
from subprocess import check_output, check_call

if len(sys.argv) != 3:
    print "ERROR: invalid parameters."
    print "make_dmg.py <project rootdir> <built products dir>"
    quit()

os.chdir(sys.argv[1])
build_no = "XXXX"
try:
    commit = check_output(["git", "show", "-s", "--format=%h", "HEAD"]).strip()
    timestamp = float(check_output(["git", "show", "-s", "--format=%ct", "HEAD"]).strip())
    build_no = ".".join([datetime.utcfromtimestamp(timestamp).strftime("%Y-%m-%d"), commit])
except:
    print "WARNING: git not installed, can't determine build number"

built_product_dir = sys.argv[2]
app = os.path.join(built_product_dir, "FreeOrion.app")
dmg_file = "FreeOrion_%s_MacOSX_10.7.dmg" % build_no
out_path = os.path.join(built_product_dir, dmg_file)
if os.path.exists(out_path):
    os.remove(out_path)

print "Creating %s in %s" % (dmg_file, built_product_dir)

check_call('hdiutil create -volname FreeOrion -megabytes 1000 -srcfolder "%s" -ov -format UDZO "%s"' % (app, out_path), shell=True)
