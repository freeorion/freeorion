#!/bin/sh

cd `dirname $0`

boostgcc=43
boost_lib_suffix=-gcc43-mt
#boost_lib_suffix=

if [ $boostgcc == "43" ]; then
    echo "Boost GCC 4.3"
    GLOBAL_PARAMS="with_boost_include=/usr/local/include/boost-1_42 with_boost_libdir=/usr/local/lib boost_lib_suffix=${boost_lib_suffix} build_ogre_driver=yes  build_ogre_ois_plugin=yes build_sdl_driver=no debug=yes CC=gcc-4.3 CXX=g++-4.3 CPPDEFINES=BOOST_ASIO_DISABLE_EVENTFD"
else
    GLOBAL_PARAMS="boost_lib_suffix=-gcc42-mt build_ogre_driver=no  build_ogre_ois_plugin=no debug=yes CC=gcc-4.2 CXX=g++-4.2 CPPDEFINES=BOOST_ASIO_DISABLE_EVENTFD"

fi


#build_sdl_driver=no

#Dynamically, not needed
#scons configure dynamic=yes $GLOBAL_PARAMS \
#    && scons 
#&& scons install

# build static libs
if [ -z $CONFIGURE ]; then
    CONFIGURE=0
fi

if [ "$CONFIGURE" == "1" ]; then
    scons --config=force configure $GLOBAL_PARAMS
else
    # Build both, for testing cases its better
    # First build the dynamic one, as this is used by tutorials
    ONLY_STATIC=0

    if [ "$ONLY_STATIC" == 1 ]; then
	scons -j8 dynamic=no $GLOBAL_PARAMS
    else
	nice scons -j8 dynamic=yes $GLOBAL_PARAMS  && scons -j8 dynamic=no $GLOBAL_PARAMS
    fi

fi

# && scons install
