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
else
    FILENAME=${FO_PREFIX}_i386_static.tar$GZIP_EXT
fi
OUT=${OUTDIR}/${FILENAME}

# Copy Data
(
    cd $TARGET_ROOT/..
    strip --strip-all $TARGET_BIN/freeorion*
    
    
    echo "SVN-Revision: ${SVNREV}. Output: ${OUT}"

    export GZIP="--best --verbose"

    tar --exclude=".svn*" --exclude "*~" \
        --create $TAR_GZIP_PARAM \
        --file $OUT \
        freeorion
)


(
    if [ ! -e $SVNREV ]; then
        cd "`dirname $OUT`"
        ln -sf $FILENAME ${FO_PREFIX}_i386_static.tar$GZIP_EXT

        # Nightly builds
	NIGHTLY=/home/wwwroot/freeorion.psitronic.de/download/nightly
    
	if [ -e $NIGHTLY ]; then
	    cp $FILENAME $NIGHTLY
	fi
    fi

)

