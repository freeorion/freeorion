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
# Add new source code directories here as they are added to the project;
# also add a rule to the rules at the end that recognizes the new directory's
# sources.  Try to keep the directory names in alphabetical order.
CPP_SRCS =	$(wildcard *.cpp) \
				$(wildcard ../*.cpp) \
				$(filter-out ../../Empire/Server%,$(wildcard ../../Empire/*.cpp)) \
				$(filter-out ../../network/Server%,$(wildcard ../../network/*.cpp)) \
				$(wildcard ../../UI/*.cpp) \
				$(filter-out ../../universe/Server%,$(wildcard ../../universe/*.cpp)) \
				$(wildcard ../../util/*.cpp)

OBJS = $(addprefix ../../objs/,$(notdir $(patsubst %.cpp,%.o,$(CPP_SRCS))))

INCS =  -I/usr/include  -I/usr/include/GL  -I$(SDL_INC)  -I$(FREETYPE_INC)  -I$(BOOST_INC) -I/usr/include/GG -I/usr/include/GG/net
LIBS =  -L/usr/lib -L/usr/X11R6/lib -L$(BOOST_SIG_LIB) -L$(BOOST_FS_LIB) -L$(BOOST_REGEX_LIB) -lSDLmain -lSDL -lSDL_image -lSDL_mixer -lGL -lGLU -lfreetype -lboost_signals -lboost_filesystem -lboost_regex -lexpat -llog4cpp -lz -lpthread -lGiGi

# Build Flags
CFLAGS = $(INCS) -g3 -Wall
CXXFLAGS = $(INCS) -g3 -Wall -DFREEORION_BUILD_HUMAN


###############################
## Targets and Rules ##########
###############################
.PHONY: all all-before all-after clean clean-custom

all: all-before freeorion.exe all-after

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

