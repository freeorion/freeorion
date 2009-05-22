#!/bin/sh
export PKG_CONFIG_PATH=GG:../bullet-2.73
export CC=gcc-4.3
export CXX=g++-4.3
export CPPDEFINES=BOOST_ASIO_DISABLE_EVENTFD

# boost_lib_suffix=-gcc42-mt-1_35

# use --config=force to really force a new configuration
scons --config=force configure \
    with_boost_include=/usr/local/include/boost-1_37 with_boost_libdir=/usr/local/lib boost_lib_suffix=-gcc43-mt \
    with_python=python2.5 \
    debug=yes \
    with_gg=./GG with_gg_include=./GG with_gg_libdir=./GG \
    with_bullet=../bullet-2.73 \
    with_bullet_include=../bullet-2.73/src \
    with_bullet_libdir=../bullet-2.73/src \
    CC=${CC} CXX=${CXX} CPPDEFINES=${CPPDEFINES} \




#with_graphviz=../graphviz-2.16.1
#multithreaded=no 
#with_graphviz_include=../graphviz-2.16.1/lib with_graphviz_lib=../graphviz-2.16.1/
