# FreeOrion Human Client
# This Makefile assumes you've installed boost in /usr/boost_1_30_0, and that
# you've built it using bjam (see boost build docs for explanation of bjam).
# If you've put boost somewhere else, change BOOST_DIR accordingly.

###############################
## Variables ##################
###############################
# Directories
BOOST_DIR = /usr/boost_1_30_0

# Includes
SDL_INC = /usr/include/SDL
FREETYPE_INC = /usr/include/freetype2
BOOST_INC = $(BOOST_DIR)

# Libs
BOOST_SIG_LIB = $(BOOST_DIR)/libs/signals/build/bin/libboost_signals.a/gcc/debug/runtime-link-dynamic
BOOST_FS_LIB = $(BOOST_DIR)/libs/filesystem/build/bin/libboost_filesystem.a/gcc/debug/runtime-link-dynamic
BOOST_REGEX_LIB = $(BOOST_DIR)/libs/regex/build/bin/libboost_regex.a/gcc/debug/runtime-link-dynamic
#BOOST_PYTHON_LIB = $(BOOST_DIR)/libs/python/build/bin/libboost_python.a/gcc/debug/runtime-link-dynamic

# Executable name
BIN  = freeorion.exe

# Build commands
CC   = gcc -D__DEBUG__
CPP  = g++ -D__DEBUG__

# Lists of objects, includes, and libs
# Add object files to OBJ when they are added to the project
OBJ  = ../../objs/chmain.o ../../objs/HumanClientApp.o ../../objs/ClientApp.o ../../objs/GGApp.o ../../objs/GGButton.o ../../objs/GGClr.o ../../objs/GGControl.o ../../objs/GGDrawUtil.o ../../objs/GGDropDownList.o ../../objs/GGEdit.o ../../objs/GGListBox.o ../../objs/GGMenu.o ../../objs/GGPtRect.o ../../objs/GGScroll.o ../../objs/GGStaticGraphic.o ../../objs/GGText.o ../../objs/GGTextControl.o ../../objs/GGTexture.o ../../objs/GGWnd.o ../../objs/GGZList.o ../../objs/SDL_ttf.o ../../objs/SDLGGApp.o ../../objs/XMLDoc.o ../../objs/ClientNetworkCore.o ../../objs/Message.o ../../objs/NetworkCore.o ../../objs/XDiff.o ../../objs/XHash.o ../../objs/XLut.o ../../objs/XParser.o ../../objs/XTree.o ../../objs/SDLnetUDP.o ../../objs/SDLnetselect.o ../../objs/SDLnetTCP.o ../../objs/fastevents.o ../../objs/net2.o ../../objs/trace.o ../../objs/Process.o ../../objs/OrderSet.o ../../objs/SDLnet.o ../../objs/Order.o ../../objs/ClientUniverse.o ../../objs/UniverseObject.o ../../objs/Fleet.o ../../objs/Planet.o ../../objs/PopCenter.o ../../objs/ProdCenter.o ../../objs/ServerUniverse.o ../../objs/Ship.o ../../objs/System.o
INCS =  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(FREETYPE_INC)  -I$(BOOST_INC)  -I../../SDL_net  -I../../SDL_net2  -I../../GG  -I../../GG/XML
LIBS =  -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -L$(BOOST_FS_LIB) -L$(BOOST_REGEX_LIB) -lSDLmain -lSDL -lSDL_image -lSDL_mixer -lGL -lGLU -lfreetype -lboost_signals -lboost_filesystem -lboost_regex -lexpat -llog4cpp -lz -lpthread

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall -DFREEORION_BUILD_HUMAN

# Directories to search for source files
# Add directories here when they are added to the project
VPATH = ..:../..:../../Empire:../../GG:../../GG/XML:../../GG/dialogs:../../network:../../SDL_net:../../SDL_net2:../../UI:../../universe:../../util


###############################
## Targets and Rules ##########
###############################
.PHONY: all all-before all-after clean clean-custom

all: all-before freeorion.exe all-after


clean: clean-custom
	rm -f $(OBJ)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS)
	/bin/mv $(BIN) ..

../../objs/%.o : %.c
	$(CC) $< -c $(CFLAGS) -o $@

../../objs/%.o : %.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@






