bin_PROGRAMS = freeoriond.exe

# Normally you should use $(top_srcdir) instead of @top_srcdir, but there seems to be a bug in automake
# that let it create a directory named "$(top_srcdir)" instead...

# Add every source file the server needs, also add the header file, if possible, since this will
# guarantee that the header-files are also distributed when doing a 'make dist'

freeoriond_exe_SOURCES =			\
@top_srcdir@/Empire/Empire.cpp			\
@top_srcdir@/Empire/Empire.h			\
@top_srcdir@/Empire/EmpireManager.cpp		\
@top_srcdir@/Empire/EmpireManager.h		\
@top_srcdir@/Empire/ServerEmpireManager.cpp	\
@top_srcdir@/Empire/ServerEmpireManager.h	\
@top_srcdir@/Empire/TechLevel.cpp		\
@top_srcdir@/Empire/TechLevel.h			\
@top_srcdir@/Empire/TechManager.cpp		\
@top_srcdir@/Empire/TechManager.h		\
@top_srcdir@/network/Message.cpp		\
@top_srcdir@/network/Message.h			\
@top_srcdir@/network/NetworkCore.cpp		\
@top_srcdir@/network/NetworkCore.h		\
@top_srcdir@/network/ServerNetworkCore.cpp	\
@top_srcdir@/network/ServerNetworkCore.h	\
@top_srcdir@/network/XDiff.cpp			\
@top_srcdir@/network/XDiff.hpp			\
@top_srcdir@/network/XHash.cpp			\
@top_srcdir@/network/XHash.hpp			\
@top_srcdir@/network/XLut.cpp			\
@top_srcdir@/network/XLut.hpp			\
@top_srcdir@/network/XParser.cpp		\
@top_srcdir@/network/XParser.hpp		\
@top_srcdir@/network/XTree.cpp			\
@top_srcdir@/network/XTree.hpp			\
@top_srcdir@/server/ServerApp.cpp		\
@top_srcdir@/server/ServerApp.h			\
@top_srcdir@/server/dmain.cpp			\
@top_srcdir@/universe/ClientUniverse.cpp	\
@top_srcdir@/universe/ClientUniverse.h		\
@top_srcdir@/universe/Fleet.cpp			\
@top_srcdir@/universe/Fleet.h			\
@top_srcdir@/universe/Planet.cpp		\
@top_srcdir@/universe/Planet.h			\
@top_srcdir@/universe/PopCenter.cpp		\
@top_srcdir@/universe/PopCenter.h		\
@top_srcdir@/universe/ProdCenter.cpp		\
@top_srcdir@/universe/ProdCenter.h		\
@top_srcdir@/universe/ServerUniverse.cpp	\
@top_srcdir@/universe/ServerUniverse.h		\
@top_srcdir@/universe/Ship.cpp			\
@top_srcdir@/universe/Ship.h			\
@top_srcdir@/universe/System.cpp		\
@top_srcdir@/universe/System.h			\
@top_srcdir@/universe/UniverseObject.cpp	\
@top_srcdir@/universe/UniverseObject.h		\
@top_srcdir@/util/Order.cpp			\
@top_srcdir@/util/Order.h			\
@top_srcdir@/util/OrderSet.cpp			\
@top_srcdir@/util/OrderSet.h			\
@top_srcdir@/util/Process.cpp			\
@top_srcdir@/util/Process.h			\
@top_srcdir@/util/SitRepEntry.h

AM_CPPFLAGS = @GIGI_CFLAGS@ @GIGISDL_CFLAGS@ @GIGINET_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_SERVER 
AM_CXXFLAGS = -g3 -Wall

freeoriond_exe_LDADD = @SDL_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@
