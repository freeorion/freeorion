bin_PROGRAMS = freeorionca.exe

freeorionca_exe_SOURCES =			\
@top_srcdir@/client/ClientApp.cpp		\
@top_srcdir@/client/ClientApp.h			\
@top_srcdir@/client/AI/AIClientApp.cpp		\
@top_srcdir@/client/AI/AIClientApp.h		\
@top_srcdir@/client/AI/camain.cpp

AM_CPPFLAGS = @GIGINET_CFLAGS@ @GIGISDL_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_AI
AM_CXXFLAGS = -g3 -Wall

freeorionca_exe_LDADD = @SDL_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@ \
@top_srcdir@/Empire/libFOempire-common.a	\
@top_srcdir@/network/libFOnetwork-ai.a		\
@top_srcdir@/network/libFOnetwork-common.a	\
@top_srcdir@/universe/libFOuniverse-ai.a	\
@top_srcdir@/universe/libFOuniverse-common.a	\
@top_srcdir@/util/libFOutil-ai.a		\
@top_srcdir@/util/libFOutil-common.a

