# FreeOrion Human Client
# This Makefile assumes you've installed boost in /usr/include/boost or /usr/local/include/boost, that
# you've built it using bjam (see boost build docs for explanation of bjam), and that you've put the 
# libraries in /usr/lib or /usr/local/lib

###############################
## Variables ##################
###############################
# Includes
SDL_INC = /usr/include/SDL
FREETYPE_INC = /usr/include/freetype2

# Executable name
BIN  = freeorion.exe

# Build commands
CC   = gcc -D__DEBUG__
CPP  = g++ -D__DEBUG__

# Lists of objects, includes, and libs
# Add new source code directories here as they are added to the project;
# also add a rule to the rules at the end that recognizes the new directory's
# sources.  Try to keep the directory names in alphabetical order.
CPP_SRCS =	$(wildcard *.cpp) \
				$(wildcard ../*.cpp) \
				$(filter-out ../../Empire/Server%,$(wildcard ../../Empire/*.cpp)) \
				$(filter-out ../../network/ClientNetworkCore.cpp,$(filter-out ../../network/Server%,$(wildcard ../../network/*.cpp))) \
				$(wildcard ../../UI/*.cpp) \
				$(filter-out ../../universe/Universe.cpp,$(wildcard ../../universe/*.cpp)) \
				$(filter-out ../../util/AppInterface.cpp,$(wildcard ../../util/*.cpp))

OBJS = $(addprefix ../../objs/,$(notdir $(patsubst %.cpp,%.o,$(CPP_SRCS)))) ../../objs/human/Universe.o ../../objs/human/AppInterface.o ../../objs/human/ClientNetworkCore.o

INCS =  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(FREETYPE_INC)  -I/usr/include/GG -I/usr/include/GG/net -I/usr/include/GG/SDL
LIBS =  -L/usr/lib -L/usr/X11R6/lib  -lSDLmain -lSDL -lSDL_mixer -lGL -lGLU -lfreetype -lboost_signals -lboost_filesystem -lboost_regex -lexpat -llog4cpp -lz -lpthread -lGiGi -lGiGiNet -lGiGiSDL

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall -DFREEORION_BUILD_HUMAN


###############################
## Targets and Rules ##########
###############################
.PHONY: all all-before all-after clean clean-custom

all: all-before freeorion.exe all-after

all-before :
	mkdir -p ../../objs/human

clean: clean-custom
	rm -f $(OBJS)

$(BIN): $(OBJS)
	$(CPP) $(OBJS) -o $(BIN) $(LIBS)
	/bin/mv $(BIN) ../../

../../objs/%.o : %.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/%.o : ../%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/%.o : ../../Empire/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/%.o : ../../network/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/%.o : ../../UI/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/%.o : ../../universe/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/%.o : ../../util/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/human/Universe.o : ../../universe/Universe.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/human/AppInterface.o : ../../util/AppInterface.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/human/ClientNetworkCore.o : ../../network/ClientNetworkCore.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@
