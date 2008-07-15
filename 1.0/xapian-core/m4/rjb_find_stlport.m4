dnl @synopsis RJB_FIND_STLPORT
dnl
dnl Allow the user to specify paths and things for using stlport.
AC_DEFUN([RJB_FIND_STLPORT],
[dnl
dnl Find STLport
dnl
STLPORT_INCLUDE=""
STLPORT_LIBS=""
STLPORT_COMPILER=""
use_stlport=no

AC_MSG_CHECKING([whether to use STLport])

AC_ARG_WITH(stlport-compiler,
[  --with-stlport-compiler=name  Set name of compiler to use with STLport],
[case "${withval}" in
  yes) AC_MSG_ERROR([Can't set STLport compiler to yes]) ;;
  no)  AC_MSG_ERROR([Can't set STLport compiler to no]) ;;
  *)   use_stlport=yes ;
       STLPORT_COMPILER="${withval}" ;;
esac], [])

AC_ARG_WITH(stlport,
[  --with-stlport=path     Compile with STLport],
[case "${withval}" in
  yes) use_stlport=yes ;;
  no)  use_stlport=no ;;
  *)   use_stlport=yes
       if test -d "${withval}/stlport/"; then
         STLPORT_INCLUDE="-I${withval}/stlport/"
       else
         if test -d "${withval}/include/stlport"; then
	   STLPORT_INCLUDE="-I${withval}/include/stlport"
	 else
	   AC_MSG_RESULT()
	   AC_MSG_ERROR([Can't find STLport directory at specified path])
	 fi
       fi
       STLPORT_LIBS="-L${withval}/lib/" ;;
esac], [])

if test "x$use_stlport" = "xno" ; then
  STLPORT_INCLUDE=""
  STLPORT_LIBS=""
  AC_MSG_RESULT(no)
else
  if test -z "$STLPORT_INCLUDE" ; then
    AC_MSG_RESULT()
    AC_MSG_ERROR([Don't know include path for STLport])
  fi
  if test -z "$STLPORT_LIBS" ; then
    AC_MSG_RESULT()
    AC_MSG_ERROR([Don't know library path for STLport])
  fi
  if test -z "$STLPORT_COMPILER" ; then
    AC_MSG_RESULT()
    AC_MSG_ERROR([Need to know name of compiler for compiling with STLport])
  fi
  STLPORT_LIBS="$STLPORT_LIBS -lstlport_${STLPORT_COMPILER}"
  AC_MSG_RESULT(yes)
fi

AC_SUBST(STLPORT_INCLUDE)
AC_SUBST(STLPORT_LIBS)
])
