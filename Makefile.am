common_files.inc : Makefile.am
	./create_commonfilesinc.pl
include common_files.inc

EXTRA_DIST = m4 remake_libs.mak default Vera.ttf create_commonfilesinc.pl
ACLOCAL_AMFLAGS = -I m4
noinst_DATA = freeoriond.exe freeorion.exe freeorionca.exe save
CLEANFILES = freeoriond.exe freeorion.exe freeorionca.exe
install:
	@echo For now, you have to run ./freeorion.exe from the source-directory.
save :
	mkdir -p save

AM_CXXFLAGS = -Wall

bin_PROGRAMS = freeoriond.exe freeorion.exe freeorionca.exe 

freeorion_exe_CPPFLAGS=@FT2_CFLAGS@ @GIGI_CFLAGS@ @GIGISDL_CFLAGS@ @GIGINET_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_HUMAN
freeorion_exe_LDADD = @SDL_LIBS@ @GIGI_LIBS@ @GIGINET_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@ @FT2_LIBS@

freeoriond_exe_CPPFLAGS = @GIGI_CFLAGS@ @GIGINET_CFLAGS@ @FT2_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_SERVER #-DWITH_PYTHON -I/usr/include/python2.2 
freeoriond_exe_LDADD = @SDL_LIBS@ @GIGI_LIBS@ @GIGINET_LIBS@ @FT2_LIBS@	#-lboost_python  -lpython2.2 

freeorionca_exe_CPPFLAGS = @GIGINET_CFLAGS@ @GIGISDL_CFLAGS@ @SDL_CFLAGS@ -DFREEORION_BUILD_AI
freeorionca_exe_LDADD = @SDL_LIBS@ @GIGISDL_LIBS@ @GIGINET_LIBS@

# The list of prefixes that will be prepended to the names of the object files
# THESE HAVE TO BE IDENTICAL TO THOSE DEFINED IN create_commonfilesinc.pl!
freeoriond_exe_SHORTNAME = server
freeorion_exe_SHORTNAME = client
freeorionca_exe_SHORTNAME = ai

# Add files that are needed by server and client here
# (conditional compiles via FREEORION_SERVER/CLIENT will be handled
# automatically)
COMMON_FILES=					\
Empire/ClientEmpireManager.cpp			\
Empire/ClientEmpireManager.h			\
Empire/Empire.cpp				\
Empire/Empire.h					\
Empire/EmpireManager.cpp			\
Empire/EmpireManager.h  			\
Empire/TechLevel.cpp				\
Empire/TechLevel.h  				\
Empire/TechManager.cpp				\
Empire/TechManager.h  				\
combat/CombatSystem.cpp				\
combat/CombatSystem.h  				\
network/Message.cpp				\
network/Message.h  				\
network/NetworkCore.cpp				\
network/NetworkCore.h  				\
network/XDiff.cpp				\
network/XDiff.hpp				\
network/XHash.cpp				\
network/XHash.hpp				\
network/XLut.cpp				\
network/XLut.hpp  				\
network/XParser.cpp				\
network/XParser.hpp  				\
network/XTree.cpp				\
network/XTree.hpp  				\
universe/Fleet.cpp				\
universe/Fleet.h	 			\
universe/Planet.cpp				\
universe/Planet.h  				\
universe/PopCenter.cpp				\
universe/PopCenter.h  				\
universe/Predicates.cpp				\
universe/Predicates.h  				\
universe/ProdCenter.cpp				\
universe/ProdCenter.h  				\
universe/Ship.cpp				\
universe/Ship.h  				\
universe/System.cpp				\
universe/System.h  				\
universe/Universe.cpp				\
universe/Universe.h  				\
universe/UniverseObject.cpp			\
universe/UniverseObject.h  			\
util/AppInterface.cpp				\
util/AppInterface.h  				\
util/DataTable.cpp				\
util/DataTable.h   				\
util/GLStateComparator.h			\
util/GZStream.cpp				\
util/GZStream.h  				\
util/OptionValidators.h				\
util/OptionsDB.cpp				\
util/OptionsDB.h  				\
util/Order.cpp					\
util/Order.h  					\
util/OrderSet.cpp				\
util/OrderSet.h  				\
util/Process.cpp				\
util/Process.h  				\
util/Random.h					\
util/SitRepEntry.cpp				\
util/SitRepEntry.h  				\
util/VarText.cpp				\
util/VarText.h  

# Files specific to SERVER
#Python/universe.cpp
freeoriond_exe_SOURCES =			\
server/ServerApp.cpp				\
server/ServerApp.h				\
server/dmain.cpp				\
Empire/ServerEmpireManager.cpp			\
Empire/ServerEmpireManager.h			\
network/ServerNetworkCore.cpp			\
network/ServerNetworkCore.h			\
$(COMMON_FILES)

# Files specific to CLIENT
freeorion_exe_SOURCES =				\
client/ClientApp.cpp				\
client/ClientApp.h				\
client/human/HumanClientApp.cpp			\
client/human/HumanClientApp.h			\
client/human/chmain.cpp				\
UI/About.cpp					\
UI/About.h					\
UI/CUIControls.cpp				\
UI/CUIControls.h				\
UI/CUIDrawUtil.cpp				\
UI/CUIDrawUtil.h 				\
UI/CUISpin.h					\
UI/CUI_Wnd.cpp					\
UI/CUI_Wnd.h					\
UI/ClientUI.cpp					\
UI/ClientUI.h					\
UI/EmpireSelect.cpp				\
UI/EmpireSelect.h				\
UI/FleetButton.cpp				\
UI/FleetButton.h 				\
UI/FleetWindow.cpp				\
UI/FleetWindow.h				\
UI/GalaxySetupWnd.cpp				\
UI/GalaxySetupWnd.h				\
UI/InGameOptions.cpp				\
UI/InGameOptions.h				\
UI/IntroScreen.cpp				\
UI/IntroScreen.h				\
UI/LinkText.cpp					\
UI/LinkText.h					\
UI/MapWnd.cpp					\
UI/MapWnd.h					\
UI/MultiplayerLobbyWnd.cpp			\
UI/MultiplayerLobbyWnd.h			\
UI/Parser.cpp  					\
UI/Parser.h  					\
UI/ServerConnectWnd.cpp				\
UI/ServerConnectWnd.h				\
UI/ServerUI.cpp					\
UI/ServerUI.h					\
UI/SidePanel.cpp				\
UI/SidePanel.h					\
UI/SitRepPanel.cpp				\
UI/SitRepPanel.h				\
UI/StringTable.cpp				\
UI/StringTable.h				\
UI/SystemIcon.cpp				\
UI/SystemIcon.h					\
UI/ToolContainer.cpp				\
UI/ToolContainer.h				\
UI/ToolWnd.cpp					\
UI/ToolWnd.h					\
UI/TurnProgressWnd.cpp				\
UI/TurnProgressWnd.h				\
network/ClientNetworkCore.cpp			\
network/ClientNetworkCore.h			\
$(COMMON_FILES)

# Files specific for AI
freeorionca_exe_SOURCES =			\
client/ClientApp.cpp				\
client/ClientApp.h				\
client/AI/AIClientApp.cpp			\
client/AI/AIClientApp.h				\
client/AI/camain.cpp				\
network/ClientNetworkCore.cpp			\
network/ClientNetworkCore.h			\
$(COMMON_FILES)
