#!/bin/sh
cd `dirname "$0"`
. ./mdist.config.sh

# Change back into the FreeOrion root directory
cd ..

# Create a tar.gz file

if [ -z $USE_GZIP ]; then
    USE_GZIP=1
fi

if [ "$USE_GZIP" = 1 ]; then
    GZIP_EXT=.gz
    TAR_GZIP_PARAM="--gzip"
else
    GZIP_EXT=
    TAR_GZIP_PARAM=
fi



SVNREV=`svn info | grep "^Revision.*" | sed -e 's/Revision: *\([0-9]*\)/\1/' `

if [ -e $SVNREV ]; then
    echo "Konnte SVN-Version nicht bestimmen"
fi

if grep ubuntu904i386 /etc/debian_chroot ; then
    FO_PREFIX=freeorion_ubuntu
else
    FO_PREFIX=freeorion
fi


OUTDIR=/tmp/


if [ ! -e $SVNREV ]; then
    FILENAME=${FO_PREFIX}_rev${SVNREV}_i386_static.tar$GZIP_EXT
    FILENAME_DBG=${FO_PREFIX}_rev${SVNREV}_debugsymbols.tar$GZIP_EXT
else
    FILENAME=${FO_PREFIX}_i386_static.tar$GZIP_EXT
    FILENAME_DBG=${FO_PREFIX}_debugsymbols.tar$GZIP_EXT
fi
OUT=${OUTDIR}/${FILENAME}
OUT_DBG=${OUTDIR}/${FILENAME_DBG}

# Copy Data
(
    cd $TARGET_ROOT/..
    # Strip the binary and put the debug strings into separate file (-o)
    # /tmp/freeorion_rev${SVNREV}.symbols
    # strip --keep-file-symbols --strip-all $TARGET_BIN/freeorion*
    # Stripping is done in copying
    
    echo "SVN-Revision: ${SVNREV}."
    echo "Output: ${OUT}"

    export GZIP="--best --verbose"

    tar --exclude=".svn*" --exclude "*~" \
        --create $TAR_GZIP_PARAM \
        --file $OUT \
        freeorion


    # Pack debugging symbols
    echo "Output: ${OUT_DBG}"
    tar --create $TAR_GZIP_PARAM \
	--file $OUT_DBG \
	-C freeorion-debug \
	application


)


(
    if [ ! -e $SVNREV ]; then
        cd "`dirname $OUT`"
        ln -sf $FILENAME ${FO_PREFIX}_i386_static.tar$GZIP_EXT

        # Nightly builds
	NIGHTLY=/home/wwwroot/freeorion.psitronic.de/download/nightly
    
	if [ -e $NIGHTLY ]; then
	    cp -v $FILENAME $FILENAME_DBG  $NIGHTLY
	fi
    fi

)

