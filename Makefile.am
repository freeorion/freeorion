SUBDIRS = server client 
EXTRA_DIST = m4
ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -DFREEORION_LINUX
noinst_DATA = freeoriond.exe freeorion.exe freeorionca.exe

freeoriond.exe : server/freeoriond.exe
	$(LN_S) server/freeoriond.exe freeoriond.exe

freeorion.exe : client/human/freeorion.exe
	$(LN_S) client/human/freeorion.exe freeorion.exe

freeorionca.exe : client/AI/freeorionca.exe
	$(LN_S) client/AI/freeorionca.exe freeorionca.exe
