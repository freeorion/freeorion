# FreeOrion AI Client
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
BIN  = freeorionca.exe

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
				$(filter-out ../../network/Server%,$(wildcard ../../network/*.cpp)) \
				$(filter-out ../../universe/Server%,$(wildcard ../../universe/*.cpp)) \
				$(wildcard ../../util/*.cpp)

OBJS = $(addprefix ../../objs/AI/,$(notdir $(patsubst %.cpp,%.o,$(CPP_SRCS))))

INCS =  -I/usr/include  -I/usr/include/GG  -I/usr/include/GG/net  -I$(SDL_INC)  -I$(BOOST_INC)
LIBS =  -L/usr/lib -L$(BOOST_SIG_LIB) -lSDLmain -lSDL -lboost_signals -lexpat -llog4cpp -lz -lpthread -lGiGi -lGiGiNet

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall -DFREEORION_BUILD_AI


###############################
## Targets and Rules ##########
###############################
.PHONY: all all-before all-after clean clean-custom

all: all-before freeorionca.exe all-after

all-before :
	mkdir -p ../../objs/AI

clean: clean-custom
	rm -f $(OBJS)

$(BIN): $(OBJS)
	$(CPP) $(OBJS) -o $(BIN) $(LIBS)
	/bin/mv $(BIN) ../../

../../objs/AI/%.o : %.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/AI/%.o : ../%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/AI/%.o : ../../Empire/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/AI/%.o : ../../network/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/AI/%.o : ../../universe/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@

../../objs/AI/%.o : ../../util/%.cpp
	$(CPP) $< -c $(CXXFLAGS) -o $@
