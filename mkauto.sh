#!/bin/sh
rm -f FO-autotools.tar
tar cvhf FO-autotools.tar			\
    aclocal.m4					\
    config.guess				\
    config.h.in					\
    config.sub					\
    configure					\
    configure.ac				\
    depcomp					\
    install-sh					\
    ltmain.sh					\
    m4						\
    missing					\
    mkinstalldirs                               \
    `find . -name "Makefile.am" -o -name "Makefile.in"`
tar f FO-autotools.tar --delete m4/CVS
rm -f FO-autotools.tar.bz2
bzip2 -9 FO-autotools.tar
if [ x$SF_USERNAME != x -a x$1 = xU ]; then
  echo Uploading new version to Sourceforge
  scp FO-autotools.tar.bz2 $SF_USERNAME@freeorion.sourceforge.net:/home/groups/f/fr/freeorion/htdocs
fi