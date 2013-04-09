#!/bin/bash
export PKG_CONFIG_PATH=GG:../bullet-2.73/
# Update, build, static link, copy binaries and tar.gz the files

svn update

pushd GG
  CXXFLAGS=BOOST_ASIO_DISABLE_EVENTFD cmake -DBUILD_SDL_DRIVER=OFF
  make -j6
popd

cmake -DGIGIDIR=GG .
make -j6

. ./linux-static/mdist.config.sh

if [ ! -d $TARGET_BIN ]; then
    mkdir -pv $TARGET_BIN
fi

if [ ! -d ${TARGET_BIN}/lib ]; then
    mkdir -pv ${TARGET_BIN}/lib
fi

if [ ! -d $TARGET_DBG ]; then
    mkdir -pv $TARGET_DBG
fi

# FreeorionD and FreeorionCA need to be named without -static
for EXE in freeorion freeoriond freeorionca; do
    # Strip the exes
    # see "man objcopy" in section  --only-keep-debug
    objcopy --only-keep-debug -v ${EXE} ${TARGET_DBG}/${EXE}.dbg 
    objcopy --strip-all -v ${EXE} ${TARGET_BIN}/${EXE}
    objcopy --add-gnu-debuglink=${TARGET_DBG}/${EXE}.dbg ${TARGET_BIN}/${EXE}
done

# OISInput.cfg is needed so that mouse focus is not grabbed
cp OISInput.cfg  $TARGET_BIN

chmod -R u+w   $TARGET_ROOT/
chmod -R o+rX  $TARGET_ROOT/

rsync --recursive --verbose --exclude ".git" --exclude ".svn" --exclude "*~" --exclude "#*#" loki_setup/* $TARGET_ROOT

strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/uninstall
strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/setup
strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/glibc-2.1/setup.gtk2
strip --strip-all -v $TARGET_ROOT/setup.data/bin/Linux/x86/glibc-2.1/setup.gtk

# Copy wrapper / Run-Script
mv ${TARGET_ROOT}/freeorion-wrapper ${TARGET_ROOT}/freeorion
chmod +x ${TARGET_ROOT}/freeorion

# Create an autorun-script - for now its the same as wrapper
cp -a ${TARGET_ROOT}/freeorion ${TARGET_ROOT}/autorun.sh

pushd ${TARGET_ROOT}
ln -sf setup.data/logo.png .
ln -sf README.txt README
popd

# Sync Data and Fonts
# --delete will result in cleaning everything unwanted at target
rsync --recursive --verbose --delete --exclude ".git" --exclude ".svn" --exclude "*~" --exclude "*.pyc" default $TARGET_BIN

# Copy Info-Files to root
cp changelog.txt $TARGET_ROOT

# Link Copying to Root
pushd $TARGET_ROOT
ln -sf application/default/COPYING COPYING
popd

PY_ROOT=$TARGET_ROOT/application/python2.7
mkdir -pv ${PY_ROOT}/lib-dynload/

cp -a /usr/lib/libpython2.7*  ${PY_ROOT}
cp -a /lib/libutil.so.1 /lib/libutil-2.7.so ${PY_ROOT}

for M in pickle site os warnings posixpath stat UserDict copy_reg types struct linecache codecs re sre_compile sre_constants sre_parse binascii StringIO copy; do
    cp -a /usr/lib64/python2.7/$M.py* ${PY_ROOT}
        
    for D in /usr/lib64/python2.7/lib-dynload/_$M.so /usr/lib64/python2.7/lib-dynload/$M.so; do
        if [ -e $D ]; then
            cp -a $D ${PY_ROOT}
        fi
    done
done

cp -a /usr/lib64/python2.7/encodings/* ${PY_ROOT}

(
SVNREV=`svn info | grep "^Revision.*" | sed -e 's/Revision: *\([0-9]*\)/\1/' `

if [ -e $SVNREV ]; then
    echo "Konnte SVN-Version nicht bestimmen"
    SVNREV=""
else
    echo "SVN-Revision: ${SVNREV}."
    SVNREV="_rev${SVNREV}"
fi

OUTDIR=/tmp/

FILENAME=freeorion${SVNREV}_i386_static.tar.gz
FILENAME_DBG=freeorion${SVNREV}_debugsymbols.tar.gz
OUT=${OUTDIR}/${FILENAME}
OUT_DBG=${OUTDIR}/${FILENAME_DBG}

# Copy Data
(
pushd $TARGET_ROOT/..
 
    echo "Output: ${OUT}"

    GZIP=--best tar --exclude=".svn*" --exclude "*~" \
        --create --gzip \
        --file $OUT \
        freeorion


    # Pack debugging symbols
    echo "Output: ${OUT_DBG}"
    GZIP=--best tar --create --gzip \
	--file $OUT_DBG \
	-C freeorion-debug \
	application

popd
)

