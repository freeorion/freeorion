bin_PROGRAMS = freeoriond.exe

# Normally you should use $(top_srcdir) instead of @top_srcdir, but there seems to be a bug in automake
# that let it create a directory named "$(top_srcdir)" instead...

freeoriond_exe_SOURCES =			\
@top_srcdir@/server/ServerApp.cpp		\
@top_srcdir@/server/ServerApp.h			\
@top_srcdir@/server/dmain.cpp
# @top_srcdir@/Python/my_universe.cpp

AM_CPPFLAGS = @GIGI_CFLAGS@ @GIGINET_CFLAGS@ @FT2_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_SERVER #-DWITH_PYTHON -I/usr/include/python2.2
AM_CXXFLAGS = -g3 -Wall

# Remake convenience libs if needed

%-common.a: force
	make -C $(@D)

force : ;

freeoriond_exe_LDADD = @SDL_LIBS@ @GIGI_LIBS@ @GIGINET_LIBS@ @FT2_LIBS@	\
@top_srcdir@/network/libFOnetwork-server.a				\
@top_srcdir@/network/libFOnetwork-common.a				\
@top_srcdir@/Empire/libFOempire-common.a				\
@top_srcdir@/universe/libFOuniverse-server.a				\
@top_srcdir@/universe/libFOuniverse-common.a				\
@top_srcdir@/util/libFOutil-server.a					\
@top_srcdir@/util/libFOutil-common.a					\
@top_srcdir@/combat/libFOcombat-server.a				\
@top_srcdir@/combat/libFOcombat-common.a
#-lboost_python  -lpython2.2

