# a macro to get the libs/cflags for omsee library
# serial 2

dnl OM_PATH_OMSEE[ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to Omsee library
dnl Defines OMSEE_CFLAGS and OMSEE_LIBS
dnl
AC_DEFUN(OM_PATH_OMSEE,
[dnl
dnl Get the cflags and libraries from the omsee-config script
dnl
AC_ARG_WITH(omsee-config,
[  --with-omsee-config   Location of omsee-config],
OMSEE_CONFIG="$withval")

AC_ARG_WITH(omsee-uninst,
[  --with-omsee-uninst   Location of top dir of uninstalled omsee],
OMSEE_UNINST="$withval")

AC_ARG_WITH(omsee-uninst_exec,
[  --with-omsee-uninst-exec  Location of built omsee, if srcdir not builddir],
OMSEE_UNINST_EXEC="$withval")


if test "x$OMSEE_CONFIG" = "x"; then
  OMSEE_CONFIG=no
fi

if test "x$OMSEE_UNINST" = "xyes"; then
  AC_MSG_ERROR(--with-omsee-uninst needs path of top dir of omsee)
fi
if test "x$OMSEE_UNINST" = "x"; then
  OMSEE_UNINST=no
fi

if test "x$OMSEE_UNINST_EXEC" = "xyes"; then
  AC_MSG_ERROR(--with-omsee-uninst-exec needs path of top dir of omsee build)
fi
if test "x$OMSEE_UNINST_EXEC" = "x"; then
  OMSEE_UNINST_EXEC=no
fi


dnl If paths aren't absolute, complain
case x$OMSEE_UNINST in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for omsee-uninst must be absolute (was $OMSEE_UNINST)]) ;;
esac
case x$OMSEE_UNINST_EXEC in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for omsee-uninst-exec must be absolute (was $OMSEE_UNINST_EXEC)]) ;;
esac

dnl OMSEE_UNINST and OMSEE_UNINST_EXEC are either an absolute path or no

if test "x$OMSEE_UNINST" = "xno"; then
  if test "x$OMSEE_UNINST_EXEC" != "xno"; then
    AC_MSG_ERROR(must specify --with-omsee-uninst if using --with-omsee-uninst-exec)
  fi
  if test "x$OMSEE_CONFIG" = "xno"; then
    AC_PATH_PROG(OMSEE_CONFIG, omsee-config, no)
  fi

  AC_MSG_CHECKING(for omsee)
  if test "x$OMSEE_CONFIG" = "xno"; then
    AC_MSG_RESULT(not found)
    ifelse([$2], , :, [$2])
  else
    if $OMSEE_CONFIG --check; then
      AC_MSG_RESULT(yes)
      OMSEE_CFLAGS=`$OMSEE_CONFIG --cflags $module_args`
      OMSEE_LIBS=`$OMSEE_CONFIG --libs $module_args`
      ifelse([$1], , :, [$1])
    else
      AC_MSG_RESULT(bad installation)
    fi
  fi
else
  AC_MSG_CHECKING(for omsee)

  AC_MSG_RESULT(using uninstalled version)
  if test "x$OMSEE_UNINST_EXEC" = "xno"; then
    OMSEE_UNINST_EXEC=$OMSEE_UNINST
  fi
  OMSEE_CFLAGS="-I$OMSEE_UNINST/include -I$OMSEE_UNINST/common"
  OMSEE_LIBS="$OMSEE_UNINST_EXEC/libomsee.la"
fi

AC_SUBST(OMSEE_CFLAGS)
AC_SUBST(OMSEE_LIBS)
])
