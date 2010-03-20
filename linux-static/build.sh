#!/bin/sh
export PKG_CONFIG_PATH=GG:../bullet-2.73/
export SCRIPT_PATH=./linux-static

# Update, build, static link, copy binaries and tar.gz the files

# rm -rf /tmp/freeorion
svn update && GG/build.sh && nice scons -j8 ; nice ${SCRIPT_PATH}/link.sh \
    && ${SCRIPT_PATH}/copy_binaries.sh && ${SCRIPT_PATH}/copy_setup.sh && ${SCRIPT_PATH}/copy_data.sh && ${SCRIPT_PATH}/copy_python.sh \
    && ${SCRIPT_PATH}/mdist.sh

