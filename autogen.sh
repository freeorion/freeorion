#!/bin/sh
echo "** Creating configure and friends"
set -x
aclocal -I m4
autoconf
libtoolize
autoheader
automake -a --foreign
set +x
if test ! -e configure -o ! -e ltmain.sh -o ! -e config.h.in -o ! -e Makefile.in; then
   echo "** Unable to generate all files!"
   echo "** you'll need autoconf 2.5, automake 1.7, libtool 1.5 and aclocal installed"
   echo "** You can also use the update-configure.sh script which will"
   echo "** download the required files, just remember to call it"
   echo "** from time to time, it will only download if anything has changed"
   exit 1
fi
echo "Now run ./configure, see ./configure --help for more information"

   