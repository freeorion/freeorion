#!/bin/sh
cd `dirname $0`
. ./mdist.config.sh


cd ..

# FreeorionD and FreeorionCA need to be named without -static
for F in freeorion freeoriond freeorionca; do
    cp -va ${F} $TARGET_BIN/$F
    strip --strip-all -v -o $TARGET_BIN/$F ${F}
done

cp -aLv GG/libGiGiOgrePlugin_OIS.so             $TARGET_LIB
cp -aLv GG/libGiGiOgre.so                       $TARGET_LIB
cp -aLv GG/libGiGi.so                           $TARGET_LIB


# nvidia Cg toolkit libs
cp -aLv /usr/lib/libCg.so                       $TARGET_LIB
cp -aLv /usr/lib/libCgGL.so                     $TARGET_LIB

# OGRE modules must reside where freeorion binary is located
for OGRE_PLUGIN in RenderSystem_GL.so Plugin_ParticleFX.so Plugin_OctreeSceneManager.so Plugin_CgProgramManager.so ; do
    cp -aL /usr/local/lib/OGRE/${OGRE_PLUGIN}  $TARGET_BIN
done

# Create or copy ogre_plugins.cfg 
if [ ! -e /usr/local/lib/OGRE/Plugin_CgProgramManager.so ]; then
    sed -e '/Plugin_CgProgramManager/d' <ogre_plugins.cfg >$TARGET_BIN/ogre_plugins.cfg
else
    cp ogre_plugins.cfg             $TARGET_BIN
fi

# OISInfput.cfg is needed so that mouse focus is not grabbed
cp OISInput.cfg  $TARGET_BIN


chmod -R u+w   $TARGET_ROOT/
chmod -R o+rX  $TARGET_ROOT/
