# a macro to get the libs/cflags for libomus
# serial 1

dnl AM_PATH_LIBOMUS([ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to LIBOMUS
dnl Defines LIBOMUS_CFLAGS and LIBOMUS_LIBS
dnl
AC_DEFUN(AM_PATH_LIBOMUS,
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


if test "x$LIBOMUS_UNINST" = "xyes"; then
  AC_MSG_ERROR(--with-libomus-uninst needs path of top dir of libomus)
fi

if test "x$LIBOMUS_UNINST_EXEC" = "xyes"; then
  AC_MSG_ERROR(--with-libomus-uninst-exec needs path of top dir of libomus build)
fi

AC_MSG_CHECKING(for libomus)

if test "x$LIBOMUS_UNINST" = "x"; then
  if test "x$LIBOMUS_UNINST_EXEC" != "x"; then
    AC_MSG_ERROR(must specify --with-libomus-uninst if using --with-libomus-uninst-exec)
  fi
  if test "x$LIBOMUS_CONFIG" = "x"; then
    AC_PATH_PROG(LIBOMUS_CONFIG, libomus-config, no)
  fi
  
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
  AC_MSG_RESULT(using uninstalled version)
  if test "x$LIBOMUS_UNINST_EXEC" = "x"; then
    LIBOMUS_UNINST_EXEC=no
  fi
  if test "x$LIBOMUS_UNINST_EXEC" = "xno"; then
    LIBOMUS_UNINST_EXEC=$LIBOMUS_UNINST
  fi
  LIBOMUS_CFLAGS="-I$LIBOMUS_UNINST/include -I$LIBOMUS_UNINST/common"
  LIBOMUS_LIBS="$LIBOMUS_UNINST_EXEC/libomus.la"
fi

AC_SUBST(LIBOMUS_CFLAGS)
AC_SUBST(LIBOMUS_LIBS)
])
