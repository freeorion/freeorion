# FreeOrion Server
# This Makefile assumes you've installed boost in /usr/include/boost or /usr/local/include/boost, that
# you've built it using bjam (see boost build docs for explanation of bjam), and that you've put the 
# libraries in /usr/lib or /usr/local/lib

###############################
## Variables ##################
###############################
# Includes
SDL_INC = /usr/include/SDL

# Executable name
BIN  = freeoriond.exe

# Build commands
CC   = gcc -D__DEBUG__
CPP  = g++ -D__DEBUG__

# Lists of objects, includes, and libs
# Add new source code directories here as they are added to the project;
# also add a rule to the rules at the end that recognizes the new directory's
# sources.  Try to keep the directory names in alphabetical order.
CPP_SRCS =	$(wildcard *.cpp) \
				$(filter-out ../Empire/Client%,$(wildcard ../Empire/*.cpp)) \
				$(filter-out ../network/Client%,$(wildcard ../network/*.cpp)) \
				$(filter-out ../universe/Universe.cpp,$(wildcard ../universe/*.cpp)) \
				$(filter-out ../util/AppInterface.cpp,$(wildcard ../util/*.cpp))

OBJS = $(addprefix ../objs/,$(notdir $(patsubst %.cpp,%.o,$(CPP_SRCS)))) ../objs/server/AppInterface.o ../objs/server/Universe.o

INCS =  -I/usr/include  -I/usr/include/GG  -I/usr/include/GG/net  -I$(SDL_INC)
LIBS =  -L/usr/lib  -lSDLmain -lSDL -lboost_signals -lexpat -llog4cpp -lz -lpthread -lGiGi -lGiGiNet

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall -DFREEORION_BUILD_SERVER

###############################
## Targets and Rules ##########
###############################
.PHONY: all all-before all-after clean clean-custom

all: all-before freeoriond.exe all-after

all-before :
	mkdir -p ../objs/server

clean: clean-custom
	rm -f $(OBJS)

$(BIN): $(OBJS)
	$(CPP) $(OBJS) -o $(BIN) $(LIBS)
	/bin/mv $(BIN) ..

../objs/%.o : %.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/%.o : ../Empire/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/%.o : ../network/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/%.o : ../universe/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/%.o : ../util/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/server/AppInterface.o : ../util/AppInterface.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/server/Universe.o : ../universe/Universe.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@
