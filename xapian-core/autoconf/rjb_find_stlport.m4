dnl RJB_FIND_STLPORT
dnl Get paths and things for using stlport.
dnl At present, this macro does no auto-detection - it simply provides an
dnl easy way for the user to compile with STLport.
AC_DEFUN(RJB_FIND_STLPORT,
[dnl
dnl Find STLport
dnl


STLPORT_INCLUDE=""
STLPORT_LIBS=""
STLPORT_COMPILER=""
use_stlport=no

dnl Insert auto detection here, filling STLPORT_INCLUDE and STLPORT_LIBS,
dnl and setting use_stlport to yes.

AC_MSG_CHECKING([whether to use STLport])

AC_ARG_WITH(stlport-compiler,
[  --with-stlport-compiler=name  Set name of compiler to use with STLport],
[case "${withval}" in
  yes) use_stlport=yes ;;
  no)  AC_MSG_ERROR([Can't set STLport compiler to no]) ;;
  *)   use_stlport=yes ;
       STLPORT_COMPILER="${withval}" ;;
esac], [])

AC_ARG_WITH(stlport,
[  --with-stlport=path     Compile with STLport],
[case "${withval}" in
  yes) use_stlport=yes ;;
  no)  use_stlport=no ;;
  *)   use_stlport=yes ;
       STLPORT_INCLUDE="-I${withval}/stlport/" ;
       STLPORT_LIBS="-L${withval}/libs/" ;;
esac], [])

if test "x$use_stlport" = "xno" ; then
  STLPORT_INCLUDE=""
  STLPORT_LIBS=""
  AC_MSG_RESULT(no)
else
  if test "x$STLPORT_INCLUDE" = "x" ; then
    AC_MSG_RESULT()
    AC_MSG_ERROR([Don't know include path for STLport])
  fi
  if test "x$STLPORT_LIBS" = "x" ; then
    AC_MSG_RESULT()
    AC_MSG_ERROR([Don't know library path for STLport])
  fi
  if test "x$STLPORT_COMPILER" = "x" ; then
    AC_MSG_RESULT()
    AC_MSG_ERROR([Need to know name of compiler for compiling with STLport])
  fi
  STLPORT_LIBS="$STLPORT_LIBS -lstlport_${STLPORT_COMPILER}"
  AC_MSG_RESULT(yes)
fi

AC_SUBST(STLPORT_INCLUDE)
AC_SUBST(STLPORT_LIBS)

])
