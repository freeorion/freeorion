# FreeOrion Server
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
BOOST_INC = $(BOOST_DIR)

# Libs
BOOST_SIG_LIB = $(BOOST_DIR)/libs/signals/build/bin/libboost_signals.a/gcc/debug/runtime-link-dynamic
#BOOST_PYTHON_LIB = $(BOOST_DIR)/libs/python/build/bin/libboost_python.a/gcc/debug/runtime-link-dynamic

# Executable name
BIN  = freeoriond.exe

# Build commands
CC   = gcc -D__DEBUG__
CPP  = g++ -D__DEBUG__

# Lists of objects, includes, and libs
# Add object files to OBJ when they are added to the project
OBJ  = ../objs/server/dmain.o ../objs/server/ServerApp.o ../objs/server/SDLnetUDP.o ../objs/server/SDLnetselect.o ../objs/server/SDLnetTCP.o ../objs/server/fastevents.o ../objs/server/net2.o ../objs/server/trace.o ../objs/server/Message.o ../objs/server/NetworkCore.o ../objs/server/ServerNetworkCore.o ../objs/server/XDiff.o ../objs/server/XHash.o ../objs/server/XLut.o ../objs/server/XParser.o ../objs/server/XTree.o ../objs/server/XMLDoc.o ../objs/server/Process.o ../objs/server/SDLnet.o ../objs/server/Order.o ../objs/server/OrderSet.o ../objs/server/ServerUniverse.o ../objs/server/ClientUniverse.o ../objs/server/UniverseObject.o ../objs/server/System.o ../objs/server/Planet.o ../objs/server/PopCenter.o ../objs/server/ProdCenter.o ../objs/server/Ship.o ../objs/server/Fleet.o ../objs/server/SitRepEntry.o ../objs/server/Empire.o ../objs/server/EmpireManager.o ../objs/server/ServerEmpireManager.o ../objs/server/GGClr.o
INCS =  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(BOOST_INC)  -I../SDL_net  -I../SDL_net2  -I../GG  -I../GG/XML
LIBS =  -L/usr/lib -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -lSDLmain -lSDL -lboost_signals -lexpat -llog4cpp -lz -lpthread

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall -DFREEORION_BUILD_SERVER

# Directories to search for source files
# Add directories here when they are added to the project
VPATH = ..:../Empire:../GG:../GG/XML:../network:../SDL_net:../SDL_net2:../UI:../universe:../util


###############################
## Targets and Rules ##########
###############################
.PHONY: all all-before all-after clean clean-custom

all: all-before freeoriond.exe all-after


clean: clean-custom
	rm -f $(OBJ)

$(BIN): $(OBJ)
	$(CPP) $(OBJ) -o $(BIN) $(LIBS)
	/bin/mv $(BIN) ..

../objs/server/%.o : %.c
	$(CC) $< -c $(CFLAGS) -o $@

../objs/server/%.o : %.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

