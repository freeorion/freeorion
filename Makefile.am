common_files.inc : Makefile.am
	./create_commonfilesinc.pl
include common_files.inc

EXTRA_DIST = m4 remake_libs.mak default Vera.ttf
ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -DFREEORION_LINUX
noinst_DATA = freeoriond.exe freeorion.exe freeorionca.exe save
CLEANFILES = freeoriond.exe freeorion.exe freeorionca.exe
install:
	@echo For now, you have to run ./freeorion.exe from the source-directory.
save :
	mkdir -p save

AM_CXXFLAGS = -Wall

bin_PROGRAMS = freeoriond.exe freeorion.exe freeorionca.exe

freeorion_exe_CPPFLAGS=@FT2_CFLAGS@ @GIGI_CFLAGS@ @GIGISDL_CFLAGS@ @GIGINET_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_HUMAN -DFREEORION_LINUX
freeorion_exe_LDADD = @SDL_LIBS@ @GIGI_LIBS@ @GIGINET_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@ @FT2_LIBS@

freeoriond_exe_CPPFLAGS = @GIGI_CFLAGS@ @GIGINET_CFLAGS@ @FT2_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_SERVER -DFREEORION_LINUX #-DWITH_PYTHON -I/usr/include/python2.2 
freeoriond_exe_LDADD = @SDL_LIBS@ @GIGI_LIBS@ @GIGINET_LIBS@ @FT2_LIBS@	#-lboost_python  -lpython2.2 

freeorionca_exe_CPPFLAGS = @GIGINET_CFLAGS@ @GIGISDL_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_AI -DFREEORION_LINUX
freeorionca_exe_LDADD = @SDL_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@

# The list of prefixes that will be prepended to the names of the object files
# THESE HAVE TO BE IDENTICAL TO THOSE DEFINED IN create_commonfilesinc.pl!
freeoriond_exe_SHORTNAME = server
freeorion_exe_SHORTNAME = client
freeorionca_exe_SHORTNAME = ai

COMMON_FILES=					\
Empire/ClientEmpireManager.cpp			\
Empire/Empire.cpp				\
Empire/EmpireManager.cpp			\
Empire/TechLevel.cpp				\
Empire/TechManager.cpp				\
combat/CombatSystem.cpp				\
network/Message.cpp				\
network/NetworkCore.cpp				\
network/XDiff.cpp				\
network/XHash.cpp				\
network/XLut.cpp				\
network/XParser.cpp				\
network/XTree.cpp				\
universe/Fleet.cpp				\
universe/Planet.cpp				\
universe/PopCenter.cpp				\
universe/Predicates.cpp				\
universe/ProdCenter.cpp				\
universe/Ship.cpp				\
universe/System.cpp				\
universe/Universe.cpp				\
universe/UniverseObject.cpp			\
util/AppInterface.cpp				\
util/DataTable.cpp				\
util/GZStream.cpp				\
util/OptionsDB.cpp				\
util/Order.cpp					\
util/OrderSet.cpp				\
util/Process.cpp				\
util/SitRepEntry.cpp				\
util/VarText.cpp

freeoriond_exe_SOURCES =			\
server/ServerApp.cpp				\
server/ServerApp.h				\
server/dmain.cpp				\
Empire/ServerEmpireManager.cpp			\
network/ServerNetworkCore.cpp			\
$(COMMON_FILES)

freeorion_exe_SOURCES =				\
client/ClientApp.cpp				\
client/ClientApp.h				\
client/human/HumanClientApp.cpp			\
client/human/HumanClientApp.h			\
client/human/chmain.cpp				\
UI/About.cpp					\
UI/CUIControls.cpp				\
UI/CUIDrawUtil.cpp				\
UI/CUI_Wnd.cpp					\
UI/ClientUI.cpp					\
UI/EmpireSelect.cpp				\
UI/FleetButton.cpp				\
UI/FleetWindow.cpp				\
UI/GalaxySetupWnd.cpp				\
UI/InGameOptions.cpp				\
UI/IntroScreen.cpp				\
UI/LinkText.cpp					\
UI/MapWnd.cpp					\
UI/MultiplayerLobbyWnd.cpp			\
UI/Parser.cpp					\
UI/ServerConnectWnd.cpp				\
UI/ServerUI.cpp					\
UI/SidePanel.cpp				\
UI/SitRepPanel.cpp				\
UI/StringTable.cpp				\
UI/SystemIcon.cpp				\
UI/ToolContainer.cpp				\
UI/ToolWnd.cpp					\
UI/TurnProgressWnd.cpp				\
network/ClientNetworkCore.cpp			\
$(COMMON_FILES)

freeorionca_exe_SOURCES =			\
client/ClientApp.cpp				\
client/ClientApp.h				\
client/AI/AIClientApp.cpp			\
client/AI/AIClientApp.h				\
client/AI/camain.cpp				\
network/ClientNetworkCore.cpp			\
$(COMMON_FILES)
