# FreeOrion Server
# This Makefile assumes you've installed boost in /usr/boost_1_30_0, and that
# you've built it using bjam (see boost build docs for explanation of bjam).

CPP  = g++ -D__DEBUG__
CC   = gcc -D__DEBUG__

SDL_INC = /usr/include/SDL
FREETYPE_INC = /usr/include/freetype2
BOOST_INC = /usr/boost_1_30_0

BOOST_SIG_LIB = /usr/boost_1_30_0/libs/signals/build/bin/libboost_signals.a/gcc/debug/runtime-link-dynamic
#BOOST_PYTHON_LIB = /usr/boost_1_30_0/libs/python/build/bin/libboost_python.a/gcc/debug/runtime-link-dynamic

OBJ  = ../objs/dmain.o ../objs/ServerApp.o ../objs/SDLnetUDP.o ../objs/SDLnet.o ../objs/SDLnetselect.o ../objs/SDLnetTCP.o ../objs/fastevents.o ../objs/net2.o ../objs/trace.o ../objs/Message.o ../objs/NetworkCore.o ../objs/ServerNetworkCore.o ../objs/XDiff.o ../objs/XHash.o ../objs/XLut.o ../objs/XParser.o ../objs/XTree.o ../objs/XMLDoc.o ../objs/Process.o 
LINKOBJ  = ../objs/dmain.o ../objs/ServerApp.o ../objs/SDLnetUDP.o ../objs/SDLnet.o ../objs/SDLnetselect.o ../objs/SDLnetTCP.o ../objs/fastevents.o ../objs/net2.o ../objs/trace.o ../objs/Message.o ../objs/NetworkCore.o ../objs/ServerNetworkCore.o ../objs/XDiff.o ../objs/XHash.o ../objs/XLut.o ../objs/XParser.o ../objs/XTree.o ../objs/XMLDoc.o ../objs/Process.o

LIBS =  -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -lSDLmain -lSDL -lboost_signals -lexpat -llog4cpp -lz

INCS =  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(BOOST_INC)  -I../SDL_net  -I../SDL_net2  -I../GG  -I../GG/XML

CXXINCS =  $(INCS)

BIN  = ../freeoriond.exe

CXXFLAGS = $(CXXINCS) -g3
CFLAGS = $(INCS) -g3

.PHONY: all all-before all-after clean clean-custom

all: all-before ../freeoriond.exe all-after


clean: clean-custom
	rm -f $(OBJ) $(BIN)

$(BIN): $(LINKOBJ)
	$(CPP) $(LINKOBJ) -o "..\freeoriond.exe" $(LIBS)

../objs/dmain.o: dmain.cpp
	$(CPP) -c dmain.cpp -o ../objs/dmain.o $(CXXFLAGS)

../objs/ServerApp.o: ServerApp.cpp
	$(CPP) -c ServerApp.cpp -o ../objs/ServerApp.o $(CXXFLAGS)

../objs/SDLnetUDP.o: ../SDL_net/SDLnetUDP.c
	$(CC) -c ../SDL_net/SDLnetUDP.c -o ../objs/SDLnetUDP.o $(CFLAGS)

../objs/SDLnet.o: ../SDL_net/SDLnet.c
	$(CC) -c ../SDL_net/SDLnet.c -o ../objs/SDLnet.o $(CFLAGS)

../objs/SDLnetselect.o: ../SDL_net/SDLnetselect.c
	$(CC) -c ../SDL_net/SDLnetselect.c -o ../objs/SDLnetselect.o $(CFLAGS)

../objs/SDLnetTCP.o: ../SDL_net/SDLnetTCP.c
	$(CC) -c ../SDL_net/SDLnetTCP.c -o ../objs/SDLnetTCP.o $(CFLAGS)

../objs/fastevents.o: ../SDL_net2/fastevents.c
	$(CC) -c ../SDL_net2/fastevents.c -o ../objs/fastevents.o $(CFLAGS)

../objs/net2.o: ../SDL_net2/net2.c
	$(CC) -c ../SDL_net2/net2.c -o ../objs/net2.o $(CFLAGS)

../objs/trace.o: ../SDL_net2/trace.c
	$(CC) -c ../SDL_net2/trace.c -o ../objs/trace.o $(CFLAGS)

../objs/Message.o: ../network/Message.cpp
	$(CPP) -c ../network/Message.cpp -o ../objs/Message.o $(CXXFLAGS)

../objs/NetworkCore.o: ../network/NetworkCore.cpp
	$(CPP) -c ../network/NetworkCore.cpp -o ../objs/NetworkCore.o $(CXXFLAGS)

../objs/ServerNetworkCore.o: ../network/ServerNetworkCore.cpp
	$(CPP) -c ../network/ServerNetworkCore.cpp -o ../objs/ServerNetworkCore.o $(CXXFLAGS)

../objs/XDiff.o: ../network/XDiff.cpp
	$(CPP) -c ../network/XDiff.cpp -o ../objs/XDiff.o $(CXXFLAGS)

../objs/XHash.o: ../network/XHash.cpp
	$(CPP) -c ../network/XHash.cpp -o ../objs/XHash.o $(CXXFLAGS)

../objs/XLut.o: ../network/XLut.cpp
	$(CPP) -c ../network/XLut.cpp -o ../objs/XLut.o $(CXXFLAGS)

../objs/XParser.o: ../network/XParser.cpp
	$(CPP) -c ../network/XParser.cpp -o ../objs/XParser.o $(CXXFLAGS)

../objs/XTree.o: ../network/XTree.cpp
	$(CPP) -c ../network/XTree.cpp -o ../objs/XTree.o $(CXXFLAGS)

../objs/XMLDoc.o: ../GG/XML/XMLDoc.cpp
	$(CPP) -c ../GG/XML/XMLDoc.cpp -o ../objs/XMLDoc.o $(CXXFLAGS)

../objs/Process.o: ../util/Process.cpp
	$(CPP) -c ../util/Process.cpp -o ../objs/Process.o $(CXXFLAGS)
