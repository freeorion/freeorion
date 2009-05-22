#!/bin/sh
export PKG_CONFIG_PATH=GG:../bullet-2.73/

svn update && GG/build.sh && nice scons -j2 && nice ./link.sh && ./copy_binaries.sh && ./copy_setup.sh && ./copy_data.sh && ./copy_python.sh && ./mdist.sh

