#!/bin/sh
cd `dirname $0`
. mdist.config.sh


# FreeorionD and FreeorionCA need to be named without -static
for F in freeorion freeoriond freeorionca; do
    #cp -va ${F}-static $TARGET_BIN/$F
    strip --strip-all -v -o $TARGET_BIN/$F ${F}-static
done


# Copy libs
cp -aLv /usr/local/lib/libOgreMain-1.6.2.so     $TARGET_LIB

cp -aLv /usr/lib/libfreeimage.so.3              $TARGET_LIB
#cp -aLv /usr/lib/libtiff.so                     $TARGET_LIB
cp -aLv /usr/lib/libzzip-0.so.13                $TARGET_LIB
cp -aLv /usr/lib/libOIS.so.1                    $TARGET_LIB
cp -aLv /usr/lib/libxcb.so.1                    $TARGET_LIB

cp -aLv GG/libGiGiOgrePlugin_OIS.so             $TARGET_LIB
cp -aLv GG/libGiGiOgre.so                       $TARGET_LIB
cp -aLv GG/libGiGi.so                           $TARGET_LIB

# nvidia Cg toolkit libs
cp -aLv /usr/lib/libCg.so                       $TARGET_LIB
cp -aLv /usr/lib/libCgGL.so                     $TARGET_LIB


# boost libs
for BOOST_LIB in \
    libboost_signals-gcc43-mt-1_37.so.1.37.0 \
    libboost_system-gcc43-mt-1_37.so.1.37.0 \
    libboost_filesystem-gcc43-mt-1_37.so.1.37.0 \
    libboost_thread-gcc43-mt-1_37.so.1.37.0
do
    cp -aLv /usr/local/lib/${BOOST_LIB}         $TARGET_LIB
done


# OGRE modules must reside where freeorion binary is located
for OGRE_PLUGIN in RenderSystem_GL.so Plugin_ParticleFX.so Plugin_OctreeSceneManager.so Plugin_CgProgramManager.so ; do
    cp -aL /usr/local/lib/OGRE/${OGRE_PLUGIN}  $TARGET_BIN
done

if [ ! -e /usr/local/lib/OGRE/Plugin_CgProgramManager.so ]; then
    sed -e '/Plugin_CgProgramManager/d' <ogre_plugins.cfg >$TARGET_BIN/ogre_plugins.cfg
else
    cp ogre_plugins.cfg             $TARGET_BIN
fi


# OISInfput.cfg is needed so that mouse focus is not grabbed
cp OISInput.cfg  $TARGET_BIN


chmod -R u+w   $TARGET_ROOT/
chmod -R o+rX  $TARGET_ROOT/


