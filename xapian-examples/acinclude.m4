# a macro to get the libs/cflags for libomus
# serial 1

dnl OM_PATH_LIBOMUS([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to LIBOMUS
dnl Defines LIBOMUS_CFLAGS and LIBOMUS_LIBS
dnl
AC_DEFUN(OM_PATH_LIBOMUS,
[dnl
dnl Get the cflags and libraries from the libomus-config script
dnl
AC_ARG_WITH(libomus-config,
[  --with-libomus-config   Location of libomus-config],
LIBOMUS_CONFIG="$withval")

AC_ARG_WITH(libomus-uninst,
[  --with-libomus-uninst   Location of top dir of uninstalled libomus],
LIBOMUS_UNINST="$withval")

AC_ARG_WITH(libomus-uninst_exec,
[  --with-libomus-uninst-exec  Location of built libomus, if srcdir not builddir],
LIBOMUS_UNINST_EXEC="$withval")


if test "x$LIBOMUS_CONFIG" = "x"; then
  LIBOMUS_CONFIG=no
fi

if test "x$LIBOMUS_UNINST" = "xyes"; then
  AC_MSG_ERROR(--with-libomus-uninst needs path of top dir of libomus)
fi
if test "x$LIBOMUS_UNINST" = "x"; then
  LIBOMUS_UNINST=no
fi

if test "x$LIBOMUS_UNINST_EXEC" = "xyes"; then
  AC_MSG_ERROR(--with-libomus-uninst-exec needs path of top dir of libomus build)
fi
if test "x$LIBOMUS_UNINST_EXEC" = "x"; then
  LIBOMUS_UNINST_EXEC=no
fi


dnl If paths aren't absolute, complain
case x$LIBOMUS_UNINST in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for libomus-uninst must be absolute (was $LIBOMUS_UNINST)]) ;;
esac
case x$LIBOMUS_UNINST_EXEC in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for libomus-uninst-exec must be absolute (was $LIBOMUS_UNINST_EXEC)]) ;;
esac

dnl LIBOMUS_UNINST and LIBOMUS_UNINST_EXEC are either an absolute path or no

if test "x$LIBOMUS_UNINST" = "xno"; then
  if test "x$LIBOMUS_UNINST_EXEC" != "xno"; then
    AC_MSG_ERROR(must specify --with-libomus-uninst if using --with-libomus-uninst-exec)
  fi
  if test "x$LIBOMUS_CONFIG" = "xno"; then
    AC_PATH_PROG(LIBOMUS_CONFIG, libomus-config, no)
  fi
  
  AC_MSG_CHECKING(for libomus)
  if test "x$LIBOMUS_CONFIG" = "xno"; then
    AC_MSG_RESULT(not found)
    ifelse([$2], , :, [$2])
  else
    if $LIBOMUS_CONFIG --check; then
      AC_MSG_RESULT(yes)
      LIBOMUS_CFLAGS=`$LIBOMUS_CONFIG --cflags $module_args`
      LIBOMUS_LIBS=`$LIBOMUS_CONFIG --libs $module_args`
      ifelse([$1], , :, [$1])
    else
      AC_MSG_RESULT(bad installation)
    fi
  fi
else
  AC_MSG_CHECKING(for libomus)

  AC_MSG_RESULT(using uninstalled version)
  if test "x$LIBOMUS_UNINST_EXEC" = "xno"; then
    LIBOMUS_UNINST_EXEC=$LIBOMUS_UNINST
  fi
  LIBOMUS_CFLAGS="-I$LIBOMUS_UNINST/include"
  LIBOMUS_LIBS="$LIBOMUS_UNINST_EXEC/libomus.la"
fi

AC_SUBST(LIBOMUS_CFLAGS)
AC_SUBST(LIBOMUS_LIBS)
])

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
       STLPORT_LIBS="-L${withval}/lib/" ;;
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
