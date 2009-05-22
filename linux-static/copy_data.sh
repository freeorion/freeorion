#!/bin/sh
cd `dirname $0`
. mdist.config.sh


# Sync Data and Fonts
# --delete will result in cleaning everything unwanted at target
rsync --recursive --verbose --delete \
    --exclude ".svn" \
    --exclude "*~" \
    --exclude "*.pyc" \
    default License.DejaVu License.Vera  \
    $TARGET_BIN


# Copy Info-Files to root
cp RELEASE-NOTES-V03.txt $TARGET_ROOT


# Link Copying to Root
(
cd $TARGET_ROOT
ln -sf application/default/COPYING COPYING

)