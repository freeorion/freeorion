#!/bin/sh
# This script tries to download the configure files for people
# who don't have autotools installed. It requires wget for
# automatic downloading
# see http://www.gnu.org/software/wget for more information about wget
# $Id$

FILENAME=FO-autotools.tar.bz2
URL=http://freeorion.sourceforge.net/$FILENAME
TAR_OPTIONS=-xjf

while test $# != 0; do
  case $1 in 
  -[nN])
      NO_DOWNLOAD=yes
      ;;
  -[fF])
      FORCE_REBUILD=yes
      ;;
  -[hH]|--help)
      echo "Usage: $0 [-n] [-f]"
      echo "-n        Do not download $FILENAME automatically, it must"
      echo "          be downloaded manually from"
      echo "          $URL"
      echo
      echo "-f        Force operation regardless whether $FILENAME was changed."
      echo "          Useful if unpacking lead to an error"
      echo 
      echo "-h --help This message"
      exit
      ;;
  *)
      echo "$0: Unknown option $1"
      exit 1
  esac
  shift
done
rm -f update-configure.stamp
touch -r$FILENAME update-configure.stamp 2>/dev/null
if test x$NO_DOWNLOAD != xyes; then
  echo "*** Downloading $URL"
  if wget -N $URL; then :; else
    echo >&2 "*** An error occured while trying to download $FILENAME from"
    echo >&2 "*** $URL."
    echo >&2 "(I'll need wget to download the file automatically)"
    echo >&2 "You can try to download the file yourself from"
    echo >&2 "$URL"
    echo >&2 "then place it into thus directory and execute this script with"
    echo >&2 "the '-n' option to skip automatic downloading."
    exit 1
  fi
fi
if test ! -e $FILENAME; then
  echo >&2 "*** $FILENAME is missing! Exiting"
  exit 2
fi 

if test x$FORCE_REBUILD = xyes -o $FILENAME -nt update-configure.stamp; then
  echo "*** Extracting $FILENAME"   
  tar $TAR_OPTIONS $FILENAME
  if test $? -ne 0; then
    echo >&2 "*** tar exited with non-zero status. Aborting!"
    exit $?
  fi
  if test -e config.status; then
     echo "*** re-runnning configure"
     ./config.status --recheck
     ./config.status
     echo "*** done"
  else
     echo "*** now run ./configure, see ./configure --help for more information"
  fi
else
  echo "*** $FILENAME is up-to-date, nothing to be done"
fi

rm -f update-configure.stamp