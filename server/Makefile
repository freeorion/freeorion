# FreeOrion Server
# This Makefile assumes you've installed boost in /usr/boost_1_30_0, and that
# you've built it using bjam (see boost build docs for explanation of bjam).
# If you've put boost somewhere else, change BOOST_DIR accordingly.
#

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
#BOOST_PYTHON_LIB = $(BOOST_DIR)/libs/python/build/bin/libboost_python.a/gcc/debug/runtime-link-dynamic

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
				$(wildcard ../universe/*.cpp) \
				$(wildcard ../util/*.cpp)

OBJS = $(addprefix ../objs/server/,$(notdir $(patsubst %.cpp,%.o,$(CPP_SRCS))))

INCS =  -I/usr/include  -I/usr/include/GG  -I/usr/include/GG/net  -I$(SDL_INC)  -I$(FREETYPE_INC) -I$(BOOST_INC)
LIBS =  -L/usr/lib -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -lSDLmain -lSDL -lfreetype -lboost_signals -lexpat -llog4cpp -lz -lpthread -lGiGi

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

all-before :
	mkdir -p ../objs/server

clean: clean-custom
	rm -f $(OBJS)

$(BIN): $(OBJS)
	$(CPP) $(OBJS) -o $(BIN) $(LIBS)
	/bin/mv $(BIN) ..

../objs/server/%.o : %.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/server/%.o : ../Empire/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/server/%.o : ../network/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/server/%.o : ../universe/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../objs/server/%.o : ../util/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@
