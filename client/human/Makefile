# FreeOrion Human Client
# This Makefile assumes you've installed boost in /usr/boost_1_30_0, and that
# you've built it using bjam (see boost build docs for explanation of bjam).

CPP  = g++ -D__DEBUG__
CC   = gcc -D__DEBUG__

SDL_INC = /usr/include/SDL
FREETYPE_INC = /usr/include/freetype2
BOOST_INC = /usr/boost_1_30_0

BOOST_SIG_LIB = /usr/boost_1_30_0/libs/signals/build/bin/libboost_signals.a/gcc/debug/runtime-link-dynamic
#BOOST_PYTHON_LIB = /usr/boost_1_30_0/libs/python/build/bin/libboost_python.a/gcc/debug/runtime-link-dynamic

OBJ  = ../../objs/chmain.o ../../objs/HumanClientApp.o ../../objs/ClientApp.o ../../objs/GGApp.o ../../objs/GGButton.o ../../objs/GGClr.o ../../objs/GGControl.o ../../objs/GGDrawUtil.o ../../objs/GGDropDownList.o ../../objs/GGEdit.o ../../objs/GGListBox.o ../../objs/GGMenu.o ../../objs/GGPtRect.o ../../objs/GGScroll.o ../../objs/GGStaticGraphic.o ../../objs/GGText.o ../../objs/GGTextControl.o ../../objs/GGTexture.o ../../objs/GGWnd.o ../../objs/GGZList.o ../../objs/SDL_ttf.o ../../objs/SDLGGApp.o ../../objs/XMLDoc.o ../../objs/ClientNetworkCore.o ../../objs/Message.o ../../objs/NetworkCore.o ../../objs/XDiff.o ../../objs/XHash.o ../../objs/XLut.o ../../objs/XParser.o ../../objs/XTree.o ../../objs/SDLnetUDP.o ../../objs/SDL_net.o ../../objs/SDLnetselect.o ../../objs/SDLnetTCP.o ../../objs/fastevents.o ../../objs/net2.o ../../objs/trace.o ../../objs/Process.o ../../objs/OrderSet.o
LINKOBJ  = ../../objs/chmain.o ../../objs/HumanClientApp.o ../../objs/ClientApp.o ../../objs/GGApp.o ../../objs/GGButton.o ../../objs/GGClr.o ../../objs/GGControl.o ../../objs/GGDrawUtil.o ../../objs/GGDropDownList.o ../../objs/GGEdit.o ../../objs/GGListBox.o ../../objs/GGMenu.o ../../objs/GGPtRect.o ../../objs/GGScroll.o ../../objs/GGStaticGraphic.o ../../objs/GGText.o ../../objs/GGTextControl.o ../../objs/GGTexture.o ../../objs/GGWnd.o ../../objs/GGZList.o ../../objs/SDL_ttf.o ../../objs/SDLGGApp.o ../../objs/XMLDoc.o ../../objs/ClientNetworkCore.o ../../objs/Message.o ../../objs/NetworkCore.o ../../objs/XDiff.o ../../objs/XHash.o ../../objs/XLut.o ../../objs/XParser.o ../../objs/XTree.o ../../objs/SDLnetUDP.o ../../objs/SDL_net.o ../../objs/SDLnetselect.o ../../objs/SDLnetTCP.o ../../objs/fastevents.o ../../objs/net2.o ../../objs/trace.o ../../objs/Process.o ../../objs/OrderSet.o

LIBS =  -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -lSDLmain -lSDL -lSDL_image -lGL -lGLU -lfreetype -lboost_signals -lexpat -llog4cpp -lz

INCS =  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(FREETYPE_INC)  -I$(BOOST_INC)  -I../../SDL_net  -I../../SDL_net2  -I../../GG  -I../../GG/XML

CXXINCS =  $(INCS)

BIN  = ../../freeorionca.exe

CXXFLAGS = $(CXXINCS) -g3
CFLAGS = $(INCS) -g3

.PHONY: all all-before all-after clean clean-custom

all: all-before ../../freeorion.exe all-after


clean: clean-custom
	rm -f $(OBJ) $(BIN)

$(BIN): $(LINKOBJ)
	$(CPP) $(LINKOBJ) -o "..\..\freeorion.exe" $(LIBS)

../../objs/chmain.o: chmain.cpp
	$(CPP) -c chmain.cpp -o ../../objs/chmain.o $(CXXFLAGS)

../../objs/HumanClientApp.o: HumanClientApp.cpp
	$(CPP) -c HumanClientApp.cpp -o ../../objs/HumanClientApp.o $(CXXFLAGS)

../../objs/ClientApp.o: ../ClientApp.cpp
	$(CPP) -c ../ClientApp.cpp -o ../../objs/ClientApp.o $(CXXFLAGS)

../../objs/GGApp.o: ../../GG/GGApp.cpp
	$(CPP) -c ../../GG/GGApp.cpp -o ../../objs/GGApp.o $(CXXFLAGS)

../../objs/GGButton.o: ../../GG/GGButton.cpp
	$(CPP) -c ../../GG/GGButton.cpp -o ../../objs/GGButton.o $(CXXFLAGS)

../../objs/GGClr.o: ../../GG/GGClr.cpp
	$(CPP) -c ../../GG/GGClr.cpp -o ../../objs/GGClr.o $(CXXFLAGS)

../../objs/GGControl.o: ../../GG/GGControl.cpp
	$(CPP) -c ../../GG/GGControl.cpp -o ../../objs/GGControl.o $(CXXFLAGS)

../../objs/GGDrawUtil.o: ../../GG/GGDrawUtil.cpp
	$(CPP) -c ../../GG/GGDrawUtil.cpp -o ../../objs/GGDrawUtil.o $(CXXFLAGS)

../../objs/GGDropDownList.o: ../../GG/GGDropDownList.cpp
	$(CPP) -c ../../GG/GGDropDownList.cpp -o ../../objs/GGDropDownList.o $(CXXFLAGS)

../../objs/GGEdit.o: ../../GG/GGEdit.cpp
	$(CPP) -c ../../GG/GGEdit.cpp -o ../../objs/GGEdit.o $(CXXFLAGS)

../../objs/GGListBox.o: ../../GG/GGListBox.cpp
	$(CPP) -c ../../GG/GGListBox.cpp -o ../../objs/GGListBox.o $(CXXFLAGS)

../../objs/GGMenu.o: ../../GG/GGMenu.cpp
	$(CPP) -c ../../GG/GGMenu.cpp -o ../../objs/GGMenu.o $(CXXFLAGS)

../../objs/GGPtRect.o: ../../GG/GGPtRect.cpp
	$(CPP) -c ../../GG/GGPtRect.cpp -o ../../objs/GGPtRect.o $(CXXFLAGS)

../../objs/GGScroll.o: ../../GG/GGScroll.cpp
	$(CPP) -c ../../GG/GGScroll.cpp -o ../../objs/GGScroll.o $(CXXFLAGS)

../../objs/GGStaticGraphic.o: ../../GG/GGStaticGraphic.cpp
	$(CPP) -c ../../GG/GGStaticGraphic.cpp -o ../../objs/GGStaticGraphic.o $(CXXFLAGS)

../../objs/GGText.o: ../../GG/GGText.cpp
	$(CPP) -c ../../GG/GGText.cpp -o ../../objs/GGText.o $(CXXFLAGS)

../../objs/GGTextControl.o: ../../GG/GGTextControl.cpp
	$(CPP) -c ../../GG/GGTextControl.cpp -o ../../objs/GGTextControl.o $(CXXFLAGS)

../../objs/GGTexture.o: ../../GG/GGTexture.cpp
	$(CPP) -c ../../GG/GGTexture.cpp -o ../../objs/GGTexture.o $(CXXFLAGS)

../../objs/GGWnd.o: ../../GG/GGWnd.cpp
	$(CPP) -c ../../GG/GGWnd.cpp -o ../../objs/GGWnd.o $(CXXFLAGS)

../../objs/GGZList.o: ../../GG/GGZList.cpp
	$(CPP) -c ../../GG/GGZList.cpp -o ../../objs/GGZList.o $(CXXFLAGS)

../../objs/SDL_ttf.o: ../../GG/SDL_ttf.c
	$(CC) -c ../../GG/SDL_ttf.c -o ../../objs/SDL_ttf.o $(CFLAGS)

../../objs/SDLGGApp.o: ../../GG/SDLGGApp.cpp
	$(CPP) -c ../../GG/SDLGGApp.cpp -o ../../objs/SDLGGApp.o $(CXXFLAGS)

../../objs/XMLDoc.o: ../../GG/XML/XMLDoc.cpp
	$(CPP) -c ../../GG/XML/XMLDoc.cpp -o ../../objs/XMLDoc.o $(CXXFLAGS)

../../objs/ClientNetworkCore.o: ../../network/ClientNetworkCore.cpp
	$(CPP) -c ../../network/ClientNetworkCore.cpp -o ../../objs/ClientNetworkCore.o $(CXXFLAGS)

../../objs/Message.o: ../../network/Message.cpp
	$(CPP) -c ../../network/Message.cpp -o ../../objs/Message.o $(CXXFLAGS)

../../objs/NetworkCore.o: ../../network/NetworkCore.cpp
	$(CPP) -c ../../network/NetworkCore.cpp -o ../../objs/NetworkCore.o $(CXXFLAGS)

../../objs/XDiff.o: ../../network/XDiff.cpp
	$(CPP) -c ../../network/XDiff.cpp -o ../../objs/XDiff.o $(CXXFLAGS)

../../objs/XHash.o: ../../network/XHash.cpp
	$(CPP) -c ../../network/XHash.cpp -o ../../objs/XHash.o $(CXXFLAGS)

../../objs/XLut.o: ../../network/XLut.cpp
	$(CPP) -c ../../network/XLut.cpp -o ../../objs/XLut.o $(CXXFLAGS)

../../objs/XParser.o: ../../network/XParser.cpp
	$(CPP) -c ../../network/XParser.cpp -o ../../objs/XParser.o $(CXXFLAGS)

../../objs/XTree.o: ../../network/XTree.cpp
	$(CPP) -c ../../network/XTree.cpp -o ../../objs/XTree.o $(CXXFLAGS)

../../objs/SDLnetUDP.o: ../../SDL_net/SDLnetUDP.c
	$(CC) -c ../../SDL_net/SDLnetUDP.c -o ../../objs/SDLnetUDP.o $(CFLAGS)

../../objs/SDL_net.o: ../../SDL_net/SDL_net.c
	$(CC) -c ../../SDL_net/SDL_net.c -o ../../objs/SDL_net.o $(CFLAGS)

../../objs/SDLnetselect.o: ../../SDL_net/SDLnetselect.c
	$(CC) -c ../../SDL_net/SDLnetselect.c -o ../../objs/SDLnetselect.o $(CFLAGS)

../../objs/SDLnetTCP.o: ../../SDL_net/SDLnetTCP.c
	$(CC) -c ../../SDL_net/SDLnetTCP.c -o ../../objs/SDLnetTCP.o $(CFLAGS)

../../objs/fastevents.o: ../../SDL_net2/fastevents.c
	$(CC) -c ../../SDL_net2/fastevents.c -o ../../objs/fastevents.o $(CFLAGS)

../../objs/net2.o: ../../SDL_net2/net2.c
	$(CC) -c ../../SDL_net2/net2.c -o ../../objs/net2.o $(CFLAGS)

../../objs/trace.o: ../../SDL_net2/trace.c
	$(CC) -c ../../SDL_net2/trace.c -o ../../objs/trace.o $(CFLAGS)

../../objs/Process.o: ../../util/Process.cpp
	$(CPP) -c ../../util/Process.cpp -o ../../objs/Process.o $(CXXFLAGS)

../../objs/OrderSet.o: ../../util/OrderSet.cpp
	$(CPP) -c ../../util/OrderSet.cpp -o ../../objs/OrderSet.o $(CXXFLAGS)
