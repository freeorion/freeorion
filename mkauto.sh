#!/bin/sh
rm -f FO-autotools.tar
#    ltmain.sh					
tar cvhf FO-autotools.tar			\
    aclocal.m4					\
    config.guess				\
    config.h.in					\
    config.sub					\
    configure					\
    depcomp					\
    install-sh					\
    m4						\
    missing					\
    mkinstalldirs                               \
    Makefile.am                                 \
    Makefile.in                                 \
    common_files.inc
tar f FO-autotools.tar --delete m4/CVS
rm -f FO-autotools.tar.bz2
bzip2 -9 FO-autotools.tar
if [ x$SF_USERNAME != x -a x$1 != xNO_UPLOAD ]; then
  echo Uploading new version to Sourceforge
  scp FO-autotools.tar.bz2 $SF_USERNAME@freeorion.sourceforge.net:/home/groups/f/fr/freeorion/htdocs
fi
