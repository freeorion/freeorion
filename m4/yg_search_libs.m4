#YG_SEARCH_LIBS(DESCRIPTION, LIBRARY-LIST, PROLOGUE, MAIN, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# like AC_SEARCH_LIBS, but takes a prologue and the exact syntax for calling the function
# !DOES NOT SET AC_LANG, SET AC_LANG_CPLUSPLUS OR AC_LANG_CC IF NEEDED!
# DEFINES:
#  adds -lLIBRARY to LIBS when found
# example: YG_SEARCH_LIBS([OpenGL libraries], [GL MesaGL opengl32], [#include <GL/gl.h>],[glEnd();])

AC_DEFUN([YG_SEARCH_LIBS],
[AS_VAR_PUSHDEF([YG_slib],[yg_cv_search_for_libs_$1_$2])dnl
 YG_LIBS_BAK="$LIBS"
 AC_CACHE_CHECK([for $1],YG_slib,
	       [AS_VAR_SET(YG_slib,missing)
		for lib in $2; do 
		  LIBS="-l$lib $YG_LIBS_BAK"
		  AC_LINK_IFELSE([AC_LANG_PROGRAM([$3],[$4])],[AS_VAR_SET(YG_slib,[-l$lib]);break])
                done])
 if test x[]AS_VAR_GET(YG_slib) = xmissing; then
    LIBS="$YG_LIBS_BAK"
    ifelse([$6], , :,[$6])
 else
    LIBS="$YG_LIBS_BAK []AS_VAR_GET(YG_slib)"
    ifelse([$5], , :,[$5])
 fi
 AS_VAR_POPDEF([YG_slib])
])
