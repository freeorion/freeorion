# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(FreeOrion, 0.1) dnl no bug-report address for now [bugs@freeorion.org])

dnl for cross-compilation
dnl BUILD is the system that is compiling
dnl HOST  is the system that the programs will run under
dnl TARGET is needed only if compiling a compiler, but AC_PATH_SDL needs it (a bug?)
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

case "$host" in
   *-*-mingw* | *-*-cygwin*)
      CPPFLAGS="-DFREEORION_WIN32 $CPPFLAGS"
      AC_MSG_NOTICE([Building WINDOWS version])
      ;;
   *-*-linux*)
      CPPFLAGS="-DFREEORION_LINUX $CPPFLAGS"
      AC_MSG_NOTICE([Building UNIX version])
      ;;
   *)
      AC_MSG_WARN([Host system '$host' unknown, assuming Linux/UNIX])
      CPPFLAGS="-DFREEORION_LINUX $CPPFLAGS"
      ;;
esac

AC_CONFIG_SRCDIR([server/ServerApp.cpp])
AM_CONFIG_HEADER([config.h])

dnl Do not use subdir-objects for now.
dnl this would compile each required file only once, but some files
dnl have #ifdef's depending whether they are built for client or server!
AM_INIT_AUTOMAKE(foreign) dnl subdir-objects)
 
# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
#AM_PROG_CC_C_O
AC_PROG_LN_S
AC_PROG_CPP
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h inttypes.h memory.h netdb.h netinet/in.h stddef.h stdlib.h string.h sys/socket.h sys/time.h syslog.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_MMAP
AC_FUNC_SELECT_ARGTYPES
AC_FUNC_SETVBUF_REVERSED
AC_TYPE_SIGNAL
AC_SEARCH_LIBS(sqrt,[m],[],[AC_MSG_FAILURE([The sqrt-function was not found. This is bad!])])
AC_CHECK_FUNCS([floor gethostbyaddr gethostbyname memmove memset munmap pow select socket strcasecmp])


# Checks for libraries.
AC_CACHE_SAVE

# Check for SDL
SDL_CONFIG=no
if test x$cross_compiling = xno; then
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
fi
if test "$SDL_CONFIG" = "no"; then
   if test $WIN32 = yes; then
    # Windows target
    AC_MSG_NOTICE([Target is WIN32, won't rely on sdl-config])
    YG_GET_PATH(sdl)
    SDL_CFLAGS=$SDL_CPPFLAGS
    AC_SUBST(SDL_CFLAGS)
    SDL_LIBS="-lSDL -lSDLmain"
    AC_SUBST(SDL_LIBS)
    OLD_LIBS=$LIBS
    LIBS="$OLD_LIBS $SDL_LIBS"
    AC_LANG_C
    AC_CHECK_HEADERS([SDL.h])
    AC_TRY_LINK([#include "SDL.h"],[SDL_Init(SDL_INIT_VIDEO);],[],[AC_MSG_FAILURE([dnl
Unable to link SDL test program, did you specify the correct paths?
(--with-sdl-include,--with-sdl-libdir)? See ./configure --help])])
    LIBS=$OLD_LIBS
  else
     # Unix target, we require sdl-config here (at least for now, to eliminate strange linking errors)
     AM_PATH_SDL([1.2.0],[],[AC_MSG_ERROR([SDL is required])])
  fi
else
  AM_PATH_SDL([1.2.0],[],[AC_MSG_ERROR([SDL is required])])
fi

AC_CACHE_SAVE
#Check for FreeType
FT_CONFIG=no
if test x$cross_compiling = xno; then
  AC_PATH_PROG(FT_CONFIG, freetype-config, no, [$PATH])
fi
if test "$FT_CONFIG" = "no"; then
  if test $WIN32 = yes; then
    # Windows target
    AC_MSG_NOTICE([Target is WIN32, won't rely on freetype-config])
    YG_GET_PATH(ft)
    FT2_CFLAGS=$FT_CPPFLAGS
    AC_SUBST(FT2_CFLAGS)
    FT2_LIBS="-lfreetype $FT2_LDADD"
    AC_SUBST(FT2_LIBS)
    OLD_LIBS=$LIBS
    LIBS="$OLD_LIBS $FT2_LIBS"
    AC_LANG_C
    AC_CHECK_HEADERS([ft2build.h])
    AC_TRY_LINK([
#include "ft2build.h"
#include FT_FREETYPE_H],[
  FT_Library library;
  FT_Init_FreeType(&library);],[],[AC_MSG_FAILURE([
Freetype was not found, did you specify the correct paths
(--with-ft-include,--with-ft-libdir)? See ./configure --help])])
    LIBS=$OLD_LIBS
  else
     # Unix target, we require freetype-config here (at least for now, to eliminate strange linking errors)
     AC_CHECK_FT2(9.0.0,[],[AC_MSG_ERROR([FreeType is required])])
  fi
else
  AC_CHECK_FT2(9.0.0,[],[AC_MSG_ERROR([FreeType is required])])
fi

AC_CACHE_SAVE

#YG_GET_PATH(sdl_image)
#X_OLD_CPPFLAGS="$CPPFLAGS"
#CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
#AC_CHECK_HEADERS([SDL_image.h],[],[AC_MSG_ERROR(missing header)])
#AC_CHECK_LIB(SDL_image,IMG_Load,[],[AC_MSG_FAILURE([SDL_image is needed])])
#CPPFLAGS="$X_OLD_CPPFLAGS"

# Check for OpenGL
AC_CHECK_HEADERS([GL/gl.h GL/glu.h],,[AC_MSG_FAILURE([I can't find the OpenGL headers.
Make sure you have the OpenGL-developer-libraries installed and have given
the correct include path!])])

YG_SEARCH_LIBS([OpenGl],[GL MesaGL opengl32],
               [#include <GL/gl.h>],[glEnd();],
               [],[AC_MSG_FAILURE([OpenGL headers were found, but unable to link a simple GL program!])])
YG_SEARCH_LIBS([OpenGL's GLU],[GLU MesaGLU glu32],
               [#include <GL/glu.h>],[gluPerspective(0.,0.,0.,0.);],
               [],[AC_MSG_FAILURE([OpenGL's GLU library was not found!])])

AC_CACHE_SAVE
# Check for Boost
FAILED=no
YG_GET_PATH(boost)
YG_CHECK_BOOST([1.3.0],,[FAILED=yes])
#AC_CHECK_LIB(boost_regex,main,[],[FAILED=yes])
AC_CHECK_LIB(boost_filesystem,main,[],[FAILED=yes])
AC_CHECK_LIB(boost_signals,main,[],[FAILED=yes])

if test $FAILED = yes; then 
  AC_MSG_FAILURE([boost libs not found.
** You need the Boost C++ libraries, version 1.3.0 or better,
** as well as the following boost-modules:
** - boost.filesystem
** - boost.signals
** download them from http://www.boost.org])
fi

# Check for Expat
#YG_GET_PATH(expat)
#AC_CHECK_LIB(expat,XML_ParserCreate,[],[AC_MSG_FAILURE([You'll need expat])])

# Check for log4cpp
YG_GET_PATH(log4cpp)
AC_LANG_CPLUSPLUS
AC_CHECK_HEADERS([log4cpp/Category.hh],,[AC_MSG_FAILURE(log4cpp/Category.hh not found in include-path)])
YG_SEARCH_LIBS([for log4cpp],[log4cpp],
               [#include <log4cpp/Category.hh>],[log4cpp::Category::shutdown();],
               [],[AC_MSG_FAILURE(log4cpp is missing)])
dnl AC_CHECK_LIB(z,deflate,[],[AC_MSG_FAILURE(Zlib is missing!)])

# Check for SDL_mixer
AC_CHECK_LIB(SDL_mixer,Mix_OpenAudio,[],[AC_MSG_FAILURE([SDL_mixer not found])])

# Search for GiGi
FOUND=0
if test $cross_compiling = no; then
  PKG_CHECK_MODULES(GIGI, GiGi, [FOUND=1], [FOUND=0])
  PKG_CHECK_MODULES(GIGINET, GiGiNet, [FOUND=1], [FOUND=0])
  PKG_CHECK_MODULES(GIGISDL, GiGiSDL, [FOUND=1], [FOUND=0])
fi

if test $FOUND = 0; then
  # either we are cross compiling, or pkg-config was not found, so 
  # let's try it ourselves
  YG_GET_PATH(gigi)
  AC_LANG_CPLUSPLUS
  AC_CHECK_LIB(GiGi,main,[],[AC_MSG_FAILURE(The GiGi library was not found)])
  AC_CHECK_LIB(GiGiSDL,main,[],[AC_MSG_FAILURE(The GiGiSDL library was not found)])
  AC_CHECK_LIB(GiGiNet,main,[],[AC_MSG_FAILURE(The GiGiNet library was not found)])
  GIGI_LIBS="-lGiGi"
  GIGISDL_LIBS="-lGiGiSDL -lGiGi $SDL_LIBS"
  GIGINET_LIBS="-lGiGiNet $SDL_LIBS"
  GIGI_CFLAGS=""
  GIGISDL_CFLAGS="$SDL_CFLAGS"
  GIGINET_CFLAGS="$SDL_CFLAGS"
  AC_SUBST(GIGI_LIBS)
  AC_SUBST(GIGISDL_LIBS)
  AC_SUBST(GIGINET_LIBS)
  AC_SUBST(GIGI_CFLAGS)
  AC_SUBST(GIGISDL_CFLAGS)
  AC_SUBST(GIGINET_CFLAGS)
fi

AC_CONFIG_FILES([Makefile
		 Empire/Makefile
		 network/Makefile
		 UI/Makefile
		 universe/Makefile
		 util/Makefile 
		 combat/Makefile
                 server/Makefile 
  		 client/Makefile
		   client/human/Makefile
		   client/AI/Makefile
		])
AC_OUTPUT
