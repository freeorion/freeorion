AC_DEFUN([YG_GET_PATH],[dnl
AC_ARG_WITH($1,
[AC_HELP_STRING([--with-$1=DIR],[root directory of $1 installation])],
[with_$1=$withval 
if test "${with_$1}" != yes; then
	$1_include="$withval/include" 
	$1_libdir="$withval/lib"
fi])

AC_ARG_WITH($1-include,
[AC_HELP_STRING([--with-$1-include=DIR],[specify exact include dir for $1 headers])],
[$1_include="$withval"])

AS_VAR_PUSHDEF([YG_name],[AS_TR_CPP([$1_CPPFLAGS])])
if test x${$1_include} != x; then
  AS_VAR_SET(YG_name,[-I${$1_include}])
  CPPFLAGS="$CPPFLAGS -I${$1_include}"
else
  AS_VAR_SET(YG_name,[])
fi
AC_SUBST(YG_name)
AS_VAR_POPDEF([YG_name])

AC_ARG_WITH($1-libdir,
[AC_HELP_STRING([--with-$1-libdir=DIR],[specify exact library dir for $1 library])],
[$1_libdir="$withval"])

AS_VAR_PUSHDEF([YG_name],[AS_TR_CPP([$1_LDFLAGS])])
if test x${$1_libdir} != x; then
  AS_VAR_SET(YG_name,[-L${$1_libdir}])
  LDFLAGS="$LDFLAGS -L${$1_libdir}"
else
  AS_VAR_SET(YG_name,[])
fi
AC_SUBST(YG_name)
AS_VAR_POPDEF([YG_name])])
