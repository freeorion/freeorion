#!/bin/sh
cd `dirname $0`
. ./mdist.config.sh

# Change back into the FreeOrion root directory
cd ..

# Copy Installer
rsync --recursive --verbose \
    --exclude ".svn" --exclude "CVS" --exclude "*~" --exclude "#*#" \
    loki_setup/* \
    $TARGET_ROOT

strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/uninstall
strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/setup
strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/glibc-2.1/setup.gtk2
strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/glibc-2.1/setup.gtk

(
    cd $TARGET_ROOT

    # Copy wrapper / Run-Script
    mv freeorion-wrapper freeorion

    # Make wrapper it executable
    chmod +x freeorion
    
    # Create an autorun-script - for now its the same as wrapper
    cp -a freeorion autorun.sh

    # Link logo for loki setup
    ln -sf setup.data/logo.png .

    ln -sf README.txt README
)
