bin_PROGRAMS = freeorionca.exe

freeorionca_exe_SOURCES =				\
@top_srcdir@/Empire/ClientEmpireManager.cpp	\
@top_srcdir@/Empire/ClientEmpireManager.h	\
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
@top_srcdir@/client/ClientApp.cpp		\
@top_srcdir@/client/ClientApp.h			\
@top_srcdir@/client/AI/AIClientApp.cpp		\
@top_srcdir@/client/AI/AIClientApp.h		\
@top_srcdir@/client/AI/camain.cpp		\
@top_srcdir@/network/ClientNetworkCore.cpp	\
@top_srcdir@/network/ClientNetworkCore.h	\
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
@top_srcdir@/util/SitRepEntry.h			\
@top_srcdir@/server/ServerApp.cpp

AM_CPPFLAGS = @FT2_CFLAGS@ @GIGI_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_AI
AM_CXXFLAGS = -g3 -Wall

freeorionca_exe_LDADD = @SDL_LIBS@ @GIGI_LIBS@ @FT2_LIBS@
