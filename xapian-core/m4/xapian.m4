# a macro to get the libs/cflags for xapian library
# serial 3

dnl OM_PATH_XAPIAN[ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Find paths to Xapian library
dnl Defines XAPIAN_CFLAGS and XAPIAN_LIBS
dnl
AC_DEFUN(OM_PATH_XAPIAN,
[dnl
dnl Get the cflags and libraries from the xapian-config script
dnl
AC_ARG_WITH(xapian-config,
[  --with-xapian-config   Location of xapian-config],
XAPIAN_CONFIG="$withval")

AC_ARG_WITH(xapian-uninst,
[  --with-xapian-uninst   Location of top dir of uninstalled xapian],
XAPIAN_UNINST="$withval")

AC_ARG_WITH(xapian-uninst_exec,
[  --with-xapian-uninst-exec  Location of built xapian, if srcdir not builddir],
XAPIAN_UNINST_EXEC="$withval")


if test "x$XAPIAN_CONFIG" = "x"; then
  XAPIAN_CONFIG=no
fi

if test "x$XAPIAN_UNINST" = "xyes"; then
  AC_MSG_ERROR(--with-xapian-uninst needs path of top dir of xapian)
fi
if test "x$XAPIAN_UNINST" = "x"; then
  XAPIAN_UNINST=no
fi

if test "x$XAPIAN_UNINST_EXEC" = "xyes"; then
  AC_MSG_ERROR(--with-xapian-uninst-exec needs path of top dir of xapian build)
fi
if test "x$XAPIAN_UNINST_EXEC" = "x"; then
  XAPIAN_UNINST_EXEC=no
fi


dnl If paths aren't absolute, complain
case x$XAPIAN_UNINST in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for xapian-uninst must be absolute (was $XAPIAN_UNINST)]) ;;
esac
case x$XAPIAN_UNINST_EXEC in
  xno) : ;;
  x/*) : ;;
  x*) AC_MSG_ERROR([Path specified for xapian-uninst-exec must be absolute (was $XAPIAN_UNINST_EXEC)]) ;;
esac

dnl XAPIAN_UNINST and XAPIAN_UNINST_EXEC are either an absolute path or no

if test "x$XAPIAN_UNINST" = "xno"; then
  if test "x$XAPIAN_UNINST_EXEC" != "xno"; then
    AC_MSG_ERROR(must specify --with-xapian-uninst if using --with-xapian-uninst-exec)
  fi
  if test "x$XAPIAN_CONFIG" = "xno"; then
    AC_PATH_PROG(XAPIAN_CONFIG, xapian-config, no)
  fi

  AC_MSG_CHECKING(for xapian)
  if test "x$XAPIAN_CONFIG" = "xno"; then
    AC_MSG_RESULT(not found)
    ifelse([$2], , :, [$2])
  else
    if $XAPIAN_CONFIG --check; then
      AC_MSG_RESULT(yes)
      XAPIAN_CFLAGS=`$XAPIAN_CONFIG --cflags`
      XAPIAN_LIBS=`$XAPIAN_CONFIG --libs`
      ifelse([$1], , :, [$1])
    else
      AC_MSG_RESULT(bad installation)
    fi
  fi
else
  AC_MSG_CHECKING(for xapian)

  AC_MSG_RESULT(using uninstalled version)
  if test "x$XAPIAN_UNINST_EXEC" = "xno"; then
    XAPIAN_UNINST_EXEC=$XAPIAN_UNINST
  fi
  XAPIAN_CONFIG_TMP="$XAPIAN_UNINST_EXEC/xapian-config"
  if test -x "$XAPIAN_CONFIG_TMP"; then
    : ;
  else
    cp $XAPIAN_CONFIG_TMP .xapian-config_tmp
    XAPIAN_CONFIG_TMP=./.xapian-config_tmp
    chmod +x .xapian-config_tmp
  fi
  XAPIAN_CFLAGS=`$XAPIAN_CONFIG_TMP --uninst --prefix=$XAPIAN_UNINST --exec-prefix=$XAPIAN_UNINST_EXEC --cflags`
  XAPIAN_LIBS=`$XAPIAN_CONFIG_TMP --uninst --prefix=$XAPIAN_UNINST --exec-prefix=$XAPIAN_UNINST_EXEC --libs`
  if test -x ".xapian-config_tmp"; then
    rm -f .xapian-config_tmp;
  fi
fi

AC_SUBST(XAPIAN_CFLAGS)
AC_SUBST(XAPIAN_LIBS)
])
