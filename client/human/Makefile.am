bin_PROGRAMS = freeorion.exe

freeorion_exe_SOURCES =				\
@top_srcdir@/client/ClientApp.cpp		\
@top_srcdir@/client/ClientApp.h			\
@top_srcdir@/client/human/HumanClientApp.cpp	\
@top_srcdir@/client/human/HumanClientApp.h	\
@top_srcdir@/client/human/chmain.cpp		\
@top_srcdir@/universe/PopCenter.cpp

AM_CPPFLAGS = @FT2_CFLAGS@ @GIGI_CFLAGS@ @GIGISDL_CFLAGS@ @GIGINET_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_HUMAN 
AM_CXXFLAGS = -g3 -Wall

include @top_srcdir@/remake_libs.mak

# In the following, order DOES matter!

freeorion_exe_LDADD =						\
@SDL_LIBS@ @GIGI_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@ @FT2_LIBS@	\
@top_srcdir@/network/libFOnetwork-human.a			\
@top_srcdir@/network/libFOnetwork-common.a			\
@top_srcdir@/Empire/libFOempire-common.a			\
@top_srcdir@/universe/libFOuniverse-human.a			\
@top_srcdir@/universe/libFOuniverse-common.a			\
@top_srcdir@/util/libFOutil-human.a				\
@top_srcdir@/util/libFOutil-common.a				\
@top_srcdir@/UI/libFOui-common.a				\
@top_srcdir@/combat/libFOcombat-human.a				\
@top_srcdir@/combat/libFOcombat-common.a

